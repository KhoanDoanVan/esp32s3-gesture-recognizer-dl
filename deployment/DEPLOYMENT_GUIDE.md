# ESP32-S3 Deployment Guide

This document describes the full deployment flow for this project on macOS, from environment setup to successful inference on an ESP32-S3 board.

This project is an **ESP-IDF project**, not an Arduino sketch. It does **not** use a `.ino` file.

## 1. Hardware Used

This guide matches an ESP32-S3 board with:

- 16 MB SPI Flash
- 8 MB PSRAM
- ESP32-S3-WROOM-1 class module

The tested board profile is a safe configuration for:

- Flash mode: `DIO`
- Flash speed: `40 MHz`
- PSRAM: enabled
- PSRAM mode: `Octal`
- PSRAM speed: `40 MHz`

## 2. Recommended Software Versions

Use the same software baseline that this project was tested with:

- ESP-IDF: `v5.3.1`
- ESP-DL: commit `ac58ec9c0398e665c9b1d66d3760ac47d1676018`

The repository notes these tested commits in [index.md](/Users/doanvankhoan/Desktop/esp32s3-gesture-dl/index.md:36).

## 3. Install Prerequisites on macOS

Install Xcode Command Line Tools:

```bash
xcode-select --install
```

Install Homebrew packages:

```bash
brew install git cmake ninja dfu-util ccache python@3.10
```

## 4. Clone ESP-IDF and ESP-DL

Create a common workspace:

```bash
mkdir -p ~/esp
cd ~/esp
```

Clone ESP-IDF:

```bash
git clone -b v5.3.1 --recursive https://github.com/espressif/esp-idf.git
```

Clone ESP-DL:

```bash
git clone https://github.com/espressif/esp-dl.git
```

Pin ESP-DL to the tested commit:

```bash
git -C ~/esp/esp-dl fetch --all
git -C ~/esp/esp-dl checkout ac58ec9c0398e665c9b1d66d3760ac47d1676018
```

This step matters. A newer `esp-dl` commit may use APIs or model schema versions that do not match this project.

## 5. Install and Activate ESP-IDF

Install ESP-IDF tools for ESP32-S3:

```bash
cd ~/esp/esp-idf
./install.sh esp32s3
```

Activate ESP-IDF in the current shell:

```bash
. ~/esp/esp-idf/export.sh
```

Verify:

```bash
idf.py --version
```

If `idf.py` is not found, your shell is not using the ESP-IDF environment yet.

## 6. Optional Python Environment for Helper Scripts

This project also uses Python helper scripts such as:

- `generate_image_header.py`
- `pack_model.py`

From the repository root:

```bash
cd /Users/doanvankhoan/Desktop/esp32s3-gesture-dl
python3.10 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip
python -m pip install -r requirements.txt
```

You can keep a separate training environment if you want, but deployment itself runs through ESP-IDF.

## 7. Open the ESP-IDF Project

The correct ESP-IDF project folder is:

```bash
cd /Users/doanvankhoan/Desktop/esp32s3-gesture-dl/deployment
```

This folder contains:

- `CMakeLists.txt`
- `sdkconfig`
- `partitions.csv`
- `main/`

Run `idf.py` commands from here.

## 8. Check the ESP-DL Component Override Path

This project uses a local ESP-DL checkout through:

[main/idf_component.yml](/Users/doanvankhoan/Desktop/esp32s3-gesture-dl/deployment/main/idf_component.yml)

Recommended form:

```yaml
dependencies:
  espressif/esp-dl:
    version: "^3.0.0-rc.1"
    override_path: "${HOME}/esp/esp-dl/esp-dl"
```

If the path does not exist, build will fail before compilation starts.

Verify the directory:

```bash
ls ~/esp/esp-dl/esp-dl
```

## 9. Check Required Project Files

Before building, make sure these files exist:

- `deployment/partitions.csv`
- `deployment/model/gesture_model_int8.espdl`
- `deployment/model/test_image.hpp`

The original repository includes an `EDL1` model file. This is important for compatibility with the runtime used here.

## 10. Configure the Project with `menuconfig`

Run:

```bash
idf.py menuconfig
```

Recommended settings for a 16 MB Flash / 8 MB PSRAM ESP32-S3 board:

### Serial flasher config

- Flash size: `16 MB`
- Flash mode: `DIO`
- Flash frequency: `40 MHz`

### Partition Table

- Partition Table: `Custom partition table CSV`
- Custom partition CSV file: `partitions.csv`

### Component config

Enable external RAM / PSRAM:

- PSRAM: `Enabled`
- PSRAM mode: `Octal`
- PSRAM speed: `40 MHz`

Save and exit.

## 11. About `partitions.csv`

This file defines the flash layout of the board.

Current layout:

```csv
# Name,   Type, SubType,  Offset,   Size,  Flags
nvs,      data, nvs,      0x9000,   24K,
phy_init, data, phy,      0xf000,   4K,
factory,  app,  factory,  0x10000,  4000K,
model,    data, spiffs,   ,         256K,
```

Meaning:

- `factory`: main firmware
- `model`: dedicated flash partition for the packaged `.espdl` model

The application loads the model from the `model` partition at runtime.

## 12. Generate the Test Image Header

This demo does not read from a live camera. It runs inference on a preprocessed image embedded into `test_image.hpp`.

Generate it like this:

```bash
cd /Users/doanvankhoan/Desktop/esp32s3-gesture-dl/deployment
source ../.venv/bin/activate
python generate_image_header.py /absolute/path/to/your_test_image.jpg
deactivate
```

This creates:

```text
deployment/model/test_image.hpp
```

## 13. Model Path in `main/CMakeLists.txt`

In the original project, the model path can be provided through `CONFIG_MODEL_FILE_PATH`.

If your build shows:

```text
warning: unknown kconfig symbol 'MODEL_FILE_PATH'
CONFIG_MODEL_FILE_PATH:
```

then hardcode the model path in:

[main/CMakeLists.txt](/Users/doanvankhoan/Desktop/esp32s3-gesture-dl/deployment/main/CMakeLists.txt)

Change:

```cmake
set(MODEL_FILE_PATH ${PROJECT_DIR}/${CONFIG_MODEL_FILE_PATH})
```

to:

```cmake
set(MODEL_FILE_PATH ${PROJECT_DIR}/model/gesture_model_int8.espdl)
```

This is a practical workaround when `MODEL_FILE_PATH` is not loaded correctly through Kconfig.

## 14. Build

Build the project:

```bash
cd /Users/doanvankhoan/Desktop/esp32s3-gesture-dl/deployment
idf.py fullclean
idf.py reconfigure
idf.py build
```

If build succeeds, it will:

- build the app binary
- package the model into `build/espdl_models/models.espdl`
- prepare flashing for the `model` partition

## 15. Flash

Find the serial port:

```bash
ls /dev/cu.*
```

Flash the board:

```bash
idf.py -p /dev/cu.usbmodemXXXXXXXX flash
```

For this board, successful flash should include:

- `bootloader.bin`
- `gesture.bin`
- `partition-table.bin`
- `espdl_models/models.espdl`

## 16. Monitor

Open the serial monitor:

```bash
idf.py -p /dev/cu.usbmodemXXXXXXXX monitor
```

Quit monitor with:

```text
Ctrl + ]
```

## 17. Expected Successful Runtime Output

On a good run, you should see:

- PSRAM detected
- model loaded successfully
- model graph printed
- one input tensor named `input`
- inference time
- gesture confidence output

Example successful signs:

```text
dl::Model: model:main_graph, version:0
GESTURE_RECOGNITION: Number of input tensors: 1
GESTURE_RECOGNITION: Running inference...
GESTURE_RECOGNITION: Gesture Recognition Results:
```

## 18. Important Model Format Note: `EDL1` vs `EDL2`

This was the most important runtime issue during bring-up.

The loader in the tested ESP-DL version understands:

- `EDL1`
- `PDL1`

If your `.espdl` file starts with:

```text
EDL2
```

the runtime may report:

```text
Unsupported format, or the model file is corrupted!
```

even though the file itself is valid.

This is a **schema version mismatch**, not actual corruption.

Check the model header:

```bash
xxd -l 16 deployment/model/gesture_model_int8.espdl
```

For the tested runtime, the model should begin with:

```text
EDL1
```

If you accidentally use an `EDL2` model, replace it with the original `EDL1` model artifact from the original repository.

## 19. Common Errors and Fixes

### `idf.py: command not found`

Cause:

- ESP-IDF environment is not active

Fix:

```bash
conda deactivate
. ~/esp/esp-idf/export.sh
```

### `override_path does not point to a directory`

Cause:

- `main/idf_component.yml` points to a missing ESP-DL checkout

Fix:

```bash
ls ~/esp/esp-dl/esp-dl
```

and update `override_path` if needed.

### `partitions.csv not found`

Cause:

- missing file
- wrong working project folder
- wrong custom partition CSV path

Fix:

- confirm `deployment/partitions.csv` exists
- confirm `menuconfig` points to `partitions.csv`

### `TypeError: can't concat NoneType to bytes`

Cause:

- `pack_model.py` was given the wrong model path
- no `.espdl` file was found

Fix:

- make sure `MODEL_FILE_PATH` points to the actual model file

### `gesture_model_int8.espdl missing`

Cause:

- model artifact is not present in `deployment/model/`

Fix:

- copy the correct model file into `deployment/model/`

### `MALLOC_CAP_SIMD was not declared`

Cause:

- ESP-DL version too new for the ESP-IDF / project combination

Fix:

Use the tested ESP-DL commit:

```bash
git -C ~/esp/esp-dl fetch --all
git -C ~/esp/esp-dl checkout ac58ec9c0398e665c9b1d66d3760ac47d1676018
```

### `Unsupported format, or the model file is corrupted!`

Cause:

- you flashed an `EDL2` model while the runtime expects `EDL1` or `PDL1`

Fix:

- replace the model artifact with the compatible `EDL1` version
- rebuild
- flash again

## 20. What the Demo Actually Does

The current deployment demo is:

```text
image on PC -> Python preprocessing -> test_image.hpp -> ESP32-S3 inference
```

It is **not** a live camera pipeline yet.

If you want a realtime system later, you will need to replace the static `test_image.hpp` input with:

```text
camera frame -> preprocessing on device -> int8 tensor -> model->run()
```

## 21. Build Summary

Minimal command sequence:

```bash
xcode-select --install
brew install git cmake ninja dfu-util ccache python@3.10

mkdir -p ~/esp
cd ~/esp
git clone -b v5.3.1 --recursive https://github.com/espressif/esp-idf.git
git clone https://github.com/espressif/esp-dl.git
git -C ~/esp/esp-dl fetch --all
git -C ~/esp/esp-dl checkout ac58ec9c0398e665c9b1d66d3760ac47d1676018

cd ~/esp/esp-idf
./install.sh esp32s3
. ~/esp/esp-idf/export.sh

cd /Users/doanvankhoan/Desktop/esp32s3-gesture-dl/deployment
idf.py menuconfig
idf.py fullclean
idf.py reconfigure
idf.py build
idf.py -p /dev/cu.usbmodemXXXXXXXX flash
idf.py -p /dev/cu.usbmodemXXXXXXXX monitor
```

## 22. Final Result

A successful final run should:

- boot the ESP32-S3
- detect PSRAM
- load the model from the `model` partition
- run inference on `test_image.hpp`
- print confidence values for the 8 gesture classes

At that point, the deployment pipeline is working end to end.

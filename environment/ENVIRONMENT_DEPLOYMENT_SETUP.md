# Environment Deployment Setup on MacOS for setup ESP-IDF tools

## macOS Environment Setup

This section is for the `deployment/` stage on macOS, including:

- `idf.py menuconfig`
- `idf.py build`
- `idf.py flash`
- `idf.py monitor`
- Python helper scripts such as `deployment/generate_image_header.py`

### Project assumptions

This repository is aligned with:

- ESP-IDF `v5.3.1`
- ESP-DL checked out locally
- ESP32-S3 target

The project overrides `esp-dl` from a local path in `deployment/main/idf_component.yml`, so the expected layout is:

```text
~/esp/esp-idf
~/esp/esp-dl
```

### 1. Install macOS prerequisites

Install Xcode Command Line Tools:

```bash
xcode-select --install
```

Install Homebrew packages:

```bash
brew install git cmake ninja dfu-util ccache python@3.10
```

Verify:

```bash
git --version
cmake --version
ninja --version
python3.10 --version
```

### 2. Clone ESP-IDF and ESP-DL

```bash
mkdir -p ~/esp
cd ~/esp
git clone -b v5.3.1 --recursive https://github.com/espressif/esp-idf.git
git clone https://github.com/espressif/esp-dl.git
```

Optional, if you also want to study Espressif quantization tooling:

```bash
cd ~/esp
git clone https://github.com/espressif/esp-ppq.git
```

### 3. Install ESP-IDF tools

```bash
cd ~/esp/esp-idf
./install.sh esp32s3
```

If GitHub asset downloads are slow:

```bash
cd ~/esp/esp-idf
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"
./install.sh esp32s3
```

### 4. Activate ESP-IDF

Run this in every shell where you want to use `idf.py`:

```bash
. ~/esp/esp-idf/export.sh
```

Verify:

```bash
idf.py --version
echo $IDF_PATH
```

Optional alias for `zsh`:

```bash
echo 'alias get_idf=". ~/esp/esp-idf/export.sh"' >> ~/.zshrc
source ~/.zshrc
```

Then you can activate ESP-IDF with:

```bash
get_idf
```

### 5. Optional Python environment for helper scripts

From the repository root:

```bash
cd /path/to/esp32s3-gesture-recognizer-dl
python3.10 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip
python -m pip install -r requirements.txt
```

Verify:

```bash
python -c "import cv2, numpy, onnxruntime; print('ok')"
```

Deactivate when done:

```bash
deactivate
```

### 6. Open the deployment project

```bash
cd /path/to/esp32s3-gesture-recognizer-dl/deployment
idf.py set-target esp32s3
idf.py menuconfig
```

### 7. Recommended `menuconfig` checks

Check the following items so they match your board and this repo:

- Flash size
- Partition table using `partitions.csv`
- External RAM / SPIRAM enabled
- Octal PSRAM mode if your board supports it
- UART baud rate, typically `115200`

The repo defaults already target ESP32-S3 with PSRAM enabled.

### 8. Build

```bash
cd /path/to/esp32s3-gesture-recognizer-dl/deployment
idf.py build
```

### 9. Flash and monitor

Find your serial port:

```bash
ls /dev/cu.*
```

Flash:

```bash
idf.py -p /dev/cu.usbserial-XXXX flash
```

Flash and monitor:

```bash
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

### 10. Generate a test image header

This project currently runs inference from a preprocessed image embedded into `test_image.hpp`.

```bash
cd /path/to/esp32s3-gesture-recognizer-dl/deployment
source ../.venv/bin/activate
python generate_image_header.py /absolute/path/to/your_test_image.jpg
deactivate
```

This generates:

```text
deployment/model/test_image.hpp
```

### 11. Common issues

#### `idf.py: command not found`

Activate ESP-IDF first:

```bash
. ~/esp/esp-idf/export.sh
```

#### `xcrun: error: invalid active developer path`

Install Xcode Command Line Tools:

```bash
xcode-select --install
```

#### `ModuleNotFoundError: No module named 'cv2'`

Activate the repo virtual environment and install dependencies:

```bash
cd /path/to/esp32s3-gesture-recognizer-dl
source .venv/bin/activate
python -m pip install -r requirements.txt
```

#### ESP-DL component path error

Make sure ESP-DL is cloned here:

```text
~/esp/esp-dl
```

#### Board not detected

Check serial devices:

```bash
ls /dev/cu.*
```

If your board uses CH340/CH341, install the proper macOS driver if needed.

## Suggested daily workflow

### Terminal 1: ESP-IDF shell

```bash
get_idf
cd /path/to/esp32s3-gesture-recognizer-dl/deployment
idf.py build
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

### Terminal 2: Python helper shell

```bash
cd /path/to/esp32s3-gesture-recognizer-dl
source .venv/bin/activate
```

## Reference commands summary

```bash
xcode-select --install
brew install git cmake ninja dfu-util ccache python@3.10

mkdir -p ~/esp
cd ~/esp
git clone -b v5.3.1 --recursive https://github.com/espressif/esp-idf.git
git clone https://github.com/espressif/esp-dl.git

cd ~/esp/esp-idf
./install.sh esp32s3
. ~/esp/esp-idf/export.sh

cd /path/to/esp32s3-gesture-recognizer-dl
python3.10 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip
python -m pip install -r requirements.txt
deactivate

cd /path/to/esp32s3-gesture-recognizer-dl/deployment
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

## Community Resources

- [**Complete ESP-DL Workflow Guide**](https://github.com/alibukharai/Blogs/tree/main/ESP-DL)
- Comprehensive tutorial covering setup, model conversion, quantization and deployment
- Environment setup
- Model preparation
- Quantization process
- ESP32 deployment
- Performance optimization

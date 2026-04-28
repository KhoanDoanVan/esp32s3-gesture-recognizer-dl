# Deployment Partition Layout

This document explains the purpose of `partitions.csv` in the ESP32-S3 deployment project.

## What `partitions.csv` Does

`partitions.csv` defines the flash memory layout of the ESP32 device.

In other words, it tells ESP-IDF how to divide the board's flash into named regions, and what each region is used for.

For this project, the file looks like this:

```csv
# Name,   Type, SubType,  Offset,   Size,  Flags
nvs,      data, nvs,      0x9000,   24K,
phy_init, data, phy,      0xf000,   4K,
factory,  app,  factory,  0x10000,  4000K,
model,    data, spiffs,   ,         256K,
```

## Why This File Matters

This project does not store the AI model directly inside the application binary.

Instead, it uses a dedicated flash partition named `model` to store the packaged `.espdl` model file. At runtime, the firmware loads the model from that partition and runs inference on the ESP32-S3.

Without `partitions.csv`, ESP-IDF does not know:

- where the main firmware should be placed
- where the model data should be placed
- how large each flash region should be

That is why the build can fail if this file is missing or if `menuconfig` points to the wrong partition table path.

## Partition-by-Partition Explanation

### `nvs`

- Type: `data`
- Subtype: `nvs`
- Size: `24K`

This partition stores non-volatile key-value data used by the system and libraries.

Typical uses include:

- saved configuration values
- Wi-Fi calibration data
- application settings

## `phy_init`

- Type: `data`
- Subtype: `phy`
- Size: `4K`

This partition stores PHY initialization data used by the wireless subsystem.

It is part of the normal ESP-IDF layout for Wi-Fi and Bluetooth-capable chips.

## `factory`

- Type: `app`
- Subtype: `factory`
- Offset: `0x10000`
- Size: `4000K`

This is the main firmware partition.

Your application binary is flashed here. In this project, that includes:

- the ESP-IDF application
- model loading logic
- tensor preparation
- inference execution
- result logging

You can think of this partition as the "program" part of the project.

## `model`

- Type: `data`
- Subtype: `spiffs`
- Size: `256K`

This is the custom partition used to store the quantized model file.

In this repository, the model is packaged into a flashable file and written into this partition during deployment. The application then loads the model from this region instead of embedding it into the firmware image itself.

You can think of this partition as the "AI model storage" part of the project.

## How It Fits into the Deployment Flow

The deployment flow is roughly:

1. Quantize the trained model into `.espdl`
2. Package the model for flashing
3. Flash the firmware into the `factory` partition
4. Flash the model into the `model` partition
5. Boot the board
6. Load the model from flash at runtime
7. Run inference on the input image

## Why the `model` Partition Is Useful

Using a separate model partition has a few advantages:

- the firmware and model are managed independently
- the application binary stays smaller
- the model can be replaced without redesigning the flash layout
- the runtime code can explicitly load the model from a known partition

## Size Considerations

The current `model` partition size is `256K`.

That means the packaged `.espdl` model must fit inside 256 KB. If you later switch to a larger model and flashing fails, or if the model package exceeds this limit, you will need to increase the size of the `model` partition in `partitions.csv`.

If you change partition sizes, make sure the new layout still fits the flash size of your board.

## Common Build Issue

If you see an error like:

```text
partition table CSV file .../deployment/partitions.csv not found
```

it usually means one of these is true:

- `partitions.csv` is missing from the `deployment/` directory
- `menuconfig` is pointing to the wrong custom partition CSV path
- you are building a different copy of the project than the one you think you are using

## Summary

`partitions.csv` is the flash memory map for the ESP32-S3 deployment project.

It defines:

- system storage regions like `nvs` and `phy_init`
- the main application region `factory`
- the dedicated model storage region `model`

For this repository, it is a critical part of the deployment pipeline because the firmware depends on the custom `model` partition to load the AI model at runtime.

# ESP32-S3 Model Deployment

## Updates

#### Preprocessing
Add skin color detection and image preprocessing module
- implemented intelligent skin color segmentation based on YCrCb color space
- introduced morphological optimization (OPEN/CLOSE) to improve edge detection quality
- integrated adaptive constrast enhancement ($\alpha=1.5$) and dynamic binarization
- added robust exception handling mechanism
- supported command line calls and batch processing functions
- standardized output path management, automatically created target folders

#### Build
Configure dependencies
- introduced OpenCV core modules
- integrated NumPy numerical computing support

## Project Overview
This deployment project demonstrates running a quantized gesture recognition model on ESP32-S3. The implementation focuses on efficient memory usage, fast inference, and reliable model execution.

## Project Structure

```
.
├── model/                      # Model and header files directory
│   ├── test_image.hpp         # Generated image header
│   └── gesture_model.espdl    # Quantized model
├── CMakeLists.txt             # Project CMake configuration
├── app_main.cpp               # Main application code
├── generate_image_header.py   # Image preprocessing utility
├── pack_model.py              # Model packaging tool
└── partitions.csv            # Custom partition configuration
```

## Environment Setup

**Driver Installation**

For new ESP32-S3 devices, you may need to install USE drivers:

1. CH340/CH341 Driver (Silicon Labs):
- required for use communication
- detail guide: https://blog.csdn.net/qq_52102933/article/details/126839474

2. Alternative Driver Method:
- using Zadig tool
- detail guide: https://blog.csdn.net/k1e2n3n4y5/article/details/132684803


**ESP-IDF Configuration**

use `idf.py menuconfig` to configure:

1. Memory Configuration
```
Component Config
   └── ESP32S3-Specific
       ├── Flash Size: 8MB
       └── Support for external RAM
           └── SPIRAM: Enabled
           └── Mode: Allow .bss segment placed in PSRAM
```

2. Partition Configuration
```
Partition Table
├── Partition Table: Custom partition table CSV
└── Custom partition CSV file: partitions.csv
```

3. Serial Communication
```
Serial Flasher Config
   ├── Default serial port: [YOUR_PORT]
   └── Flash baud rate: 115200
```

4. Flash Size
```
Serial Flasher Config
   └── Flash Size: 8MB
```

## Build Configuration

### Project CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.5)

# ESP-DL library path
set(EXTRA_COMPONENT_DIRS 
    "$ENV{HOME}/esp/esp-dl/esp-dl"     # Adjust path as needed
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(gesture_recognition)
```

### Component CMakeLists.txt
```cmake
idf_component_register(
    SRCS 
        "app_main.cpp"
    INCLUDE_DIRS 
        "."
        "model"
    REQUIRES 
        esp-dl
)
```

## Memory Management

### PSRAM Utilization
- model weights storted in **PSRAM**
- input/output tensors allocated in **PSRAM**
- runtime buffers use internal **RAM** when possible

### Memory Monitoring

the application includes memory monitoring
```cpp
size_t free_mem = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
ESP_LOGI(TAG, "Free PSRAM: %u bytes", free_mem);
```

## Image Preprocessing

`generate_image_header.py` handles image preprocessing:
1. grayscale conversion
2. resize to $96\times96$
3. normalization to [0,1]
4. INT8 quantization

Key parameters:
- input size: $96\times96$
- pixel format: grayscale
- quantization: INT8 (-128 to 127)
- scale factor: 128 (for quantization)

## Model Loading and Inference
### Model Loading
- models are loaded from a dedicated flash partition
- partition defined in `partitions.csv`
- uses ESP-DL's model loader interface

### Inference Pipeline
1. input preparation
2. model execution
3. output processing
4. confidence calculation

### Performance Monitoring
- inference time measurement
- memory usage tracking
- model loading time monitoring

## Common issues and Solutions
### Memory Errors
1. PSRAM Allocation Failure
- ensure PSRAM is enabled
- check parition table configuration
- monitor memory fragmentation

2. Stack Overflow
- increase stack size in menuconfig
- optimize recursive functions
- move large buffers to heap

### USB Communication Issues
1. Port Detection
- install correct drivers
- check USB cable
- verify port permissions

2. Flash Errors
- reduce flash speed in menuconfig
- check power supply stability
- verify flash size configuration
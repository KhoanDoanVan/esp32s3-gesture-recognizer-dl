# Model Quantization for ESP32-S3

This folder contains quantized models optimized for ESP32-S3 deployment, exploring three different quantization strategies to achieve the optimal balance between model size, accuracy, and inference speed

## Directory Structure
```
.
├── gesture_model_int8/
│   ├── gesture_model_int8.espdl    # INT8 quantized model
│   ├── gesture_model_int8.info     # Performance metrics
│   └── gesture_model_int8.json     # Model configuration
├── gesture_model_mixed/
│   ├── gesture_model_mixed.espdl   # Mixed precision model
│   ├── gesture_model_mixed.info    # Performance metrics
│   └── gesture_model_mixed.json    # Model configuration
├── gesture_model_balanced/
│   ├── gesture_model_balanced.espdl # Equalized model
│   ├── gesture_model_balanced.info  # Performance metrics
│   └── gesture_model_balanced.json  # Model configuration
└── notebooks/
    ├── quantize_8bit.ipynb         # INT8 quantization script
    ├── quantize_mixed.ipynb        # Mixed precision script
    └── quantize_equalization.ipynb  # Equalization script
```


## Quantization Methods

### 1. INT8 Quantization
implementation: ```quantize_8bit.ipynb```

Baseline quantization appoarch:
- uniform 8-bit quantization across all alyers
- symmetric quantization scheme
- per-channel quantization for weights
- per-tensor quantization for activations

### 2. Mixed Precision Quantization
implementation: ```quantize_mixed.ipynb```

Uses combination of 8-bit and 16-bit quantization:
- default: 8-bit precision
- critical layers: 16-bit precision
- Layer-specific optimization based on error sensitivity

### 3. Equalization-Aware Quantization
implementation: ```quantize_equalization.ipynb```

Layer-wise scaling optimization:
- iterations: 4
- value threshold: 0.4
- optimization level: 2
- ReLU6 to ReLU conversion for compatibility

## Model Requirements
### Input format
- size: $96\times96$ pixels
- channels: grayscale (1 channel)
- type: float32
- range: [0,1]
- shape: [1, 1, 96, 96]

### Dependencies
- PPQ (PyTorch Post-training Quantization toolkit)
- PyTorch
- ONNX
- OpenCV
- Numpy
- ESP-IDF
- ESP-DL

### Future Optimizations
1. Dynamic quantization
- Runtime activation quantization
- Adaptive input handling
2. Hardware-aware optimization
- ESP32-S3 specific features
- Hardware acceleration
3. Advanced calibration
- Entropy-based methods
- Per-layer strategies
4. Post-training optimization
- Weight pruning
- Channel reduction
- Knowledge distillation
# Hand Géture Recognition Model Training

## Project Structure

```
.
├── best_model.pth
├── data_label_verification.ipynb
├── gesture_model.onnx
├── gesture_model.pth
├── gesture_model.tflite
├── model.ipynb
├── Onnx Inference.py
├── tf_gesture_model
│   ├── assets
│   ├── fingerprint.pb
│   ├── saved_model.pb
│   └── variables
│       ├── variables.data-00000-of-00001
│       └── variables.index
├── train-Copy1.ipynb
├── TRAINING_GUIDE_cn.md
├── TRAINING_GUIDE.md
└── train.ipynb
```

## Model Archiecture

**LightGestureNet**
- based on MobileNetV2's inverted residual blocks
- optimized for mobiel and edge deployment
- input: $96 \times 96$ grayscale images
- output: $8$ gesture classes
- key features:
    - depthwise separable convolutions
    - residual connections
    - batch normalization
    - ReLU6 activation functions

```python
# Network structure
first_layer = Conv2d(1, 16, 3, stride=2)  # Initial convolution
inverted_residual_blocks = [
    (16, 24, stride=2, expand_ratio=6),
    (24, 24, stride=1, expand_ratio=6),
    (24, 32, stride=2, expand_ratio=6),
    (32, 32, stride=1, expand_ratio=6)
]
classifier = Linear(32, num_classes=8)
```


## Training Details

### Data Preprocessing
- image resize to $96 \times 96$
- grayscale conversion
- normalization to [0,1] range
- data augmentation:
    - random rotation (±30°)
    - random scaling (0.8-1.2x)
    - random translation (±20%)

### Training Configuration
- optimizer: Adam
- learning rate: 0.001 with cosine annealing
- batch size: 32
- loss function: cross-entropy
- early stopping patience: 5 epochs
- target accuracy threshold: 99.27%

### Performance Monitoring
- training metrics tracked:
    - loss (training and validation)
    - accuracy (training and validation)
    - learning rate changes
    - early stopping conditions


## Data Verification Tools

The ```data_label_verification.ipynb``` notebook provides tools for:
- dataset integrity validation
- class distribution visualization
- error analysis using a simple CNN
- sample visualization and inspection
- class balance checking


## Exported Model Formats

### Pytorch models
- ```best_model.pth```: best performing checkpoint during training
- ```gesture_model.pth```: Final trained model

### ONNX format
- ```gesture_model.onnx```: cross-platform inference

### Tensorflow formats
- ```gesture_model.tflite```: mobile deployment ready
- ```tf_gesture_model/```: tensorflow savedModel


## Class Mapping
```python
CLASS_NAMES = {
    0: 'palm',
    1: 'l',
    2: 'fist',
    3: 'thumb',
    4: 'index',
    5: 'ok',
    6: 'c',
    7: 'down'
}
```


## Training results
The model achieves:
- training accuracy: >99%
- validation accuracy: >98%
- average inference time: ~5ms on CPU
- model size:
    - pytorch: ~2MB
    - TFLite: ~1.5MB
    - ONNX: ~2.2MB


## Future improvements
1. Data augmentation enhancements:
- brightness variation
- gaussian noise
- random erasing
2. Model optimization:
- quantization
- pruning
- knowledge distillation
3. Training improvements:
- mixed precision training
- gradient clipping
- label smoothing
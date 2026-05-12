# ESP32-S3 Gesture Recognizer

An end-to-end embedded AI project for hand gesture recognition on ESP32-S3.  
This repository covers the full delivery pipeline: dataset preparation, model training, quantization, hardware deployment, environment setup, debugging, and result logging.

## Project Scope

- Built a preprocessing pipeline for grayscale hand images at `96x96`.
- Trained a lightweight `LightGestureNet` model for 8 gesture classes.
- Exported the model to `PyTorch`, `ONNX`, `TFLite`, and `ESP-DL` formats.
- Evaluated 3 deployment-oriented quantization strategies: `INT8`, `mixed precision`, and `equalization-aware`.
- Deployed the quantized model with `ESP-IDF` on an ESP32-S3 board.
- Documented setup, build, troubleshooting, and quantization investigation for reproducibility.

## Results

| Metric | Result |
|---|---|
| Gesture classes | 8 |
| Processed dataset size | 16,000 images |
| Input format | `96x96` grayscale |
| Model size | 39,256 parameters |
| Embedded model size | `67-68 KB` `.espdl` |
| Documented training performance | `>99%` train accuracy, `>98%` validation accuracy |
| Best recorded validation accuracy | `99.75%` |
| Tested hardware | ESP32-S3, `16 MB` Flash, `8 MB` PSRAM |
| Model load time on device | `239 ms` |
| Inference time on device | `155 ms` |
| Runtime memory reported by ESP-DL | `322,560 bytes` |
| Sample on-device prediction | `ok` with `100%` confidence |

## Repository Structure

- `dataset/` - dataset notes and preprocessing notebooks
- `training/` - architecture, training notebooks, and exported models
- `quantization/` - quantization experiments and ESP-DL model variants
- `deployment/` - ESP-IDF project and on-device inference code
- `environment/` - local setup guide for macOS and ESP-IDF
- `issues/` - quantization debugging and validation notes
- `logs_manual/` - manual hardware execution logs

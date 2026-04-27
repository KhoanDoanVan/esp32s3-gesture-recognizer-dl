# ESP-DL / PPQ Quantization Issues

This document summarizes the issues found when comparing the owner's PPQ/ESP-DL quantization log with the result from the current code.

## Summary

The model architecture is **probably not actually different**.

Both logs show the same number of executable operators:

| Item | Owner Result | My Result |
|---|---:|---:|
| Number of Ops | 27 | 27 |
| Number of Quantized Ops | 27 | 27 |
| Number of Variables | 74 | 76 |
| Number of Quantized Variables | 74 | 76 |
| Number of Quant Configs | 102 | 104 |
| FP32 Quant Configs | Not shown | 2 |

The main difference is that my ONNX graph contains **2 extra variables / quantization configs**, and ESP-DL reports that some values are not exportable.

Therefore, the most likely problem is not the PyTorch model architecture itself, but the **ONNX export / graph representation / PPQ-ESP-DL conversion pipeline**.

---

## Issue 1: Different Layer Names

### Owner result

```text
/layers/layers.0/conv/conv.6/Conv
/layers/layers.1/conv/conv.6/Conv
/classifier/classifier.2/Gemm
```

### My result

```text
node_Conv_224
node_Conv_218
node_linear
```

### Explanation

The owner's graph preserves PyTorch-style module names, while my graph uses generated ONNX node names.

This does **not automatically mean** the architecture is different. It usually means the model was exported with different ONNX settings, PyTorch version, ONNX version, simplification process, or naming behavior.

### Action

Compare the ONNX files by operator type instead of node name.

```python
import onnx
from collections import Counter


def inspect_onnx(path):
    model = onnx.load(path)

    print("File:", path)
    print("Number of nodes:", len(model.graph.node))
    print("Inputs:", [i.name for i in model.graph.input])
    print("Outputs:", [o.name for o in model.graph.output])
    print("Initializers:", len(model.graph.initializer))

    op_types = Counter(node.op_type for node in model.graph.node)
    print("\nOperator types:")
    for op_type, count in sorted(op_types.items()):
        print(f"{op_type}: {count}")

    print("\nNodes:")
    for node in model.graph.node:
        print(node.name, node.op_type)


inspect_onnx("owner_model.onnx")
inspect_onnx("my_model.onnx")
```

If the operator counts are the same, the model is likely architecturally equivalent.

---

## Issue 2: Extra Variables and Extra Quant Configs

### Owner result

```text
Num of Variable:              [74]
Num of Quantized Var:         [74]
Num of Quant Config:          [102]
```

### My result

```text
Num of Variable:              [76]
Num of Quantized Var:         [76]
Num of Quant Config:          [104]
FP32:                         [2]
```

### Explanation

My graph has:

- 2 more variables
- 2 more quantization configs
- 2 FP32 configs

This indicates that the ONNX graph exported by my code contains extra intermediate tensors or graph values.

These are likely not real layers such as `Conv` or `Linear`. They are probably graph artifacts introduced by ONNX export, simplification, or conversion.

### Action

Inspect the extra graph values in the ONNX file:

```python
import onnx


def list_graph_values(path):
    model = onnx.load(path)

    print("Inputs:")
    for x in model.graph.input:
        print(" -", x.name)

    print("\nOutputs:")
    for x in model.graph.output:
        print(" -", x.name)

    print("\nInitializers:")
    for x in model.graph.initializer:
        print(" -", x.name)

    print("\nValue infos:")
    for x in model.graph.value_info:
        print(" -", x.name)


list_graph_values("my_model.onnx")
```

Pay special attention to values similar to:

```text
val_149
_v_64
```

---

## Issue 3: ESP-DL Reports Non-Exportable Values

### My result

```text
Skip val_149 because it's not exportable
Skip _v_64 because it's not exportable
```

### Explanation

This means ESP-DL found graph values that are not exportable to its deployment format.

Because the log also reports `FP32: [2]`, these two skipped values are likely related to the two extra FP32 quantization configs.

This can happen when the ONNX graph contains unsupported or unnecessary intermediate values.

### Possible causes

- Different ONNX opset version
- Different PyTorch exporter behavior
- Dynamic axes in ONNX export
- ONNX simplifier adding or preserving extra values
- Unsupported reshape / flatten / identity / constant pattern
- Different PPQ or ESP-DL version

### Action

Try exporting with a fixed batch size first, without dynamic axes:

```python
import torch

model.eval()

example = torch.randn(1, 1, 96, 96)

torch.onnx.export(
    model,
    example,
    "gesture_model_static.onnx",
    input_names=["input"],
    output_names=["output"],
    opset_version=13,
    do_constant_folding=True,
)
```

Avoid this temporarily while debugging:

```python
dynamic_axes={
    "input": {0: "batch_size"},
    "output": {0: "batch_size"},
}
```

For ESP32 deployment, a static batch size is usually better anyway.

---

## Issue 4: Extra `ConvTranspose Decomposition Pass`

### Owner result

```text
PPQ Quantization Fusion Pass Running ...
```

### My result

```text
ConvTranspose Decomposition Pass Running ... Finished.
PPQ Quantization Fusion Pass Running ...
```

### Explanation

My PPQ/ESP-DL pipeline runs an additional pass:

```text
ConvTranspose Decomposition Pass
```

This does not necessarily mean the model has a `ConvTranspose` layer. It may simply mean the current PPQ/ESP-DL version runs this pass by default.

### Action

Check if the ONNX graph actually contains `ConvTranspose`:

```python
import onnx

model = onnx.load("my_model.onnx")

for node in model.graph.node:
    if node.op_type == "ConvTranspose":
        print("Found ConvTranspose:", node.name)
```

If nothing is printed, this pass is harmless.

---

## Issue 5: Quantization Error Is Higher in My Result

### Owner graphwise error

```text
/layers/layers.3/conv/conv.6/Conv: 21.350%
/classifier/classifier.2/Gemm:     19.410%
```

### My graphwise error

```text
node_linear:   34.534%
node_Conv_236: 32.891%
```

### Explanation

My graphwise quantization error is higher in the last layers. This may reduce final accuracy after quantization.

Possible causes:

- Different calibration images
- Different image preprocessing
- Different normalization range
- Different batch ordering
- Different calibration dataloader behavior
- Different ONNX export graph
- Different PPQ / ESP-DL version

### Action

Verify calibration input before quantization:

```python
x = next(iter(cal_loader))
print(x.shape)
print(x.dtype)
print(x.min().item(), x.max().item())
```

Expected for the current model:

```text
torch.Size([32, 1, 96, 96])
torch.float32
approximately 0.0 to 1.0
```

Also make sure training, validation, test, and calibration preprocessing are identical except for augmentation.

---

## Issue 6: Calibration Runtime Is Slower

### Owner result

```text
Calibration Progress(Phase 1): 50/50 [00:02, 18.47it/s]
Calibration Progress(Phase 2): 50/50 [00:03, 13.13it/s]
```

### My result

```text
Calibration Progress(Phase 1): 50/50 [00:06, 7.64it/s]
Calibration Progress(Phase 2): 50/50 [00:08, 5.68it/s]
```

### Explanation

My quantization process is slower. This does not prove an architecture difference.

Possible causes:

- Different machine
- Different CPU/GPU backend
- Different PPQ/ESP-DL version
- Different ONNX graph representation
- Extra graph values
- Different Python environment

### Action

Record package versions:

```bash
python --version
pip freeze | grep -E "torch|onnx|onnxruntime|ppq|esp"
```

---

## Recommended Debugging Checklist

### 1. Compare PyTorch model structure

```python
print(model)
```

Save both outputs and compare them.

### 2. Compare ONNX operator counts

Use:

```python
from collections import Counter
```

and compare `node.op_type`, not `node.name`.

### 3. Export static ONNX

Use fixed input shape:

```text
[1, 1, 96, 96]
```

Avoid dynamic batch while debugging ESP-DL export.

### 4. Simplify ONNX carefully

```bash
python -m onnxsim gesture_model_static.onnx gesture_model_static_sim.onnx
```

Then inspect whether `val_149` and `_v_64` disappear.

### 5. Verify calibration data

Make sure calibration images use the same preprocessing as training/inference.

### 6. Check ESP-DL compatibility

If the graph still contains unsupported values, inspect which node produces them.

```python
import onnx

model = onnx.load("my_model.onnx")

for node in model.graph.node:
    outputs = list(node.output)
    if "val_149" in outputs or "_v_64" in outputs:
        print("Producer node:", node.name, node.op_type)
        print("Inputs:", node.input)
        print("Outputs:", node.output)
```

---

## Current Conclusion

The architecture is probably the same because both logs show:

```text
Num of Op:           27
Num of Quantized Op: 27
```

The difference is likely caused by ONNX export or PPQ/ESP-DL conversion differences:

```text
Owner graph: 74 variables, 102 quant configs
My graph:    76 variables, 104 quant configs, 2 FP32 configs
```

The main issue to fix is:

```text
Skip val_149 because it's not exportable
Skip _v_64 because it's not exportable
```

Start by exporting a static ONNX model, then compare operator counts and inspect the producer nodes of `val_149` and `_v_64`.

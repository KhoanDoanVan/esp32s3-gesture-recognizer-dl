# Softmax Formula Explanation in Embedded / TinyML Context

## Why the softmax formula looks strange

The implementation uses a **numerically stable softmax** combined with **dequantization** of int8 logits.

Standard softmax:

```math
softmax(x_i) = e^{x_i} / sum_j e^{x_j}
```

Implemented version:

```math
softmax(x_i) = e^{x_i - max(x)} / sum_j e^{x_j - max(x)}
```

These are mathematically equivalent because subtracting the same constant from every logit does not change the final probabilities.

---

## Why subtract `max(x)`?

This prevents overflow in the exponential function.

Example logits:

```text
[1000, 999, 998]
```

Direct `exp(1000)` is too large.

Subtract max:

```text
[0, -1, -2]
```

Now:

```text
exp(0)=1
exp(-1)=0.3679
exp(-2)=0.1353
```

Safe and same probability distribution.

---

## Why multiply by scale?

The model output tensor is quantized int8.

Real value reconstruction:

```math
real\_value = int8\_value * 2^{exponent}
```

So before softmax, the logits are converted back to approximate floating-point values.

---

## Full formula used

```math
p_i = e^{(q_i * s) - m} / sum_j e^{(q_j * s) - m}
```

Where:

- `q_i` = int8 logits
- `s = 2^{exponent}`
- `m = max(q_i * s)`

This is standard for embedded ML systems.

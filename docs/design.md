# Design

## Philosophy

DataForge is designed with three key principles:

1. **Minimal abstractions** — each quark is a self-contained operation.  
2. **Composition over inheritance** — pipelines are built by chaining quarks with `|`.  
3. **Performance first** — operations are inlined by the compiler, giving near-zero overhead.

---

## Core Abstractions

- **Quarks** — describe operations (integer encoding, endian conversion, compression).  
- **Iterators** — apply pipelines to input/output data.  
- **Pipelines** — sequences of quarks describing transformations.

---

## Example: Design in Practice

```cpp
auto pipeline = int32 | le | base32;
```

This pipeline:
1. Interprets input as 32-bit integers  
2. Converts them to little-endian  
3. Encodes the result in base32  

The result is an **iterator pipeline** without runtime overhead.

# Quarks

A **quark** is a fundamental building block of a transformation.  
Quarks describe an **operation** (e.g., endian conversion, encoding, encryption) and can be composed into pipelines.

---

## Types of Quarks

### 1. Inline quarks
Predefined, stateless objects used directly:

```cpp
int32   // 32-bit integer
le      // little-endian
be      // big-endian
base32  // base32 encoding
```

Example:

```cpp
int32 | le | base32
```

---

### 2. Stateful quarks
Created with constructors when parameters are required:

```cpp
aes(key, iv, cipher_mode::cbc)
```

---

## Composition

Quarks can be chained using the `|` operator:

```cpp
int16 | be | hex
```

This means:  
1. Convert to 16-bit integer  
2. Apply big-endian transformation  
3. Encode as hexadecimal string

---

## Why "Quark"?

In physics, quarks are fundamental particles that combine to form matter.  
In DataForge, quarks are minimal operations that combine into **data pipelines**.

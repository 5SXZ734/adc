# ADC — Interactive C/C++ Decompiler & Binary Analysis Toolkit

ADC is an interactive disassembler and experimental decompiler focused on making
compiled binary code more understandable to humans.

The primary goal of the project is to analyze executable binaries and transform
selected regions into readable **pseudo-C / pseudo-C++ representations**, exposing
structure, control flow, and data relationships that are otherwise hidden at the
machine level.

ADC targets **Intel 64 and IA-32 (x86)** architectures, with additional disassembly
support for other processor families.

---

## Key Capabilities

### Decompilation & Analysis
- Interactive disassembly and code exploration
- Experimental decompilation into pseudo-C / pseudo-C++ constructs
- Control-flow and data-flow reconstruction
- Early support for C++ concepts such as:
  - encapsulation
  - virtual tables
  - object layout inference
  - partial exception handling recovery

> ADC is not intended to be a full compiler-grade decompiler; it is a research and
> tooling project focused on understanding binaries and recovering intent.

---

### Multi-Architecture Disassembly
While decompilation work is focused on x86/x64, the disassembler core supports
multiple architectures, including:
- x86 / x64
- ARM / ARM64
- MIPS
- PowerPC
- SPARC
- others (via modular backends)

---

### Binary Format Exploration
ADC also functions as a **scriptable binary format viewer**.

Supported formats include:
- PE (Portable Executable)
- ELF
- DWARF
- PDB (partial)

Binaries can be opened and explored in a **browser-like navigation model**, where
structures reference one another via hyperlinks, enabling intuitive inspection of
headers, symbols, sections, and metadata.

---

## Typical Use Cases
- Reverse engineering and binary inspection
- Understanding legacy or undocumented executables
- Exploring compiler output and optimization effects
- Binary format research and tooling
- Education and experimentation with disassembly and decompilation techniques

---

## Architecture Overview
ADC is written primarily in **C++** and emphasizes:
- explicit data structures
- performance-aware design
- separation of parsing, analysis, and presentation layers
- extensibility for new architectures and binary formats

The project prioritizes correctness and transparency over aggressive automation.

---

## Build (Example)
```bash
mkdir build
cd build
cmake ..
cmake --build .

---








## Architecture Overview

## 1. Purpose of the System

The system is an **interactive disassembler and decompiler** supporting **Intel x86/x64**, with additional support for **ARM, ARM64, MIPS, PPC, and SPARC**. It can operate in:

- **Interactive GUI mode** (Qt5)
- **Script-driven CLI batch mode**

Its goals include producing:

- High-quality **disassembly**
- Pseudo-C / pseudo-C++ reconstruction
- Structural recovery (types, vtables, RTTI, exceptions)
- Cross-referenced, hyperlinkable binary navigation

The system exposes a **rich and extensible API** that allows:

- Implementing new **binary format loaders**
- Adding **instruction set decoders**
- Adding **IR-level analyses** or **code generators**
- Integrating into external analysis tools or scripts

---

## 2. High-Level Architecture

```
                 +------------------------------+
                 |            GUI               |
                 |         (Qt5 Frontend)       |
                 +------------------------------+
                              |
                 +------------------------------+
                 |        Core Controller       |
                 |  (IADBCore / IADCCore APIs) |
                 +------------------------------+
                              |
    +-----------+------------+-------------+-------------+
    |           |                          |             |
+--------+  +--------+                +----------+   +--------+
| Front  |  |  DB    |                |   DC     |   | Shared |
| Loader |  | Model  |                |Decompil'r|   | Utils  |
+--------+  +--------+                +----------+   +--------+
  ^             ^                            ^
  |             |                            |
Binary file → Format loader → Typed modules → IR → Pseudo-C/C++
```

### 2.1 Front-End Loaders and Decoders

Frontends perform:

- Format recognition
- Segment parsing
- Symbol and relocation extraction
- Instruction decoding
- IR emission

Key interfaces:
- `I_Front`
- `I_FrontMain`
- `I_FrontDC`
- `I_Code`
- `IPCode_t`

### 2.2 DB (Database / Analysis Model)

The DB represents:

- Types, structs, enums
- Functions, blocks, CFG
- Fields, namespaces, modules
- Address/segment mapping

Key interfaces:
- `I_Module`
- `I_SuperModule`
- `I_ModuleCB`

### 2.3 DC (Decompiler Engine)

Consumes IR and produces:

- Structured control flow
- Type-propagated variables
- High-level pseudo-C/C++

---

## 3. Build & Dependencies

### 3.1 Build System

The system uses **CMake** and produces:

- Executable: `adc`
- Libraries:
  - `shared`, `db`, `dc`, `front`, `uiqt5`

### 3.2 Dependencies

- **Capstone** – multi-ISA disassembler
- **Udis86** – x86 alternative decoder
- **Demanglers** – MSVC/GCC/Itanium name demangling
- **Qt5** – GUI
- **qx** – internal COM-like utility library
- **tiny_process** – process launcher

### 3.3 Build Instructions

```
mkdir build
cd build
cmake ..
cmake --build .
```

---

## 4. Data Flow Summary

1. **Binary → Front-End**
   - Format recognition
   - Segment + symbol loading
   - Instruction decoding → IR

2. **Front-End → DB**
   - Types, namespaces, functions, fields created
   - Segments/subranges registered

3. **DB → Decompiler**
   - IR normalized + structured
   - Types applied
   - High-level output generated

4. **Decompiler → GUI/CLI**
   - View models for binary, source, types, names, etc.
   - Scriptable operations available

---

## Documentation

### Format Loaders
- [PE Loader / Decoder Guide](doc/extended_pe_loader_guide.md)

### Debug / Symbol Formats
- (coming soon) PDB Guide
- (coming soon) DWARF Guide

### Decompiler Internals
- (coming soon) IR Model
- (coming soon) CFG Structuring
- (coming soon) Type Propagation

### Frontend Interfaces
- (coming soon) Frontend API
- (coming soon) DB / Type System API


_End of Architecture Overview_

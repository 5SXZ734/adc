# ADC — Interactive C/C++ Decompiler & Binary Analysis Toolkit

ADC is an interactive disassembler and experimental decompiler focused on making
compiled binary code more understandable to humans.

The primary goal of the project is to analyze executable binaries and transform
selected regions into readable **pseudo-C / pseudo-C++ representations**, exposing
structure, control flow, and data relationships that are otherwise hidden at the
machine level.

ADC targets **Intel 64 and IA-32 (x86)** architectures, with additional disassembly
support for other processor families.

![ADC Screenshot](doc/adc_demo.png)

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

```




## Quick Start — Try the Prebuilt Demo

If you just want to explore ADC without building it from source, you can use the
prebuilt demo binary available on the **Releases** page.

### Steps

1. **Launch ADC**
   
   Run `adc.exe`.

2. **Open a Binary**
   
   From the menu, select:

   **File → New…**

   Choose a binary to analyze. A good starting example is: Qt5Gui.dll


(Yes — ADC can analyze itself or its dependencies.)

3. **Explore Disassembly**

Once the binary is loaded and processed, double-click the `Qt5Gui.dll` entry
in the left-hand project view. The disassembly will open in the main editor.

You can:
- scroll through the disassembly
- use menu commands or keyboard shortcuts
- rename variables, types, and functions
- create new symbols at arbitrary addresses
- change object and type interpretations interactively

4. **Inspect Reconstructed Metadata**

Expand the `Qt5Gui.dll` node in the left view to access additional
reconstructed information, including:
- exports and imports
- type aliases
- strings and resources
- deduced pseudo-C / pseudo-C++ function prototypes
- inferred types (structs, enums, classes)

Much of this information is recovered from the executable’s import/export
tables and associated metadata.

5. **Optional: Load Debug Symbols**

ADC can also open **PDB** files (or **DWARF** data for ELF binaries).
When debug symbols are available and loaded alongside the executable, ADC can
reconstruct significantly richer information, including:
- function and type names
- source file hierarchy
- improved type and structure recovery

Click any item in the project tree to open dedicated sub-views showing
structured data and analysis results.

---

This workflow is designed to be exploratory and interactive: analysis results
can be refined incrementally as you navigate and annotate the binary.





### Multi-Module & Dependency-Aware Analysis (Advanced)

ADC supports **multi-module, interconnected binary analysis**, allowing you to explore
relationships between executables and their statically linked dependencies — a feature
not commonly found in traditional reverse-engineering tools.

In addition to the main module listed in the **Files** view, ADC displays a list of
**static dependencies** (for example, DLLs referenced by the loaded executable).

- Dependencies that are **not yet loaded** appear *greyed out*
- Dependencies that are **loaded and processed** appear in the normal color

You can load any greyed-out dependency from a **Files->Add Module..** menu. Once loaded, the
module is analyzed in the same way as the original binary, and its own dependencies
may also become visible — extending the dependency graph naturally.

This model enables:
- simultaneous analysis of multiple related binaries
- navigation across module boundaries
- inspection of **API call sites between modules**
- exploration of how control flow and data flow propagate across shared interfaces

Using this approach, ADC allows you to study binaries **as a connected system**, rather
than as isolated files, making it easier to understand real-world software layouts
and inter-module behavior.

After loading `Qt5Gui.dll`, open the **Files** menu and select:

**Files → Add Module…**

Use this command to load an additional module into the current analysis session.
For demonstration purposes, you can add: `Qt5Core.dll`




# Architecture Overview

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

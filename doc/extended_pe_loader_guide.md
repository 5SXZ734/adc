# Extended PE Loader Developer Guide

## 1. Introduction

The PE loader of your system is responsible for converting a raw PE file into a fully structured semantic representation:

- Segments & memory layout
- Code & data sections
- Exports & imports
- Relocations
- Type information (DWARF, RTTI, .NET metadata)
- Resources
- Exception handling tables
- TLS
- Symbol tables (COFF, PDB)

The loader communicates with the analysis subsystem (`I_ModuleCB`, `I_SuperModule`) and the frontend IR decoder (`I_Code`). It also interacts with the RTTI decoders and format-specific helpers.

This chapter documents the entire PE loader pipeline in depth.

---

## 2. File Opening & Validation

A PE loader begins with:

1. **DOS Header** (IMAGE_DOS_HEADER)
2. **PE Signature** (offset = e_lfanew)
3. **IMAGE_FILE_HEADER**
4. **IMAGE_OPTIONAL_HEADER** (32/64 bit)
5. **DataDirectories[]**
6. **IMAGE_SECTION_HEADER[]**

The loader must:

- Detect 32 vs 64 bit using OptionalHeader.Magic
- Detect DLL vs EXE
- Extract image base, alignment, and entry point RVA
- Validate section count & header sizes
- Build RVA<->FileOffset mapping tables
- Build VirtualAddress<->Segment mapping

These steps are implemented inside `PE_t<T>`.

---

## 3. Section and Memory Layout Handling

Each `IMAGE_SECTION_HEADER` defines:

- VirtualAddress
- VirtualSize
- PointerToRawData
- SizeOfRawData
- Characteristics

The loader:

1. Creates a segment in `I_SuperModule`:
   ```
   auto seg = mod.NewSegment(sec.VirtualAddress, sec.MemSize);
   ```
2. Links the segment with type information.
3. Registers raw byte readers for the segment.
4. Handles overlay and zero-fill areas.
5. Stores the section → segment mapping table for later relocations.

---

## 4. Export Table Processing

Located in `IMAGE_DIRECTORY_ENTRY_EXPORT`.

Flow:

1. Locate export directory.
2. Validate pointers.
3. Build:
   - Function RVA list
   - Name → RVA mapping
   - Ordinal → RVA mapping
4. Emit via:
   ```
   rICb.dump(va, nameOffset, nameLen, ATTR_EXPORT);
   ```

Handles forwarders and ordinal-only exports.

---

## 5. Import Table Processing

Located in `IMAGE_DIRECTORY_ENTRY_IMPORT`.

Steps:

1. Iterate `IMAGE_IMPORT_DESCRIPTOR` entries.
2. Extract DLL name.
3. For each thunk:
   - Determine name vs ordinal import
   - Emulate IAT/INT logic
   - Emit via `dump()`

Also supports **delayed imports**.

---

## 6. Relocations

From `.reloc` section.

- Parse relocation blocks
- Compute target RVA = PageRVA + Offset
- Handle `HIGHLOW`, `DIR64`, etc.
- Crucial for:
  - RIP-relative decoding
  - VTables
  - Switch tables

---

## 7. TLS (Thread Local Storage)

Extract:

- StartOfRawData
- EndOfRawData
- Callback list

Each callback RVA is declared as a function entry.

---

## 8. Exception Handling & Unwind Info

Important for x64:

- `.pdata` → RUNTIME_FUNCTION
- `.xdata` → UNWIND_INFO

Used to reconstruct:

- EH regions
- Prolog/epilog
- Stack frames
- Non-returning functions

---

## 9. Debug Directory Processing

Types:

### CodeView NB10
- PDB 2.0
- `NB10` signature

### CodeView RSDS
- PDB 7.0
- GUID + age

### COFF Debug Symbols
Processed separately.

### Misc Debug Data
- File paths
- Compiland info

---

## 10. DWARF Debug Information in PE

Rare but supported via `.debug_*` sections.

Loader:

1. Locates DWARF sections
2. Wraps them in `DebugInfo<T>`
3. Extracts:
   - Types
   - Line tables
   - Variables
   - Source paths

---

## 11. Resources (.rsrc)

Parsed as a tree:

- IMAGE_RESOURCE_DIRECTORY
- DIRECTORY_ENTRY
- DATA_ENTRY

Resources extracted:

- Icons
- Version info
- String tables
- Dialogs

---

## 12. .NET Metadata / CLR Directory

If present:

- Read IMAGE_COR20_HEADER
- Extract metadata root:
  - `#~` tables
  - `#Strings`
  - `#Blob`
  - `#GUID`
  - `#US`
- Emit managed methods

---

## 13. COFF Symbol Table

Processes static symbols:

- Reads NumberOfSymbols
- Extracts names (inline or string table)
- Computes VA
- Emits via `dump()`

Used for:
- Function discovery
- Better symbol naming

---

## 14. RTTI: MSVC and GCC

### MSVC RTTI
- `TypeDescriptor`
- `ClassHierarchyDescriptor`
- `BaseClassDescriptor`
- PMD

### GCC RTTI
- `__class_type_info`
- `__si_class_type_info`
- `__vmi_class_type_info`

Both emit inheritance info to the analyzer.

---

## 15. Instruction Decoding Pipeline

### Prefix Processing
- Legacy prefixes
- REX
- VEX/EVEX (if supported)

### Opcode Parsing
- Table-driven decode
- ModR/M, SIB, displacement
- Immediate extraction

### IR Emission
- `IPCode_t`
- Branch/fallthrough
- Dataflow metadata

### Relocation-aware Fixups
- RIP-relative
- Absolute relocations
- Jump tables

---

## 16. Loader → ModuleCB Integration

Important callbacks:

- `I_ModuleCB::dump()`
- `I_ModuleCB::selectFile()`
- `I_SuperModule::NewSegment()`
- `I_SuperModule::AddSubRange()`
- `I_SuperModule::installTypesMgr()`

These build the final analysis DB.

---

## 17. Error Handling & Fault Tolerance

The loader tolerates:

- Invalid raw sizes
- Overlaps
- Missing directories
- Packed binaries

Throws errors only on fatal corruption.

---

## 18. Extending the PE Loader

Possible extension points:

### New Directory Handlers
- Bound imports
- Delay-load directory
- Security directory

### New RTTI Models
- Custom compilers

### New Debug Formats
- New DWARF versions
- Extended CodeView

### Unwinding Logic
- New x64 unwind opcodes

### Managed Code Support
- IL disassembly
- Metadata model extensions

---

# End of Extended PE Loader Developer Guide

set(SRC_FILES_AARCH64
	src/arch/AArch64/AArch64BaseInfo.c
	src/arch/AArch64/AArch64Disassembler.c
	src/arch/AArch64/AArch64InstPrinter.c
	src/arch/AArch64/AArch64Mapping.c
	src/arch/AArch64/AArch64Module.c
	src/arch/AArch64/AArch64AddressingModes.h
	src/arch/AArch64/AArch64BaseInfo.h
	src/arch/AArch64/AArch64Disassembler.h
	src/arch/AArch64/AArch64InstPrinter.h
	src/arch/AArch64/AArch64Mapping.h
	src/arch/AArch64/AArch64Module.h
	src/arch/AArch64/AArch64GenAsmWriter.inc
	src/arch/AArch64/AArch64GenDisassemblerTables.inc
	src/arch/AArch64/AArch64GenInstrInfo.inc
	src/arch/AArch64/AArch64GenRegisterInfo.inc
	src/arch/AArch64/AArch64GenSubtargetInfo.inc
	src/arch/AArch64/AArch64MappingInsn.inc
	src/arch/AArch64/AArch64MappingInsnOp.inc
	src/arch/AArch64/ARMMappingInsnOp.inc
)

set(SRC_FILES_ARM
	src/arch/ARM/ARMDisassembler.c
	src/arch/ARM/ARMInstPrinter.c
	src/arch/ARM/ARMMapping.c
	src/arch/ARM/ARMModule.c
	src/arch/ARM/ARMAddressingModes.h
	src/arch/ARM/ARMBaseInfo.h
	src/arch/ARM/ARMDisassembler.h
	src/arch/ARM/ARMInstPrinter.h
	src/arch/ARM/ARMMapping.h
	src/arch/ARM/ARMModule.h
	src/arch/ARM/ARMGenAsmWriter.inc
	src/arch/ARM/ARMGenDisassemblerTables.inc
	src/arch/ARM/ARMGenInstrInfo.inc
	src/arch/ARM/ARMGenRegisterInfo.inc
	src/arch/ARM/ARMGenSubtargetInfo.inc
	src/arch/ARM/ARMMappingInsn.inc
	src/arch/ARM/ARMMappingInsnOp.inc
)		

set(SRC_FILES_EVM
	src/arch/EVM/EVMDisassembler.c
	src/arch/EVM/EVMInstPrinter.c
	src/arch/EVM/EVMMapping.c
	src/arch/EVM/EVMModule.c
	src/arch/EVM/EVMDisassembler.h
	src/arch/EVM/EVMInstPrinter.h
	src/arch/EVM/EVMMapping.h
	src/arch/EVM/EVMModule.h
	src/arch/EVM/EVMMappingInsn.inc
)

set(SRC_FILES_M680X
	src/arch/M680X/M680XDisassembler.c
	src/arch/M680X/M680XInstPrinter.c
	src/arch/M680X/M680XModule.c
	src/arch/M680X/M680XDisassembler.h
	src/arch/M680X/M680XDisassemblerInternals.h
	src/arch/M680X/M680XInstPrinter.h
	src/arch/M680X/M680XModule.h
	src/arch/M680X/cpu12.inc
	src/arch/M680X/hcs08.inc
	src/arch/M680X/hd6301.inc
	src/arch/M680X/hd6309.inc
	src/arch/M680X/insn_props.inc
	src/arch/M680X/m6800.inc
	src/arch/M680X/m6801.inc
	src/arch/M680X/m6805.inc
	src/arch/M680X/m6808.inc
	src/arch/M680X/m6809.inc
	src/arch/M680X/m6811.inc
)

set(SRC_FILES_M68K
	src/arch/M68K/M68KDisassembler.c
	src/arch/M68K/M68KInstPrinter.c
	src/arch/M68K/M68KModule.c
	src/arch/M68K/M68KDisassembler.h
	src/arch/M68K/M68KInstPrinter.h
	src/arch/M68K/M68KModule.h
	src/arch/M68K/M68KInstructionTable.inc
)

set(SRC_FILES_Mips
	src/arch/Mips/MipsDisassembler.c
	src/arch/Mips/MipsInstPrinter.c
	src/arch/Mips/MipsMapping.c
	src/arch/Mips/MipsModule.c
	src/arch/Mips/MipsDisassembler.h
	src/arch/Mips/MipsInstPrinter.h
	src/arch/Mips/MipsMapping.h
	src/arch/Mips/MipsModule.h
	src/arch/Mips/MipsGenAsmWriter.inc
	src/arch/Mips/MipsGenDisassemblerTables.inc
	src/arch/Mips/MipsGenInstrInfo.inc
	src/arch/Mips/MipsGenRegisterInfo.inc
	src/arch/Mips/MipsGenSubtargetInfo.inc
	src/arch/Mips/MipsMappingInsn.inc
)

set(SRC_FILES_MOS65XX
	src/arch/MOS65XX/MOS65XXDisassembler.c
	src/arch/MOS65XX/MOS65XXModule.c
	src/arch/MOS65XX/MOS65XXDisassembler.h
	src/arch/MOS65XX/MOS65XXModule.h
)

set(SRC_FILES_PowerPC
	src/arch/PowerPC/PPCDisassembler.c
	src/arch/PowerPC/PPCInstPrinter.c
	src/arch/PowerPC/PPCMapping.c
	src/arch/PowerPC/PPCModule.c
	src/arch/PowerPC/PPCDisassembler.h
	src/arch/PowerPC/PPCInstPrinter.h
	src/arch/PowerPC/PPCMapping.h
	src/arch/PowerPC/PPCModule.h
	src/arch/PowerPC/PPCPredicates.h
	src/arch/PowerPC/PPCGenAsmWriter.inc
	src/arch/PowerPC/PPCGenDisassemblerTables.inc
	src/arch/PowerPC/PPCGenInstrInfo.inc
	src/arch/PowerPC/PPCGenRegisterInfo.inc
	src/arch/PowerPC/PPCGenSubtargetInfo.inc
	src/arch/PowerPC/PPCMappingInsn.inc
)

set(SRC_FILES_Sparc
	src/arch/Sparc/SparcDisassembler.c
	src/arch/Sparc/SparcInstPrinter.c
	src/arch/Sparc/SparcMapping.c
	src/arch/Sparc/SparcModule.c
	src/arch/Sparc/Sparc.h
	src/arch/Sparc/SparcDisassembler.h
	src/arch/Sparc/SparcInstPrinter.h
	src/arch/Sparc/SparcMapping.h
	src/arch/Sparc/SparcModule.h
	src/arch/Sparc/SparcGenAsmWriter.inc
	src/arch/Sparc/SparcGenDisassemblerTables.inc
	src/arch/Sparc/SparcGenInstrInfo.inc
	src/arch/Sparc/SparcGenRegisterInfo.inc
	src/arch/Sparc/SparcGenSubtargetInfo.inc
	src/arch/Sparc/SparcMappingInsn.inc
)

set(SRC_FILES_SystemZ
	src/arch/SystemZ/SystemZDisassembler.c
	src/arch/SystemZ/SystemZInstPrinter.c
	src/arch/SystemZ/SystemZMapping.c
	src/arch/SystemZ/SystemZMCTargetDesc.c
	src/arch/SystemZ/SystemZModule.c
	src/arch/SystemZ/SystemZDisassembler.h
	src/arch/SystemZ/SystemZInstPrinter.h
	src/arch/SystemZ/SystemZMapping.h
	src/arch/SystemZ/SystemZMCTargetDesc.h
	src/arch/SystemZ/SystemZModule.h
	src/arch/SystemZ/SystemZGenAsmWriter.inc
	src/arch/SystemZ/SystemZGenDisassemblerTables.inc
	src/arch/SystemZ/SystemZGenInsnNameMaps.inc
	src/arch/SystemZ/SystemZGenInstrInfo.inc
	src/arch/SystemZ/SystemZGenRegisterInfo.inc
	src/arch/SystemZ/SystemZGenSubtargetInfo.inc
	src/arch/SystemZ/SystemZMappingInsn.inc
)

set(SRC_FILES_TMS320C64x
	src/arch/TMS320C64x/TMS320C64xDisassembler.c
	src/arch/TMS320C64x/TMS320C64xInstPrinter.c
	src/arch/TMS320C64x/TMS320C64xMapping.c
	src/arch/TMS320C64x/TMS320C64xModule.c
	src/arch/TMS320C64x/TMS320C64xDisassembler.h
	src/arch/TMS320C64x/TMS320C64xInstPrinter.h
	src/arch/TMS320C64x/TMS320C64xMapping.h
	src/arch/TMS320C64x/TMS320C64xModule.h
	src/arch/TMS320C64x/TMS320C64xGenAsmWriter.inc
	src/arch/TMS320C64x/TMS320C64xGenDisassemblerTables.inc
	src/arch/TMS320C64x/TMS320C64xGenInstrInfo.inc
	src/arch/TMS320C64x/TMS320C64xGenRegisterInfo.inc
)

set(SRC_FILES_X86
	src/arch/X86/X86ATTInstPrinter.c
	src/arch/X86/X86Disassembler.c
	src/arch/X86/X86DisassemblerDecoder.c
	src/arch/X86/X86IntelInstPrinter.c
	src/arch/X86/X86Mapping.c
	src/arch/X86/X86Module.c
	src/arch/X86/qsort.h
	src/arch/X86/X86BaseInfo.h
	src/arch/X86/X86Disassembler.h
	src/arch/X86/X86DisassemblerDecoder.h
	src/arch/X86/X86DisassemblerDecoderCommon.h
	src/arch/X86/X86InstPrinter.h
	src/arch/X86/X86Mapping.h
	src/arch/X86/X86Module.h
	src/arch/X86/X86GenAsmWriter.inc
	src/arch/X86/X86GenAsmWriter_reduce.inc
	src/arch/X86/X86GenAsmWriter1.inc
	src/arch/X86/X86GenAsmWriter1_reduce.inc
	src/arch/X86/X86GenDisassemblerTables.inc
	src/arch/X86/X86GenDisassemblerTables_reduce.inc
	src/arch/X86/X86GenInstrInfo.inc
	src/arch/X86/X86GenInstrInfo_reduce.inc
	src/arch/X86/X86GenRegisterInfo.inc
	src/arch/X86/X86ImmSize.inc
	src/arch/X86/X86MappingInsn.inc
	src/arch/X86/X86MappingInsn_reduce.inc
	src/arch/X86/X86MappingInsnOp.inc
	src/arch/X86/X86MappingInsnOp_reduce.inc
)

set(SRC_FILES_XCore
	src/arch/XCore/XCoreDisassembler.c
	src/arch/XCore/XCoreInstPrinter.c
	src/arch/XCore/XCoreMapping.c
	src/arch/XCore/XCoreModule.c
	src/arch/XCore/XCoreDisassembler.h
	src/arch/XCore/XCoreInstPrinter.h
	src/arch/XCore/XCoreMapping.h
	src/arch/XCore/XCoreModule.h
	src/arch/XCore/XCoreGenAsmWriter.inc
	src/arch/XCore/XCoreGenDisassemblerTables.inc
	src/arch/XCore/XCoreGenInstrInfo.inc
	src/arch/XCore/XCoreGenRegisterInfo.inc
	src/arch/XCore/XCoreMappingInsn.inc
)

set(SRC_FILES
	src/cs.c
	src/MCInst.c
	src/MCInstrDesc.c
	src/MCRegisterInfo.c
	src/SStream.c
	src/utils.c
	src/cs_priv.h
	src/LEB128.h
	src/MathExtras.h
	src/MCDisassembler.h
	src/MCFixedLenDisassembler.h
	src/MCInst.h
	src/MCInstrDesc.h
	src/MCRegisterInfo.h
	src/SStream.h
	src/utils.h
	${SRC_FILES_AARCH64}
	${SRC_FILES_ARM}
	${SRC_FILES_EVM}
	${SRC_FILES_M680X}
	${SRC_FILES_M68K}
	${SRC_FILES_Mips}
	${SRC_FILES_MOS65XX}
	${SRC_FILES_PowerPC}
	${SRC_FILES_Sparc}
	${SRC_FILES_SystemZ}
	${SRC_FILES_TMS320C64x}
	${SRC_FILES_X86}
	${SRC_FILES_XCore}
)

set(H_FILES
	include/capstone/arm.h
	include/capstone/arm64.h
	include/capstone/capstone.h
	include/capstone/evm.h
	include/capstone/m680x.h
	include/capstone/m68k.h
	include/capstone/mips.h
	include/capstone/mos65xx.h
	include/capstone/platform.h
	include/capstone/ppc.h
	include/capstone/sparc.h
	include/capstone/systemz.h
	include/capstone/tms320c64x.h
	include/capstone/x86.h
	include/capstone/xcore.h
)

source_group("src" FILES ${SRC_FILES})
source_group("include" FILES ${H_FILES})
source_group("AARCH64" FILES ${SRC_FILES_AARCH64})
source_group("ARM" FILES ${SRC_FILES_ARM})
source_group("EVM" FILES ${SRC_FILES_EVM})
source_group("M680X" FILES ${SRC_FILES_M680X})
source_group("M68K" FILES ${SRC_FILES_M68K})
source_group("Mips" FILES ${SRC_FILES_Mips})
source_group("MOS65XX" FILES ${SRC_FILES_MOS65XX})
source_group("PowerPC" FILES ${SRC_FILES_PowerPC})
source_group("Sparc" FILES ${SRC_FILES_Sparc})
source_group("SystemZ" FILES ${SRC_FILES_SystemZ})
source_group("TMS320C64x" FILES ${SRC_FILES_TMS320C64x})
source_group("X86" FILES ${SRC_FILES_X86})
source_group("XCore" FILES ${SRC_FILES_XCore})

add_definitions(
	-DCAPSTONE_X86_ATT_DISABLE_NO
	-DCAPSTONE_DIET_NO
	-DCAPSTONE_X86_REDUCE_NO
	-DCAPSTONE_HAS_ARM
	-DCAPSTONE_HAS_EVM
	-DCAPSTONE_HAS_ARM64
	-DCAPSTONE_HAS_M68K
	-DCAPSTONE_HAS_MIPS
	-DCAPSTONE_HAS_POWERPC
	-DCAPSTONE_HAS_SPARC
	-DCAPSTONE_HAS_SYSZ
	-DCAPSTONE_HAS_X86
	-DCAPSTONE_HAS_XCORE
	-DCAPSTONE_USE_SYS_DYN_MEM
	-DCAPSTONE_SHARED
)

add_library(capstone SHARED)
target_sources(capstone PRIVATE ${SRC_FILES} ${H_FILES})

target_include_directories(capstone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(capstone PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(capstone PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/capstone)

install(TARGETS capstone RUNTIME DESTINATION ${INSTALL_SUBDIR})


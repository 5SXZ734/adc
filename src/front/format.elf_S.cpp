#include "shared.h"
#include "format.elf.h"
//#include "decode.elf.h"


void ELF_Strucs_t::createElfStructures(I_Module& mr)
{
	bool b64bit(m64bit);// Is64bit());

	if (mr.NewScope("ELFCLASS", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NONE", 0);// Invalid class
		mr.declEField("_32BIT", 1);// 32-bit objects
		mr.declEField("_64BIT", 2);// 64-bit objects
	}

	if (mr.NewScope("ELFDATA", SCOPE_ENUM))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NONE", 0);// Invalid data encoding
		mr.declEField("LSB", 1);
		mr.declEField("MSB", 2);
	}

	if (mr.NewScope("Elf_ET", SCOPE_ENUM))//ET_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NONE", 0);
		mr.declEField("REL", 1);
		mr.declEField("EXEC", 2);
		mr.declEField("DYN", 3);
		mr.declEField("CORE", 4);
		mr.declEField("LOPROC", 0xFF00);
		mr.declEField("HIPROC", 0xFFFF);
	}

	if (mr.NewScope("Elf_EM", SCOPE_ENUM))//EM_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NONE", 0);
		mr.declEField("M32", 1);
		mr.declEField("SPARC", 2);
		mr.declEField("_386", 3);//!
		mr.declEField("_68K", 4);//!
		mr.declEField("_88K", 5);
		mr.declEField("_860", 7);
		mr.declEField("MIPS", 8);
		mr.declEField("S370", 9);
		mr.declEField("MIPS_RS3_LE", 10);
		//RESERVED	11 - 14	Reserved for future use
		mr.declEField("PARISC", 15);
		//RESERVED	16	Reserved for future use
		mr.declEField("VPP500", 17);
		mr.declEField("SPARC32PLUS", 18);
		mr.declEField("_960", 19);//!
		mr.declEField("PPC", 20);
		mr.declEField("PPC64", 21);
		mr.declEField("S390", 22);
		mr.declEField("SPU", 23);
		//RESERVED	24 - 35	Reserved for future use
		mr.declEField("V800", 36);
		mr.declEField("FR20", 37);
		mr.declEField("RH32", 38);
		mr.declEField("RCE", 39);
		mr.declEField("ARM", 40);
		mr.declEField("ALPHA", 41);
		mr.declEField("SH", 42);
		mr.declEField("SPARCV9", 43);
		mr.declEField("TRICORE", 44);
		mr.declEField("ARC", 45);
		mr.declEField("H8_300", 46);
		mr.declEField("H8_300H", 47);
		mr.declEField("H8S", 48);
		mr.declEField("H8_500", 49);
		mr.declEField("IA_64", 50);
		mr.declEField("MIPS_X", 51);
		mr.declEField("COLDFIRE", 52);
		mr.declEField("_68HC12", 53);//!
		mr.declEField("MMA", 54);
		mr.declEField("PCP", 55);
		mr.declEField("NCPU", 56);
		mr.declEField("NDR1", 57);
		mr.declEField("STARCORE", 58);
		mr.declEField("ME16", 59);
		mr.declEField("ST100", 60);
		mr.declEField("TINYJ", 61);
		mr.declEField("X86_64", 62);
		mr.declEField("PDSP", 63);
		mr.declEField("PDP10", 64);
		mr.declEField("PDP11", 65);
		mr.declEField("FX66", 66);
		mr.declEField("ST9PLUS", 67);
		mr.declEField("ST7", 68);
		mr.declEField("_68HC16", 69);//!
		mr.declEField("_68HC11", 70);//!
		mr.declEField("_68HC08", 71);//!
		mr.declEField("_68HC05", 72);//!
		mr.declEField("SVX", 73);
		mr.declEField("ST19", 74);
		mr.declEField("VAX", 75);
		mr.declEField("CRIS", 76);
		mr.declEField("JAVELIN", 77);
		mr.declEField("FIREPATH", 78);
		mr.declEField("ZSP", 79);
		mr.declEField("MMIX", 80);
		mr.declEField("HUANY", 81);
		mr.declEField("PRISM", 82);
		mr.declEField("AVR", 83);
		mr.declEField("FR30", 84);
		mr.declEField("D10V", 85);
		mr.declEField("D30V", 86);
		mr.declEField("V850", 87);
		mr.declEField("M32R", 88);
		mr.declEField("MN10300", 89);
		mr.declEField("MN10200", 90);
		mr.declEField("PJ", 91);
		mr.declEField("OPENRISC", 92);
		mr.declEField("ARC_COMPACT", 93);
		mr.declEField("XTENSA", 94);
		mr.declEField("VIDEOCORE", 95);
		mr.declEField("TMM_GPP", 96);
		mr.declEField("NS32K", 97);
		mr.declEField("TPC", 98);
		mr.declEField("SNP1K", 99);
		mr.declEField("ST200", 100);
		mr.declEField("IP2K", 101);
		mr.declEField("MAX", 102);
		mr.declEField("CR", 103);
		mr.declEField("F2MC16", 104);
		mr.declEField("MSP430", 105);
		mr.declEField("BLACKFIN", 106);
		mr.declEField("SE_C33", 107);
		mr.declEField("SEP", 108);
		mr.declEField("ARCA", 109);
		mr.declEField("UNICORE", 110);
		mr.declEField("EXCESS", 111);
		mr.declEField("DXP", 112);
		mr.declEField("ALTERA_NIOS2", 113);
		mr.declEField("CRX", 114);
		mr.declEField("XGATE", 115);
		mr.declEField("C166", 116);
		mr.declEField("M16C", 117);
		mr.declEField("DSPIC30F", 118);
		mr.declEField("CE", 119);
		mr.declEField("M32C", 120);
		//reserved	121 - 130	Reserved for future use
		mr.declEField("TSK3000", 131);
		mr.declEField("RS08", 132);
		mr.declEField("SHARC", 133);
		mr.declEField("ECOG2", 134);
		mr.declEField("SCORE7", 135);
		mr.declEField("DSP24", 136);
		mr.declEField("VIDEOCORE3", 137);
		mr.declEField("LATTICEMICO32", 138);
		mr.declEField("SE_C17", 139);
		mr.declEField("TI_C6000", 140);
		mr.declEField("TI_C2000", 141);
		mr.declEField("TI_C5500", 142);
		mr.declEField("TI_ARP32", 143);
		mr.declEField("TI_PRU", 144);
		//reserved	145 - 159	Reserved for future use
		mr.declEField("MMDSP_PLUS", 160);
		//reserved	145 - 159	Reserved for future use
		mr.declEField("MMDSP_PLUS", 160);
		mr.declEField("CYPRESS_M8C", 161);
		mr.declEField("R32C", 162);
		mr.declEField("TRIMEDIA", 163);
		mr.declEField("QDSP6", 164);
		mr.declEField("_8051", 165);//!
		mr.declEField("STXP7X", 166);
		mr.declEField("NDS32", 167);
		mr.declEField("ECOG1", 168);
		mr.declEField("ECOG1X", 168);
		mr.declEField("MAXQ30", 169);
		mr.declEField("XIMO16", 170);
		mr.declEField("MANIK", 171);
		mr.declEField("CRAYNV2", 172);
		mr.declEField("RX", 173);
		mr.declEField("METAG", 174);
		mr.declEField("MCST_ELBRUS", 175);
		mr.declEField("ECOG16", 176);
		mr.declEField("CR16", 177);
		mr.declEField("ETPU", 178);
		mr.declEField("SLE9X", 179);
		mr.declEField("L10M", 180);
		mr.declEField("K10M", 181);
		//reserved	182	Reserved for future Intel use
		mr.declEField("AARCH64", 183);
		//reserved	184	Reserved for future ARM use
		mr.declEField("AVR32", 185);
		mr.declEField("STM8", 186);
		mr.declEField("TILE64", 187);
		mr.declEField("TILEPRO", 188);
		mr.declEField("MICROBLAZE", 189);
		mr.declEField("CUDA", 190);
		mr.declEField("TILEGX", 191);
		mr.declEField("CLOUDSHIELD", 192);
		mr.declEField("COREA_1ST", 193);
		mr.declEField("COREA_2ND", 194);
		mr.declEField("ARC_COMPACT2", 195);
		mr.declEField("OPEN8", 196);
		mr.declEField("RL78", 19);
		mr.declEField("VIDEOCORE5", 198);
		mr.declEField("_78KOR", 199);//!
		mr.declEField("_56800EX", 200);//!
		mr.declEField("BA1", 201);
		mr.declEField("BA2", 202);
		mr.declEField("XCORE", 203);
		mr.declEField("MCHP_PIC", 204);
		mr.declEField("INTEL205", 205);
		mr.declEField("INTEL206", 206);
		mr.declEField("INTEL207", 207);
		mr.declEField("INTEL208", 208);
		mr.declEField("INTEL209", 209);
		mr.declEField("KM32", 210);
		mr.declEField("KMX32", 211);
		mr.declEField("KMX16", 212);
		mr.declEField("KMX8", 213);
		mr.declEField("KVARC", 214);
		mr.declEField("CDP", 215);
		mr.declEField("COGE", 216);
		mr.declEField("COOL", 217);
		mr.declEField("NORC", 218);
		mr.declEField("CSR_KALIMBA", 219);
		mr.declEField("Z80", 220);
		mr.declEField("VISIUM", 221);
		mr.declEField("FT32", 222);
		mr.declEField("MOXIE", 223);
		mr.declEField("AMDGPU", 224);
		//225 - 242
		mr.declEField("RISCV", 243);
	}

	if (mr.NewScope("Elf_NIdent"))//EI_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("MAG", mr.arrayOf(mr.type(_char), 4));
		mr.declField("CLASS", mr.enumOf(mr.type("ELFCLASS"), _uchar));
		mr.declField("DATA", mr.enumOf(mr.type("ELFDATA"), _uchar));
		mr.declField("VERSION", mr.type(_uchar));
		mr.declField("PAD", mr.arrayOf(mr.type(_uchar), 9), ATTR_COLLAPSED);
	}

	if (mr.NewScope("Elf_SHT", SCOPE_ENUM))//SHT_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NULL", 0);
		mr.declEField("PROGBITS", 1);
		mr.declEField("SYMTAB", 2);
		mr.declEField("STRTAB", 3);
		mr.declEField("RELA", 4);
		mr.declEField("HASH", 5);
		mr.declEField("DYNAMIC", 6);
		mr.declEField("NOTE", 7);
		mr.declEField("NOBITS", 8);
		mr.declEField("REL", 9);
		mr.declEField("SHLIB", 10);
		mr.declEField("DYNSYM", 11);
		mr.declEField("INIT_ARRAY", 14);
		mr.declEField("FINI_ARRAY", 15);
		mr.declEField("PREINIT_ARRAY", 16);
		mr.declEField("GROUP", 17);
		mr.declEField("SYMTAB_SHNDX", 18);
		mr.declEField("NUM", 19);
		// GNU extensions
		mr.declEField("GNU_verdef", 0x6ffffffd);
		mr.declEField("GNU_verneed", 0x6ffffffe);
		mr.declEField("GNU_versym", 0x6fffffff);
		//mr.declEField("LOPROC", 0x70000000);
		//mr.declEField("HIPROC", 0x7fffffff);
		//mr.declEField("LOUSER", 0x80000000);
		//mr.declEField("HIUSER", 0xffffffff);
	}

	/* Special section indices.  */
	if (mr.NewScope("ELF_SHN", SCOPE_ENUM))//SHN_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("UNDEF", 0);		/* Undefined section */
		mr.declEField("LORESERVE", 0xff00);		/* Start of reserved indices */
		mr.declEField("LOPROC", 0xff00);		/* Start of processor-specific */
		mr.declEField("BEFORE", 0xff00);		/* Order section before all others (Solaris).  */
		mr.declEField("AFTER", 0xff01);		/* Order section after all others (Solaris).  */
		mr.declEField("HIPROC", 0xff1f);		/* End of processor-specific */
		mr.declEField("LOOS", 0xff20);		/* Start of OS-specific */
		mr.declEField("HIOS", 0xff3f);		/* End of OS-specific */
		mr.declEField("ABS", 0xfff1);		/* Associated symbol is absolute */
		mr.declEField("COMMON", 0xfff2);		/* Associated symbol is common */
		mr.declEField("XINDEX", 0xffff);		/* Index is in extra table.  */
		mr.declEField("HIRESERVE", 0xffff);		/* End of reserved indices */
	}

	if (mr.NewScope("ELF_STT", SCOPE_ENUM))//STT_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NOTYPE", 0);
		mr.declEField("OBJECT", 1);
		mr.declEField("FUNC", 2);
		mr.declEField("SECTION", 3);
		mr.declEField("FILE", 4);
		mr.declEField("LOPROC", 13);
		mr.declEField("HIPROC", 15);
	}

	if (mr.NewScope("ELF_STB", SCOPE_ENUM))//STB_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("LOCAL", 0);
		mr.declEField("GLOBAL", 1);
		mr.declEField("WEAK", 2);
		mr.declEField("LOPROC", 13);
		mr.declEField("HIPROC", 15);
	}

	if (mr.NewScope("ELF_PT", SCOPE_ENUM))//PT_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NULL", 0);
		mr.declEField("LOAD", 1);
		mr.declEField("DYNAMIC", 2);
		mr.declEField("INTERP", 3);
		mr.declEField("NOTE", 4);
		mr.declEField("SHLIB", 5);
		mr.declEField("PHDR", 6);
		mr.declEField("TLS", 7);
		//mr.declEField("LOOS", 0x60000000);
		// x86-64 program header types.
		mr.declEField("SUNW_UNWIND", 0x6464e550);
		mr.declEField("SUNW_EH_FRAME", 0x6474e550);
		mr.declEField("GNU_STACK", 0x6474e551);
		mr.declEField("GNU_RELRO", 0x6474e552);
		mr.declEField("OPENBSD_RANDOMIZE", 0x65a3dbe6);
		mr.declEField("OPENBSD_WXNEEDED", 0x65a3dbe7);
		mr.declEField("OPENBSD_BOOTDATA", 0x65a41be6);
		mr.declEField("LOSUNW", 0x6ffffffa);
		mr.declEField("SUNWBSS", 0x6ffffffa);
		mr.declEField("SUNWSTACK", 0x6ffffffb);
		mr.declEField("SUNWDTRACE", 0x6ffffffc);
		mr.declEField("SUNWCAP", 0x6ffffffd);
		mr.declEField("HISUNW", 0x6fffffff);
		//mr.declEField("HIOS", 0x6fffffff);
		//mr.declEField("LOPROC", 0x70000000);
		 // ARM program header types.
		mr.declEField("ARM_ARCHEXT", 0x70000000);
		// These all contain stack unwind tables.
		mr.declEField("ARM_EXIDX", 0x70000001);
		mr.declEField("ARM_UNWIND", 0x70000001);
		// MIPS program header types.
		//mr.declEField("MIPS_REGINFO", 0x70000000);
		//mr.declEField("MIPS_RTPROC", 0x70000001);
		//mr.declEField("MIPS_OPTIONS", 0x70000002);
		//mr.declEField("MIPS_ABIFLAGS", 0x70000003);
		//mr.declEField("HIPROC", 0x7fffffff);
	}

	if (mr.NewScope("ELF_PF"))//PF_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("X", mr.type(_Word));//0x1
		mr.declBField("W", mr.type(_Word));//0x2
		mr.declBField("R", mr.type(_Word));//0x4
		//PF_MASKOS = 0x0ff00000,  // Bits for operating system-specific semantics.
		//PF_MASKPROC = 0xf0000000 // Bits for processor-specific semantics.
		mr.skipBits(CHAR_BIT * sizeof(Elf_Word) - 3);
	}

	if (mr.NewScope("ELF_DT", SCOPE_ENUM))//DT_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NULL", 0);
		mr.declEField("NEEDED", 1);
		mr.declEField("PLTRELSZ", 2);
		mr.declEField("PLTGOT", 3);
		mr.declEField("HASH", 4);
		mr.declEField("STRTAB", 5);
		mr.declEField("SYMTAB", 6);
		mr.declEField("RELA", 7);
		mr.declEField("RELASZ", 8);
		mr.declEField("RELAENT", 9);
		mr.declEField("STRSZ", 10);
		mr.declEField("SYMENT", 11);
		mr.declEField("INIT", 12);
		mr.declEField("FINI", 13);
		mr.declEField("SONAME", 14);
		mr.declEField("RPATH", 15);
		mr.declEField("SYMBOLIC", 16);
		mr.declEField("REL", 17);
		mr.declEField("RELSZ", 18);
		mr.declEField("RELENT", 19);
		mr.declEField("PLTREL", 20);
		mr.declEField("DEBUG", 21);
		mr.declEField("TEXTREL", 22);
		mr.declEField("JMPREL", 23);
		mr.declEField("BIND_NOW", 24);
		mr.declEField("INIT_ARRAY", 25);
		mr.declEField("FINI_ARRAY", 26);
		mr.declEField("INIT_ARRAYSZ", 27);
		mr.declEField("FINI_ARRAYSZ", 28);
		//mr.declEField("LOOS", 0x60000000);
		mr.declEField("LOOS", 0x6000000d);
		mr.declEField("HIOS", 0x6ffff000);
		mr.declEField("VALRNGLO", 0x6ffffd00);
		mr.declEField("VALRNGHI", 0x6ffffdff);
		mr.declEField("ADDRRNGLO", 0x6ffffe00);
		mr.declEField("ADDRRNGHI", 0x6ffffeff);
		mr.declEField("VERSYM", 0x6ffffff0);
		mr.declEField("RELACOUNT", 0x6ffffff9);
		mr.declEField("RELCOUNT", 0x6ffffffa);
		mr.declEField("FLAGS_1", 0x6ffffffb);
		mr.declEField("VERDEF", 0x6ffffffc);
		mr.declEField("VERDEFNUM", 0x6ffffffd);
		mr.declEField("VERNEED", 0x6ffffffe);
		mr.declEField("VERNEEDNUM", 0x6fffffff);
		//mr.declEField("HIOS", 0x6FFFFFFF);
		//mr.declEField("LOPROC", 0x70000000);
		//mr.declEField("HIPROC", 0x7FFFFFFF);
	}

	if (mr.NewScope("ELF_R_386", SCOPE_ENUM))//ELF_R_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declEField("NONE", 0);
		mr.declEField("_32", 1);//!
		mr.declEField("PC32", 2);
		mr.declEField("GOT32", 3);
		mr.declEField("PLT32", 4);
		mr.declEField("COPY", 5);
		mr.declEField("GLOB_DAT", 6);
		mr.declEField("JMP_SLOT", 7);
		mr.declEField("RELATIVE", 8);
		mr.declEField("GOTOFF", 9);
		mr.declEField("GOTPC", 10);
		mr.declEField("32PLT", 11);
		mr.declEField("TLS_TPOFF", 14);
		mr.declEField("TLS_IE", 15);
		mr.declEField("TLS_GOTIE", 16);
		mr.declEField("TLS_LE", 17);
		mr.declEField("TLS_GD", 18);
		mr.declEField("TLS_LDM", 19);
		mr.declEField("_16", 20);//!
		mr.declEField("PC16", 21);
		mr.declEField("_8", 22);//!
		mr.declEField("PC8", 23);
		mr.declEField("TLS_GD_32", 24);
		mr.declEField("TLS_GD_PUSH", 25);
		mr.declEField("TLS_GD_POP", 27);
		mr.declEField("TLS_LDM_32", 28);
		mr.declEField("TLS_LDM_PUSH", 29);
		mr.declEField("TLS_LDM_CALL", 30);
		mr.declEField("TLS_LDM_POP", 31);
		mr.declEField("TLS_LDO_32", 32);
		mr.declEField("TLS_IE_32", 33);
		mr.declEField("TLS_LE_32", 34);
		mr.declEField("TLS_DTPMOD32", 35);
		mr.declEField("TLS_DTPOFF32", 36);
		mr.declEField("TLS_TPOFF32", 37);
		mr.declEField("SIZE32", 38);
		mr.declEField("TLS_GOTDESC", 39);
		mr.declEField("TLS_DESC_CALL", 40);
		mr.declEField("TLS_DESC", 41);
		mr.declEField("IRELATIVE", 42);
		mr.declEField("GOT32X", 43);
		//mr.declEField("NUM", 44);
	}

	if (mr.NewScope("Elf_Verdef"))//vd_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("version", mr.type(_Half));
		mr.declField("flags", mr.type(_Half));
		mr.declField("ndx", mr.type(_Half));
		mr.declField("cnt", mr.type(_Half));
		mr.declField("hash", mr.type(_Word));
		mr.declField("aux", mr.type(_Word));
		mr.declField("next", mr.type(_Word));
	}
	if (mr.NewScope("Elf_Verdaux"))//_vda
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("name", mr.type(_Word), (AttrIdEnum)ATTR_ELF_STRING_INDEX_DYNSYM);
		mr.declField("next", mr.type(_Word));
	}
	if (mr.NewScope("Elf_Verneed"))//vn_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("version", mr.type(_Half));
		mr.declField("cnt", mr.type(_Half), ATTR_DECIMAL);
		mr.declField("file", mr.type(_Word), (AttrIdEnum)ATTR_ELF_STRING_INDEX_DYNSYM);
		mr.declField("aux", mr.type(_Word));
		mr.declField("next", mr.type(_Word));
	}
	if (mr.NewScope("Elf_Vernaux"))//vna_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("hash", mr.type(_Word));
		mr.declField("flags", mr.type(_Half));
		mr.declField("other", mr.type(_Half), ATTR_DECIMAL);
		mr.declField("name", mr.type(_Word), (AttrIdEnum)ATTR_ELF_STRING_INDEX_DYNSYM);
		mr.declField("next", mr.type(_Word));
	}

	if (b64bit)
		create64bitStructures(mr);
	else
		create32bitStructures(mr);
}

void ELF_Strucs_t::create32bitStructures(I_Module& mr)
{
	if (mr.NewScope("Elf32_Ehdr"))//e_
	{
		SAFE_SCOPE_HERE(mr);
		//mr.declField("e_ident", mr.arrayOf(mr.type(_uchar), EI_NIDENT));
		mr.declField("ident", mr.type("Elf_NIdent"));
		mr.declField("type", mr.enumOf(mr.type("Elf_ET"), _Half));
		mr.declField("machine", mr.enumOf(mr.type("Elf_EM"), _Half));
		mr.declField("version", mr.type(_Word));
		mr.declField("entry", mr.type(_Addr), ATTR_VA);
		mr.declField("phoff", mr.type(_Off));
		mr.declField("shoff", mr.type(_Off), ATTR_FILEPTR);
		mr.declField("flags", mr.type(_Word));
		mr.declField("ehsize", mr.type(_Half));
		mr.declField("phentsize", mr.type(_Half));
		mr.declField("phnum", mr.type(_Half));
		mr.declField("shentsize", mr.type(_Half));
		mr.declField("shnum", mr.type(_Half));
		mr.declField("shstrndx", mr.type(_Half));
	}

	if (mr.NewScope("Elf32_SHF"))//SHF_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("WRITE", mr.type(_Word));//0x1
		mr.declBField("ALLOC", mr.type(_Word));//0x2
		mr.declBField("EXECINSTR", mr.type(_Word));//0x4
		//mr.setSize(CHAR_BIT * sizeof(Elf_Word));//?
		mr.skipBits(CHAR_BIT * sizeof(Elf_Word) - 3);
	}

	if (mr.NewScope("Elf32_Shdr"))//sh_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("name", mr.type(_Word), (AttrIdEnum)ATTR_ELF_STRING_INDEX0);
		mr.declField("type", mr.enumOf(mr.type("Elf_SHT"), _Word));
		mr.declField("flags", mr.type("Elf32_SHF"), ATTR_COLLAPSED);
		mr.declField("addr", mr.type(_Addr), ATTR_VA);
		mr.declField("offset", mr.type(_Off), ATTR_FILEPTR);
		mr.declField("size", mr.type(_Word));
		mr.declField("link", mr.type(_Word), (AttrIdEnum)ATTR_ELF_SECTION_HEADER);//_Word
		mr.declField("info", mr.type(_Word));
		mr.declField("addralign", mr.type(_Word));
		mr.declField("entsize", mr.type(_Word));
	}

	if (mr.NewScope("Elf32_Sym"))//st_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("name", mr.type(_Word), (AttrIdEnum)ATTR_ELF_OFF_LNK_SEC_VIA_FP);// ATTR_ELF_STRING_INDEX_DYNSYM);
		mr.declField("value", mr.type(_Addr), ATTR_VA);
		mr.declField("size", mr.type(_Word));
		mr.declBField("type", mr.arrayOf(mr.enumOf(mr.type("ELF_STT"), _uchar), 4));
		mr.declBField("bind", mr.arrayOf(mr.enumOf(mr.type("ELF_STB"), _uchar), 4));
		mr.declField("other", mr.type(_uchar));
		mr.declField("shndx", mr.enumOf(mr.type("ELF_SHN"), _Half), (AttrIdEnum)ATTR_ELF_SECTION_HEADER);
	}

	if (mr.NewScope("Elf32_Phdr"))//p_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("type", mr.enumOf(mr.type("ELF_PT"), _Word));
		mr.declField("offset", mr.type(_Off), ATTR_FILEPTR);
		mr.declField("vaddr", mr.type(_Addr), ATTR_VA);
		mr.declField("paddr", mr.type(_Addr), ATTR_VA);
		mr.declField("filesz", mr.type(_Word));
		mr.declField("memsz", mr.type(_Word));
		mr.declField("flags", mr.type("ELF_PF"), ATTR_COLLAPSED);
		mr.declField("align", mr.type(_Word));
	};

	if (mr.NewScope("Elf32_Dyn"))//d_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("tag", mr.enumOf(mr.type("ELF_DT"), _SWord));
		if (mr.NewScope(mr.declField("un")))//nullptr, SCOPE_ UNION)))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declUField("val", mr.type(_Word));
			mr.declUField("ptr", mr.type(_Addr), (AttrIdEnum)ATTR_ELF_DYN_TAG_PTR);
		}
	}

	if (mr.NewScope("Elf32_Rel"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("offset", mr.type(_Addr), ATTR_VA);
		//mr.declField("info", mr.type(_Word));
		if (mr.NewScope(mr.declField("info")))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declBField("type", mr.arrayOf(mr.enumOf(mr.type("ELF_R_386"), _Word), 8));
			mr.declBField("sym", mr.arrayOf(mr.type(_Word), 24), (AttrIdEnum)ATTR_ELF_DYNSYM_INDEX);
		}
	}

	if (mr.NewScope("Elf32_Rela"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("offset", mr.type(_Addr), ATTR_VA);
		//mr.declField("info", mr.type(_Word));
		if (mr.NewScope(mr.declField("info")))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declBField("type", mr.arrayOf(mr.enumOf(mr.type("ELF_R_386"), _Word), 8));
			mr.declBField("sym", mr.arrayOf(mr.type(_Word), 24), (AttrIdEnum)ATTR_ELF_DYNSYM_INDEX);
		}
		mr.declField("addend", mr.type(_SWord));
	}
}

void ELF_Strucs_t::create64bitStructures(I_Module& mr)
{
	if (mr.NewScope("Elf64_Ehdr"))//e_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("ident", mr.type("Elf_NIdent"));
		mr.declField("type", mr.enumOf(mr.type("Elf_ET"), _Half));
		mr.declField("machine", mr.enumOf(mr.type("Elf_EM"), _Half));
		mr.declField("version", mr.type(_Word));
		mr.declField("entry", mr.type(_Addr64), ATTR_VA);
		mr.declField("phoff", mr.type(_Off64));
		mr.declField("shoff", mr.type(_Off64), ATTR_FILEPTR);
		mr.declField("flags", mr.type(_Word));
		mr.declField("ehsize", mr.type(_Half));
		mr.declField("phentsize", mr.type(_Half));
		mr.declField("phnum", mr.type(_Half));
		mr.declField("shentsize", mr.type(_Half));
		mr.declField("shnum", mr.type(_Half));
		mr.declField("shstrndx", mr.type(_Half));
	}

	if (mr.NewScope("Elf64_SHF"))//SHF_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declBField("WRITE", mr.type(_XWord));//0x1
		mr.declBField("ALLOC", mr.type(_XWord));//0x2
		mr.declBField("EXECINSTR", mr.type(_XWord));//0x4
		//mr.setSize(CHAR_BIT * sizeof(Elf64_Xword));//?
		mr.skipBits(CHAR_BIT * sizeof(Elf64_Xword) - 3);
	}

	if (mr.NewScope("Elf64_Shdr"))//sh_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("name", mr.type(_Word), (AttrIdEnum)ATTR_ELF_STRING_INDEX0);
		mr.declField("type", mr.enumOf(mr.type("Elf_SHT"), _Word));
		mr.declField("flags", mr.type("Elf64_SHF"), ATTR_COLLAPSED);
		mr.declField("addr", mr.type(_Addr64), ATTR_VA);
		mr.declField("offset", mr.type(_Off64), ATTR_FILEPTR);
		mr.declField("size", mr.type(_XWord));
		mr.declField("link", mr.type(_Word), (AttrIdEnum)ATTR_ELF_SECTION_HEADER);
		mr.declField("info", mr.type(_Word));
		mr.declField("addralign", mr.type(_XWord));
		mr.declField("entsize", mr.type(_XWord));
	}

	if (mr.NewScope("Elf64_Sym"))//st_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("name", mr.type(_Word), (AttrIdEnum)ATTR_ELF_OFF_LNK_SEC_VIA_FP);// ATTR_ELF_STRING_INDEX_DYNSYM);
		mr.declBField("type", mr.arrayOf(mr.enumOf(mr.type("ELF_STT"), _uchar), 4));
		mr.declBField("bind", mr.arrayOf(mr.enumOf(mr.type("ELF_STB"), _uchar), 4));
		mr.declField("other", mr.type(_uchar));
		mr.declField("shndx", mr.type(_Half));
		mr.declField("value", mr.type(_Addr64), ATTR_VA);
		mr.declField("size", mr.type(_XWord));
	}

	if (mr.NewScope("Elf64_Phdr"))//p_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("type", mr.enumOf(mr.type("ELF_PT"), _Word));
		mr.declField("flags", mr.type("ELF_PF"), ATTR_COLLAPSED);
		mr.declField("offset", mr.type(_Off64), ATTR_FILEPTR);
		mr.declField("vaddr", mr.type(_Addr64), ATTR_VA);
		mr.declField("paddr", mr.type(_Addr64), ATTR_VA);
		mr.declField("filesz", mr.type(_XWord));
		mr.declField("memsz", mr.type(_XWord));
		mr.declField("align", mr.type(_XWord));
	};

	if (mr.NewScope("Elf64_Dyn"))//d_
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("tag", mr.enumOf(mr.type("ELF_DT"), _SXWord));
		if (mr.NewScope(mr.declField("un")))//nullptr, SCOPE_ UNION)))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declUField("val", mr.type(_XWord));
			mr.declUField("ptr", mr.type(_Addr64), (AttrIdEnum)ATTR_ELF_DYN_TAG_PTR);
		}
	}

	if (mr.NewScope("Elf64_Rel"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("offset", mr.type(_Addr64), ATTR_VA);
		//mr.declField("info", mr.type(_XWord));
		if (mr.NewScope(mr.declField("info")))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField("type", mr.type(_Word));
			mr.declField("sym", mr.type(_Word), (AttrIdEnum)ATTR_ELF_DYNSYM_INDEX);
		}
	}

	if (mr.NewScope("Elf64_Rela"))
	{
		SAFE_SCOPE_HERE(mr);
		mr.declField("offset", mr.type(_Addr64), ATTR_VA);
		//mr.declField("info", mr.type(_XWord));
		if (mr.NewScope(mr.declField("info")))
		{
			SAFE_SCOPE_HERE(mr);
			mr.declField("type", mr.enumOf(mr.type("ELF_R_386"), _Word));
			mr.declField("sym", mr.type(_Word), (AttrIdEnum)ATTR_ELF_DYNSYM_INDEX);
		}
		mr.declField("addend", mr.type(_SXWord));
	}
}

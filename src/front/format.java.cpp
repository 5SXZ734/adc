#include <assert.h>
#include <iostream>

#include "qx/MyString.h"
#include "shared/defs.h"
#include "shared/misc.h"
#include "shared/front.h"
#include "x86_IR.h"
#include "shared.h"
#include "format.java.h"
#include "interface/IADCMain.h"
#include "interface/IADCFront.h"


//#define TYPEID_JCODE	"@JCODE"
//#define TYPEID_JFUNC	"@JFUNC"


#include "shared/misc.h"
#include "shared/front.h"
#include <inttypes.h>

enum JAVA_AttrIdEnum
{
	ATTR_CONST_POOL_INDEX = ATTR___MISC_BEGIN,
	ATTR_FIELD_POOL_INDEX,
	ATTR_METHOD_POOL_INDEX
};


typedef unsigned char		u1;
typedef unsigned short		u2;
typedef unsigned int		u4;
typedef uint64_t			u8;

#pragma pack(1)


inline u2 Swap(u2 t) { return swap_endian(t); }
inline u4 Swap(u4 t) { return swap_endian(t); }
inline u8 Swap(u8 t) { return swap_endian(t); }

enum class ACC_e 
{
	PUBLIC = 0x0001,
	FINAL = 0x0010,
	SUPER = 0x0020,
	INTERFACE = 0x0200,
	ABSTRACT = 0x0400,
	SYNTHETIC = 0x1000,
	ANNOTATION = 0x2000,
	ENUM = 0x4000
};

enum class CT_e : u1
{
	Utf8 = 1,
	Unicode,
	Integer,				
	Float,				
	Long,				
	Double,
	Class,				
	String,				
	Fieldref,			
	Methodref,			
	InterfaceMethodref,	
	NameAndType,			
};

struct CONSTANT_info {
	CT_e tag;
};

struct CONSTANT_Class_info : public CONSTANT_info {
	u2 name_index;
};

struct CONSTANT_Fieldref_info : public CONSTANT_info {
	u2 class_index;
	u2 name_and_type_index;
};

struct CONSTANT_Methodref_info : public CONSTANT_info {
	u2 class_index;
	u2 name_and_type_index;
};

struct CONSTANT_InterfaceMethodref_info : public CONSTANT_info {
	u2 class_index;
	u2 name_and_type_index;
};

struct CONSTANT_String_info : public CONSTANT_info {
	u2 string_index;
};

struct CONSTANT_Integer_info : public CONSTANT_info {
	u4 bytes;
};

struct CONSTANT_Float_info : public CONSTANT_info {
	u4 bytes;
};

struct CONSTANT_Long_info : public CONSTANT_info {
	u4 high_bytes;
	u4 low_bytes;
};

struct CONSTANT_Double_info : public CONSTANT_info {
	u4 high_bytes;
	u4 low_bytes;
};

struct CONSTANT_Utf8_info : public CONSTANT_info {
	u2 length;
	u1 bytes[1];//length
};

struct CONSTANT_NameAndType_info : public CONSTANT_info {
	u2 name_index;
	u2 signature_index;
};

struct attribute_info {
	u2 attribute_name_index;
	u4 attribute_length;
	//u1 info[attribute_length];
};

struct field_info {
	u2 access_flags;
	u2 name_index;
	u2 signature_index;
	u2 attributes_count;
	//attribute_info  attributes[attribute_count];
};

struct method_info {
	u2 access_flags;
	u2 name_index;
	u2 signature_index;
	u2 attributes_count;
	//attribute_info  attributes[attribute_count];
};

struct ConstantValue_attribute {
	u2 attribute_name_index;
	u4 attribute_length;
	u2 constantvalue_index;
};

struct exeption_info {
	u2    start_pc;
    u2    end_pc;
    u2    handler_pc;
    u2    catch_type;
};

#pragma pack()


enum class AttributeInfoId_e
{
	Invalid,
	ConstantValue,
	Code,
	StackMapTable, StackMap,
	Exceptions,
	InnerClasses,
	EnclosingMethod,
	Synthetic,
	Signature,
	SourceFile,
	SourceDebugExtension,
	LineNumberTable,
	LocalVariableTable,
	LocalVariableTypeTable,
	Deprecated,
	RuntimeVisibleAnnotations,
	RuntimeInvisibleAnnotations,
	RuntimeVisibleParameterAnnotations,
	RuntimeInvisibleParameterAnnotations,
	AnnotationDefault,
	BootstrapMethods
};

/////////////////////////////////////////////////////


class JAVA_t
{
	const I_DataSourceBase& mRaw;
	std::vector<OFF_t> mcp;	//constant pool indices
	std::vector<OFF_t> mfp;	//fields pool
	std::vector<OFF_t> mmp;	//methods pool
public:
	JAVA_t(const I_DataSourceBase& aRaw)
		: mRaw(aRaw)
	{
		DataStream_t ds(aRaw);
		u4 magic(ds.read<u4>());
		if (magic != 0xBEBAFECA)
			throw(-1);
		u2 minor_version(Swap(ds.read<u2>()));
		u2 major_version(Swap(ds.read<u2>()));
		u2 constant_pool_count(Swap(ds.read<u2>()));
		mcp.resize(constant_pool_count);
		for (int line(1); line < constant_pool_count; line++)
		{
			mcp[line] = ds.current();
			switch (ds.peek<CT_e>())
			{
			case CT_e::Class:
				ds.skip<CONSTANT_Class_info>();
				break;
			case CT_e::Fieldref:
				ds.skip<CONSTANT_Fieldref_info>();
				break;
			case CT_e::Methodref:
				ds.skip<CONSTANT_Methodref_info>();
				break;
			case CT_e::InterfaceMethodref:
				ds.skip<CONSTANT_InterfaceMethodref_info>();
				break;
			case CT_e::Float:
				ds.skip<CONSTANT_Float_info>();
				break;
			case CT_e::Integer:
				ds.skip<CONSTANT_Integer_info>();
				break;
			case CT_e::String:
				ds.skip<CONSTANT_String_info>();
				break;
			case CT_e::Long:
				ds.skip<CONSTANT_Long_info>();
				++line;//?
				break;
			case CT_e::Double:
				ds.skip<CONSTANT_Double_info>();
				++line;//?
				break;
			case CT_e::NameAndType:
				ds.skip<CONSTANT_NameAndType_info>();
				break;
			case CT_e::Utf8:
			{
				ds.skip<CONSTANT_info>();
				u2 length(Swap(ds.read<u2>()));
				if (length > 0)
					ds.skip(length);
				break;
			}
			default:
				assert(0);
				break;
			}
		}
	}
	OFF_t fromConstIndex(u2 i) const
	{
		if (i < mcp.size())
			return mcp[i];
		return OFF_NULL;
	}
	AttributeInfoId_e attributeInfoIdFromConstIndex(u2 i) const
	{
		OFF_t off(fromConstIndex(i));
		if (off != OFF_NULL)
		{
			DataStream_t ds(mRaw, off);
			CONSTANT_info a(ds.read<CONSTANT_info>());
			if (a.tag == CT_e::Utf8)
			{
				u2 length(ds.read<u2>());
				std::ostringstream ss;
				ds.fetchString(ss, Swap(length));
				static s2e_t m;
				return m.from(ss.str());
			}
		}
		return AttributeInfoId_e::Invalid;
	}

	struct s2e_t : std::map<std::string, AttributeInfoId_e>
	{
		s2e_t()
		{
#define _add(a)	insert(std::make_pair(#a, AttributeInfoId_e::a))
			_add(Invalid);
			_add(ConstantValue);
			_add(Code);
			_add(StackMapTable); _add(StackMap);
			_add(Exceptions);
			_add(InnerClasses);
			_add(EnclosingMethod);
			_add(Synthetic);
			_add(Signature);
			_add(SourceFile);
			_add(SourceDebugExtension);
			_add(LineNumberTable);
			_add(LocalVariableTable);
			_add(LocalVariableTypeTable);
			_add(Deprecated);
			_add(RuntimeVisibleAnnotations);
			_add(RuntimeInvisibleAnnotations);
			_add(RuntimeVisibleParameterAnnotations);
			_add(RuntimeInvisibleParameterAnnotations);
			_add(AnnotationDefault);
			_add(BootstrapMethods);
#undef _add
		}
		AttributeInfoId_e from(const std::string& s) const {
			const_iterator i(find(s));
			return i != end() ? i->second : AttributeInfoId_e::Invalid;
		}
	};

};

class FrontentJava_t : public I_Front
{
	JAVA_t mJava;
public:

	FrontentJava_t(const I_DataSourceBase& aRaw)
		: mJava(aRaw)
	{
//?		AddTypeTemplate("JAVA.VM.CLASS", (PCreateTypeFunc)makeJavaClassType);
//?		AddTypeTemplate("JAVA.CODE", (PCreateTypeFunc)makeJavaCodeType);
	}

	virtual ~FrontentJava_t() {}

	const JAVA_t& java() const { return mJava; }
	/*virtual int unassemble(IStream_t& is, ADDR addr, ins_t& ins)
	{
		return 0;
	}*/

	static HTYPE makeJavaClassType(FrontentJava_t* pSelf, const char* pData, unsigned long nSize)
	{
		//T_JAVA java;
		//PTYPE t = java.create(pSelf, pData, nSize);
		return 0;
	}

	static int makeJavaCodeType(FrontentJava_t* pSelf, const char* pData, unsigned long nSize)
	{
		return 0;//? mr.CreateCode(new JCode_t(pSelf));
	}

	virtual AKindEnum translateAddress(const I_DataStreamBase& mr, int moduleId, ADDR& addr, AttrIdEnum attr)
	{
		ADDR atAddr(mr.cp());
		OFF_t off;
		switch (attr)
		{
		case ATTR_FILEPTR:
			return AKIND_RAW;
		case ATTR_OFFS:
			return AKIND_VA;
		case ATTR_CONST_POOL_INDEX:
			off = mJava.fromConstIndex((u2)addr);
			if (off == OFF_NULL)
				break;
			addr = (ADDR)off;
			return AKIND_RAW;
		default:
			break;
		}
		return AKIND_NULL;
	}
};


/////////////////////////////////////////////////


#define TYPEID_U1	TYPEID_BYTE		//"u1"
#define TYPEID_U2	TYPEID_WORD		//"u2"
#define TYPEID_U4	TYPEID_DWORD
#define TYPEID_U8	TYPEID_QWORD
#define TYPEID_F	TYPEID_FLOAT
#define TYPEID_D	TYPEID_DOUBLE
#define TYPEID_TAG	TYPEID_U1


////////////////////////////////////////////////////////


struct INS_t {
	const char* mnemo;
	u1		bytes_count;
};

INS_t INS[] = {
	{"nop", 0},	{"aconst_null", 0},	{"iconst_m1", 0}, {"iconst_0", 0}, {"iconst_1", 0},	//0
	{"iconst_2", 0}, {"iconst_3", 0}, {"iconst_4", 0}, {"iconst_5", 0},	{"lconst_0", 0},	//5
	{"lconst_1", 0}, {"fconst_0", 0}, {"fconst_1", 0}, {"fconst_2", 0},	{"dconst_0", 0},	//10
	{"dconst_1", 0}, {"bipush", 1},	{"sipush", 2},	{"ldc1", 1},	{"ldc2", 2},	//15
	{"ldc2w", 2}, {"iload", 1}, {"lload", 1}, {"fload", 1}, {"dload", 1},	//20

	{"aload", 1}, {"iload_0", 0}, {"iload_1", 0}, {"iload_2", 0}, {"iload_3", 0},	//25
	{"lload_0", 0},	{"lload_1", 0},	{"lload_2", 0},	{"lload_3", 0},	{"fload_0", 0},	//30
	{"fload_1", 0},	{"fload_2", 0},	{"fload_3", 0},	{"dload_0", 0},	{"dload_1", 0},	//35
	{"dload_2", 0},	{"dload_3", 0},	{"aload_0", 0},	{"aload_1", 0},	{"aload_2", 0},	//40
	{"aload_3", 0},	{"iaload", 0},	{"laload", 0},	{"faload", 0},	{"daload", 0},	//45

	{"aaload", 0},	{"baload", 0},	{"caload", 0},	{"saload", 0},	{"istore", 1},	//50
	{"lstore", 1}, {"fstore", 1}, {"dstore", 1}, {"astore", 1}, {"istore_0", 0},	//55
	{"istore_1", 0}, {"istore_2", 0}, {"istore_3", 0}, {"lstore_0", 0}, {"lstore_1", 0},	//60
	{"lstore_2", 0}, {"lstore_3", 0}, {"fstore_0", 0}, {"fstore_1", 0}, {"fstore_2", 0},	//65
	{"fstore_3", 0}, {"dstore_0", 0}, {"dstore_1", 0}, {"dstore_2", 0},	{"dstore_3", 0},	//70

	{"astore_0", 0}, {"astore_1", 0}, {"astore_2", 0}, {"astore_3", 0},	{"iastore", 0},	//75
	{"lastore", 0},	{"fastore", 0},	{"dastore", 0},	{"aastore", 0},	{"bastore", 0},	//80
	{"castore", 0},	{"sastore", 0},	{"pop", 0},	{"pop2", 0}, {"dup", 0},	//85
	{"dup_x1", 0}, {"dup_x2", 0}, {"dup2", 0}, {"dup2_x1", 0}, {"dup2_x2", 0},	//90
	{"swap", 0}, {"iadd", 0}, {"ladd ", 0},	{"fadd", 0}, {"dadd", 0},	//95

	{"isub", 0},	{"lsub", 0},	{"fsub", 0},	{"dsub", 0},	{"imul", 0},	//100
	{"lmul", 0},	{"fmul", 0},	{"dmul", 0},	{"idiv", 0},	{"ldiv", 0},	//105
	{"fdiv", 0},	{"ddiv", 0},	{"irem", 0},	{"lrem", 0},	{"frem", 0},	//110
	{"drem", 0},	{"ineg", 0},	{"lneg", 0},	{"fneg", 0},	{"dneg", 0},	//115
	{"ishl", 0},	{"lshl", 0},	{"ishr", 0},	{"lshr", 0},	{"iushr", 0},	//120

	{"lushr", 0},	{"iand", 0},	{"land", 0},	{"ior", 0},	{"lor", 0},	//125
	{"ixor", 0},	{"lxor", 0},	{"iinc", 0}, {"i2l", 0},	{"i2f", 0},	//130
	{"i2d", 0},	{"l2i", 0},	{"l2f", 0},	{"l2d", 0},	{"f2i", 0},	//135
	{"f2l", 0},	{"f2d", 0},	{"d2i", 0},	{"d2l", 0},	{"d2f", 0},	//140
	{"int2byte", 0},	{"int2char", 0},	{"int2short", 0}, {"lcmp", 0}, {"fcmpl", 0},	//145

	{"fcmpg", 0}, {"dcmpl", 0},	{"dcmpg", 0},	{"ifeq", 2}, {"ifne", 2},	//150
	{"iflt", 2}, {"ifge", 2}, {"ifgt", 2},	{"ifle", 2}, {"if_icmpeq", 2},	//155
	{"if_icmpne", 2}, {"if_icmplt", 2},	{"if_icmpge", 2}, {"if_icmpgt", 2},	{"if_icmple", 2},	//160
	{"if_acmpeq", 2},	{"if_acmpne", 2},	{"goto", 2},	{"jsr", 2},	{"ret", 1},	//165
	{"tableswitch", 0}, {"lookupswitch", 0}, {"ireturn", 0},	{"lreturn", 0},	{"freturn", 0},	//170

	{"dreturn", 0},	{"areturn", 0},	{"return", 0},	{"getstatic", 2},	{"putstatic", 2},	//175
	{"getfield", 2}, {"putfield", 2}, {"invokevirtual", 2}, {"invokenonvirtual", 2}, {"invokestatic", 2},	//180
	{"", 0},	{"", 0},	{"new", 2},	{"newarray ", 1},	{"anewarray", 2},	//185
	{"arraylength", 0},	{"athrow", 0},	{"checkcast", 2},	{"instanceof", 2},	{"monitorenter", 0},	//190
	{"monitorexit", 0},	{"", 0},	{"multianewarray", 2},	{"ifnull", 2},	{"ifnonnull", 2},	//195

	{"goto_w", 4}, {"jsr_w", 4}, {"breakpoint", 0},	{"", 0},	{"", 0},	//200
	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"ret_w", 2},	//205
	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//210
	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//215
	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//220

	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//225
	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//230
	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//235
	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//240
	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//245

	{"", 0},	{"", 0},	{"", 0},	{"", 0},	{"", 0},	//250
	{"", 0},													//255

};

class JCode_t : public I_Code
{
	//I_Front * mpIFront;

public:
	JCode_t()
		//: mpIFront(pIFront)
	{
		//mpIFront->AddRef();
	}

	~JCode_t()
	{
		//mpIFront->Release();
	}

	virtual int Unassemble(DataStream_t& is, ADDR64 base, ADDR va, ins_desc_t& desc)
	{
		ins_t ins;
		int len(Unassemble(is, base, va, ins));
		desc = ins;
		return len;
	}

	int Unassemble(DataStream_t& is, ADDR64 base, ADDR va, ins_t& ins)
	{
		ins.reset();

		int offs0 = 0;//addr - base_addr;
//		if (offs0 < 0)
//			return 0;

		int offs = offs0;

		//		is.seekg(offs0);

		u1 opcode = is.get();//code[offs];
		if (INS[opcode].mnemo[0] == 0)
		{
			//os << (u4)code[offs];
			//offs++;
			return 0;
		}

		strcpy(ins.mnemo, INS[opcode].mnemo);		//ins.os << INS[opcode].mnemo; 
		offs++;

		int i(0);
		for (i = 0; i < INS[opcode].bytes_count; i++)
		{
			u1 b = is.get();//os << " " << (u4)code[offs];
			ins.ops[i].opc = OPC_IMM;
			ins.ops[i].optype = sizeof(b);//?
			ins.ops[i].lval.ui8 = b;//?
			offs++;
		}

		if (opcode == 170)//tableswitch
		{
			ADDR offs2 = offs + sizeof(u4);
			offs2 &= ~3;//padding
			int d = offs2 - offs;
			is.skip(d);
			offs += d;
			u4 df = is.read<u4>();//default
			df = Swap(df);
			offs += sizeof(df);
			u4 lo = is.read<u4>();//low index
			lo = Swap(lo);
			offs += sizeof(lo);
			u4 hi = is.read<u4>();//high index
			hi = Swap(hi);
			offs += sizeof(hi);
			//os << " " << lo << " to " << hi << " default " << df;
/*			for (u4 i = lo; i <= hi; i++)
			{
				os << endl;
				os << "\t\t\t";
				os << Swap(*(u4*)(code+offs));
				addr += 4;
			}*/
			ins.ops[i].opc = OPC_IMM;
			ins.ops[i].optype = sizeof(df);
			ins.ops[i].lval.ui32 = df;
			i++;

			ins.ops[i].opc = OPC_IMM;
			ins.ops[i].optype = sizeof(df);
			ins.ops[i].lval.ui32 = lo;
			i++;

			ins.ops[i].opc = OPC_IMM;
			ins.ops[i].optype = sizeof(df);
			ins.ops[i].lval.ui32 = hi;
			i++;

			//			offs += (hi-lo+1)*sizeof(u4);
		}
		switch (opcode)
		{
		case 170://"tableswitch"
			ins.flowBreak = ins_desc_t::FB_JUMP;
			break;
		case 175://"dreturn"
		case 176://"areturn"
		case 177://"return"
			ins.flowBreak = ins_desc_t::FB_RET;
			break;
		}
		return offs - offs0;
	}
};




////////////////////////////////////////////////////////////VTableType
BEGIN_DYNAMIC_TYPE(CONSTANT_Utf8_info)
{
	mr.declField(nullptr, mr.type("CONSTANT_info"));
	DECLDATA(u2, length);
	mr.declField("length", mr.type(TYPEID_U2), ATTR_DECIMAL);
	if (length != 0)
		mr.declField("bytes", mr.arrayOf(mr.type(TYPEID_CHAR), Swap(length)));
}
END_DYNAMIC_TYPE(CONSTANT_Utf8_info);

BEGIN_DYNAMIC_TYPE(attribute_info)
{
	DECLDATAEX(attribute_info, ai);
	u4 attribute_length = Swap(ai.attribute_length);
	mr.declField("attribute_name_index", mr.type(TYPEID_U2), (AttrIdEnum)ATTR_CONST_POOL_INDEX);
	mr.declField("attribute_length", mr.type(TYPEID_U4));
	if (attribute_length != 0)
	{
		POSITION end(mr.cp() + attribute_length);
		FrontentJava_t* pFront((FrontentJava_t*)mr.frontend());
		u2 attribute_name_index(Swap(ai.attribute_name_index));
		//OFF_t off(pFront->java().fromConstIndex(attribute_name_index));
		//if (off != OFF_NULL)
		{
			//DataStream_t ds(mr, off);
			//if (ds.strCmp("Code") == 0)
			AttributeInfoId_e e(pFront->java().attributeInfoIdFromConstIndex(attribute_name_index));
			switch (e)
			{
			case AttributeInfoId_e::Code:
			{
				mr.declField("max_stack", mr.type(TYPEID_U2));
				mr.declField("max_locals", mr.type(TYPEID_U2));
				DECLDATA(u4, code_length);
				mr.declField("code_length", mr.type(TYPEID_U4), ATTR_DECIMAL);
				if (code_length != 0)
					mr.declField("code", mr.arrayOf(mr.type(TYPEID_U1), Swap(code_length)));

				DECLDATA(u2, exception_table_length);
				mr.declField("exception_table_length", mr.type(TYPEID_U2));
				if (exception_table_length != 0)
					mr.declField("exception_table", mr.arrayOf(mr.type("exeption_info"), Swap(exception_table_length)));

				DECLDATA(u2, code_attributes_count);
				mr.declField("code_attributes_count", mr.type(TYPEID_U2));
				if (code_attributes_count != 0)
				{
					if (mr.NewScope(mr.declField("code_attributes_table")))
					{
						SAFE_SCOPE_HERE(mr);
						u2 n = Swap(code_attributes_count);
						for (int i(0); i < n; i++)
							mr.declField(nullptr, mr.type(_PFX("attribute_info")));
					}

					//mr.declField("code_attributes_table", mr.arrayOf(mr.type(TYPEID_U1), Swap(code_attributes_count)));
				}

				/*						DECLDATA(u2, code_attributes_count);
						mr.declField("code_attributes_count", mr.type(TYPEID_U2));

						if (mr.NewScope(mr.declField("attributes")))
						{
							SAFE_SCOPE_HERE(mr);
							declare_attribute_pool(Swap(attributes_count));
							for (int i(0); i < methods_count; i++)
									mr.declField(nullptr, mr.type(_PFX("method_info")));
						}
			*/
				mr.skip(end - mr.cp());
				return;
			}
			case AttributeInfoId_e::LineNumberTable:
			{
				DECLDATA(u2, line_number_table_length);
				mr.declField("line_number_table_length", mr.type(TYPEID_U2));
				if (line_number_table_length != 0)
					mr.declField("line_number_table", mr.arrayOf(mr.type("line_number_info"), Swap(line_number_table_length)), ATTR_COLLAPSED);
				mr.skip(end - mr.cp());
				return;
			}
			case AttributeInfoId_e::StackMap:
			//?case AttributeInfoId_e::StackMapTable:
			{
				DECLDATA(u2, number_of_entries);
				mr.declField("number_of_entries", mr.type(TYPEID_U2));
				DECLDATA(u1, frame_type);
				mr.declField("frame_type", mr.type(TYPEID_U1));
				if (frame_type < 64)//SAME
				{
				}
				else if (frame_type < 128)// SAME_LOCALS_1_STACK_ITEM
				{
					//verification_type_info stack[1];
				}
				else if (frame_type < 247)
				{//reserved
				}
				else if (frame_type == 247)// SAME_LOCALS_1_STACK_ITEM_EXTENDED
				{
					mr.declField("offset_delta", mr.type(TYPEID_U2));
					//verification_type_info stack[1];
				}
				else if (frame_type < 251)// CHOP
				{
					mr.declField("offset_delta", mr.type(TYPEID_U2));
				}
				else if (frame_type == 251)// SAME_FRAME_EXTENDED
				{
					mr.declField("offset_delta", mr.type(TYPEID_U2));
				}
				else if (frame_type < 255)// APPEND
				{
					mr.declField("offset_delta", mr.type(TYPEID_U2));
					//verification_type_info locals[frame_type - 251];
				}
				else//255 - FULL_FRAME
				{
					mr.declField("offset_delta", mr.type(TYPEID_U2));
					mr.declField("number_of_locals", mr.type(TYPEID_U2));
					//verification_type_info locals[number_of_locals];

					//mr.declField("number_of_stack_items", mr.type(TYPEID_U2));
					//verification_type_info stack[number_of_stack_items];
				}
				mr.skip(end - mr.cp());
				return;
			}
			default:
				break;
			}
			STOP
		}
		mr.declField("info", mr.arrayOf(mr.type(TYPEID_U1), attribute_length));
	}
}
END_DYNAMIC_TYPE(attribute_info);


BEGIN_DYNAMIC_TYPE(field_info)
{
	DECLDATAEX(field_info, fi);

	u2 attributes_count = Swap(fi.attributes_count);

	mr.declField("access_flags", mr.type("ACC_id"), ATTR_COLLAPSED);
	mr.declField("name_index", mr.type(TYPEID_U2), (AttrIdEnum)ATTR_CONST_POOL_INDEX);
	mr.declField("descriptor_index", mr.type(TYPEID_U2), (AttrIdEnum)ATTR_CONST_POOL_INDEX);
	mr.declField("attributes_count", mr.type(TYPEID_U2), ATTR_DECIMAL);

	if (attributes_count > 0)
	{
		if (mr.NewScope(mr.declField("attributes")))
		{
			SAFE_SCOPE_HERE(mr);
			for (int i(0); i < attributes_count; i++)
				mr.declField(nullptr, mr.type(_PFX("attribute_info")));//MyStringf("[%d]", i)
		}
	}
}
END_DYNAMIC_TYPE(field_info);


BEGIN_DYNAMIC_TYPE(method_info)
{
	DECLDATAEX(method_info, mi);
	u2 attributes_count = Swap(mi.attributes_count);

	mr.declField("access_flags", mr.type("ACC_id"));// , ATTR_COLLAPSED);
	mr.declField("name_index", mr.type(TYPEID_U2), (AttrIdEnum)ATTR_CONST_POOL_INDEX);
	mr.declField("descriptor_index", mr.type(TYPEID_U2), (AttrIdEnum)ATTR_CONST_POOL_INDEX);
	mr.declField("attributes_count", mr.type(TYPEID_U2), ATTR_DECIMAL);

	if (attributes_count > 0)
	{
		if (mr.NewScope(mr.declField("attributes")))
		{
			SAFE_SCOPE_HERE(mr);
			for (int i = 0; i < attributes_count; i++)
				mr.declField(nullptr, mr.type(_PFX("attribute_info")));//MyStringf("[%d]", i)
		}
	}
}
END_DYNAMIC_TYPE(method_info);



//==================================================> (CFormatter_JAVA_CLASS)

class CFormatter_JAVA_CLASS : public I_FormatterType
{
public:
	CFormatter_JAVA_CLASS() {}

	class T_impl
	{
		I_SuperModule& mr;

	public:
		T_impl(I_SuperModule& r)
			: mr(r)
		{
		}

		

	public:
		void preformat(I_SuperModule&, unsigned long dataSize)
		{
			mr.setEndianness(false);//msb

			mr.installNamespace();
			mr.installFrontend(_PFX("FE_JAVA"));
			mr.installTypesMgr();

			I_Front* pFront(mr.frontend());

			//?mr.setLittleEnd(mdc, true);
			//?mr.setCodeType(mdc, mr.CreateCode(new JCode_t(pIFront)));

			create_data_strucs();
			mr.DeclareContextDependentType(_PFX("CONSTANT_Utf8_info"));
			mr.DeclareContextDependentType(_PFX("field_info"));
			mr.DeclareContextDependentType(_PFX("attribute_info"));
			mr.DeclareContextDependentType(_PFX("method_info"));
			//mr.DeclareContextDependentType(_PFX("exception_info"));


			DECLDATA(u4, magic);
			mr.declField("magic", mr.type(TYPEID_U4));

			DECLDATA(u2, minor_version);//Swap?
			mr.declField("minor_version", mr.type(TYPEID_U2), ATTR_DECIMAL);

			DECLDATA(u2, major_version);//Swap
			mr.declField("major_version", mr.type(TYPEID_U2), ATTR_DECIMAL);

			DECLDATA(u2, constant_pool_count);//Swap
			mr.declField("constant_pool_count", mr.type(TYPEID_U2), ATTR_DECIMAL);

			create_CONSTANT_POOL(Swap(constant_pool_count));

			DECLDATA(u2, access_flags);//Swap
			mr.declField("access_flags", mr.type("ACC_id"), ATTR_COLLAPSED);

			DECLDATA(u2, this_class);//Swap
			mr.declField("this_class", mr.type(TYPEID_U2), (AttrIdEnum)ATTR_CONST_POOL_INDEX);

			DECLDATA(u2, super_class);//Swap
			mr.declField("super_class", mr.type(TYPEID_U2), (AttrIdEnum)ATTR_CONST_POOL_INDEX);

			DECLDATA(u2, interfaces_count);
			mr.declField("interfaces_count", mr.type(TYPEID_U2), ATTR_DECIMAL);

			if (Swap(interfaces_count) > 0)
				mr.declField("interfaces", mr.arrayOf(mr.type(TYPEID_U2), Swap(interfaces_count)));

			DECLDATA(u2, fields_count);
			mr.declField("fields_count", mr.type(TYPEID_U2), ATTR_DECIMAL);

			create_FIELD_POOL(Swap(fields_count));

			DECLDATA(u2, methods_count);
			mr.declField("methods_count", mr.type(TYPEID_U2), ATTR_DECIMAL);
			create__METHOD_POOL(Swap(methods_count));

			DECLDATA(u2, attributes_count);
			mr.declField("attributes_count", mr.type(TYPEID_U2));

	//		declare_attribute_pool(Swap(attributes_count));

			//	addGlobal( addr, mr.mr.type(TYPEID_U2), "file_size" );
		}

		void create__METHOD_POOL(u2 methods_count)
		{
			if (methods_count > 0)
			{
				if (mr.NewScope(mr.declField("methods")))
				{
					SAFE_SCOPE_HERE(mr);
					for (int i(0); i < methods_count; i++)
						mr.declField(nullptr, mr.type(_PFX("method_info")));
				}
			}
		}


		void create_FIELD_POOL(u2 fields_count)
		{
			if (fields_count > 0)
			{
				if (mr.NewScope(mr.declField("fields", ATTR_COLLAPSED)))
				{
					SAFE_SCOPE_HERE(mr);
					for (int i(0); i < fields_count; i++)
						mr.declField(nullptr, mr.type(_PFX("field_info")));
				}
			}
		}


		void declare_attribute_pool(u2 attributes_count)
		{
			if (attributes_count > 0)
			{
				if (mr.NewScope(mr.declField("attributes")))
				{
					SAFE_SCOPE_HERE(mr);
					for (int i(0); i < attributes_count; i++)
						mr.declField(nullptr, mr.type(_PFX("attribute_info")));//MyStringf("[%d]", i)
				}
			}
		}


		void create_CONSTANT_POOL(u2 constant_pool_count)
		{
			if (mr.NewScope(mr.declField("constant_pool", ATTR_COLLAPSED)))
			{
				SAFE_SCOPE_HERE(mr);

				for (int line = 1; line < constant_pool_count; line++)
				{
					//if (line == 503) break;
					HTYPE pT;

					DECLDATAEX(CONSTANT_info, CI);
					bool skipindex = false;

					switch (CI.tag)
					{
					case CT_e::Class:
						pT = mr.type("CONSTANT_Class_info");
						break;
					case CT_e::Fieldref:
						pT = mr.type("CONSTANT_Fieldref_info");
						break;
					case CT_e::Methodref:
						pT = mr.type("CONSTANT_Methodref_info");
						break;
					case CT_e::InterfaceMethodref:
						pT = mr.type("CONSTANT_InterfaceMethodref_info");
						break;
					case CT_e::Float:
						pT = mr.type("CONSTANT_Float_info");
						break;
					case CT_e::Integer:
						pT = mr.type("CONSTANT_Integer_info");
						break;
					case CT_e::String:
						pT = mr.type("CONSTANT_String_info");
						break;
					case CT_e::Long:
						pT = mr.type("CONSTANT_Long_info");
						skipindex = true;
						break;
					case CT_e::Double:
						pT = mr.type("CONSTANT_Double_info");
						skipindex = true;
						break;
					case CT_e::NameAndType:
						pT = mr.type("CONSTANT_NameAndType_info");
						break;
					case CT_e::Utf8:
						pT = mr.type(_PFX("CONSTANT_Utf8_info"));
						break;
					default:
						break;//assert(0);
					}

					const char* fname = MyStringf("[%x]", line);
					mr.declField(fname, pT);
					if (skipindex)
						line++;
				}
			}
		}

		void create_data_strucs()
		{
			if (mr.NewScope("ACC_id"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declBField("PUBLIC", mr.type(TYPEID_U2));		// 0x0001
				mr.declBField("PRIVATE", mr.type(TYPEID_U2));		// 0x0002
				mr.declBField("PROTECTED", mr.type(TYPEID_U2));	// 0x0004
				mr.declBField("STATIC", mr.type(TYPEID_U2));		// 0x0008
				mr.declBField("FINAL", mr.type(TYPEID_U2));		// 0x0010
				mr.declBField("SUPER/SYNCHRONIZED", mr.type(TYPEID_U2));	// 0x0020
				mr.declBField("VOLATILE/BRIDGE", mr.type(TYPEID_U2));		// 0x0040
				mr.declBField("TRANSIENT/VARARGS", mr.type(TYPEID_U2));	// 0x0080
				mr.declBField("NATIVE", mr.type(TYPEID_U2));		// 0x0100
				mr.declBField("INTERFACE", mr.type(TYPEID_U2));	// 0x0200
				mr.declBField("ABSTRACT", mr.type(TYPEID_U2));	// 0x0400
				mr.declBField("STRICT", mr.type(TYPEID_U2));		// 0x0800
				mr.declBField("SYNTHETIC", mr.type(TYPEID_U2));	// 0x1000
				mr.declBField("ANNOTATION", mr.type(TYPEID_U2));	// 0x2000
				mr.declBField("ENUM", mr.type(TYPEID_U2));		// 0x4000
			};

			if (mr.NewScope("CONSTANT_id", SCOPE_ENUM))//CONST_
			{
				SAFE_SCOPE_HERE(mr);
				mr.declEField("Utf8", 1);
				mr.declEField("Unicode", 2);
				mr.declEField("Integer", 3);
				mr.declEField("Float", 4);
				mr.declEField("Long", 5);
				mr.declEField("Double", 6);
				mr.declEField("Class", 7);
				mr.declEField("String", 8);
				mr.declEField("Fieldref", 9);
				mr.declEField("Methodref", 10);
				mr.declEField("InterfaceMethodref", 11);
				mr.declEField("NameAndType", 12);
				mr.declEField("MethodHandle", 15);
				mr.declEField("MethodType", 16);
				mr.declEField("InvokeDynamic", 18);
			};

			if (mr.NewScope("CONSTANT_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField("tag", mr.enumOf(mr.type("CONSTANT_id"), TYPEID_TAG));
			}

			if (mr.NewScope("CONSTANT_Class_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("name_index", mr.type(TYPEID_U2));
			}

			if (mr.NewScope("CONSTANT_Fieldref_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("class_index", mr.type(TYPEID_U2));
				mr.declField("name_and_type_index", mr.type(TYPEID_U2));
			}

			if (mr.NewScope("CONSTANT_Methodref_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("class_index", mr.type(TYPEID_U2));
				mr.declField("name_and_type_index", mr.type(TYPEID_U2));
			}

			if (mr.NewScope("CONSTANT_InterfaceMethodref_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("class_index", mr.type(TYPEID_U2));
				mr.declField("name_and_type_index", mr.type(TYPEID_U2));
			}

			if (mr.NewScope("CONSTANT_String_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("string_index", mr.type(TYPEID_U2));
			}

			if (mr.NewScope("CONSTANT_Integer_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("bytes", mr.type(TYPEID_U4));
			}

			if (mr.NewScope("CONSTANT_Float_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("bytes", mr.type(TYPEID_U4));
			}

			if (mr.NewScope("CONSTANT_Long_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("high_bytes", mr.type(TYPEID_U4));
				mr.declField("low_bytes", mr.type(TYPEID_U4));
			}

			if (mr.NewScope("CONSTANT_Double_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("high_bytes", mr.type(TYPEID_U4));
				mr.declField("low_bytes", mr.type(TYPEID_U4));
			}

			/*if (mr.NewScope("CONSTANT_Utf8_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("length", mr.type("u2"));
				mr.declField("bytes", mr.arrayOf(mr.type("u1"), -1));
			}*/

			if (mr.NewScope("CONSTANT_NameAndType_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField(nullptr, mr.type("CONSTANT_info"));
				mr.declField("name_index", mr.type(TYPEID_U2));
				mr.declField("signature_index", mr.type(TYPEID_U2));
			}

			if (mr.NewScope("exeption_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField("start_pc", mr.type(TYPEID_U2));
				mr.declField("end_pc", mr.type(TYPEID_U2));
				mr.declField("handler_pc", mr.type(TYPEID_U2));
				mr.declField("catch_type", mr.type(TYPEID_U2));
			}

			if (mr.NewScope("line_number_info"))
			{
				SAFE_SCOPE_HERE(mr);
				mr.declField("start_pc", mr.type(TYPEID_U2));
				mr.declField("line_number", mr.type(TYPEID_U2));
			}

			//	mr.addType( new T_CONSTANT_POOL() );
			//	mr.addType( new T_FIELD_POOL() );
		}

		//int decodeConstantPool(DisplayCache_t& D, ADDR addr0);
		//int decodeFieldsPool(DisplayCache_t& D, ADDR addr0);

		/*	int addGlobal( ADDR &addr, Type_t * pT, const char * name )
			{
				mr.addGlobal( addr, pT, name );
				if ( pT->typeCustom() )
					pT = pT->typeCustom()->custom( addr );
				addr += pT->size();
				return 1;
			}*/


	};

	virtual const char* name() const { return "FORMAT_JAVA_CLASS"; }
	virtual void createz(I_SuperModule& r, unsigned long nSize)
	{
		I_SuperModule& mr(r);
		T_impl impl(mr);
		impl.preformat(mr, nSize);
	}

};


DECLARE_FORMATTER(CFormatter_JAVA_CLASS, FORMAT_JAVA_CLASS);

/*const char* str(const char* s, int len)
{
	static char buf[1024];
	memcpy(buf, s, len);
	buf[len] = 0;
	return buf;
}*/

/*

void format( char c )
{
	switch( c )
	{
	case "u1":
		break;
	case "u2":
		break;
	case "u4":
		break;
	case "tag":
		break;
	}
}




const char * CONSTANT_Class_info =
"CONSTANT_Class_info{tag:%t,name_index:%u2}";
const char * CONSTANT_Fieldref_info =
"CONSTANT_Fieldref_info{tag:%t,class_index:%u2,name_and_type_index:%u2}";
const char * CONSTANT_Methodref_info =
"CONSTANT_Methodref_info{tag:%t,class_index:%u2,name_and_type_index:%u2}";
const char * CONSTANT_InterfaceMethodref_info =
"CONSTANT_InterfaceMethodref_info{tag:%t,class_index:%u2,name_and_type_index:%u2}";
const char * CONSTANT_String_info =
"CONSTANT_String_info{tag:%t,string_index:%u2}";
const char * CONSTANT_Integer_info =
"CONSTANT_Integer_info{tag:%t,bytes:%u4}";
const char * CONSTANT_Float_info =
"CONSTANT_Float_info{tag:%t,bytes:%f}";
const char * CONSTANT_Long_info =
"CONSTANT_Long_info{tag:%t,bytes:%l}";
const char * CONSTANT_Double_info =
"CONSTANT_Double_info{tag:%t,bytes:%d}";
const char * CONSTANT_Utf8_info =
"CONSTANT_Utf8_info{tag:%t,length:%u2,bytes:[]%u1}";
const char * CONSTANT_NameAndType_info =
"CONSTANT_NameAndType_info{tag:%t,name_index:%u2,signature_index:%u2}";
*/

/*
int Java_Frontend::decodeFieldsPool( DisplayCache_t &D, ADDR addr0 )
{
	if ( m_fields_count == 0)
		return 0;

	u1 * pBuf = (u1 *)mr.getRawData( 0 );
	u1 * p = pBuf + addr0;

	D.open( p-pBuf );
	D.setcol(CLMN_NAMES);
	D.print( "fields:" );

	for ( int i = 0; i < m_fields_count; i++ )
	{
		D.newLine();
		D.setcol(CLMN_NAMES);
		D.setlevel(1);
		D.print( fmt("[%x]", i) );
		D.setcol(CLMN_CODE);

		D.print( "{" );
		u1 * p0 = p;
		field_info &fi = *(field_info *)p;
		u2 access_flags = Swap( fi.access_flags );
			ACCESS_Flags->OutFlags(D, access_flags);

		u2 name_index = Swap( fi.name_index );
			D.print( fmt(",%x", name_index) );
		u2 signature_index = Swap( fi.signature_index );
			D.print( fmt(",%x", signature_index) );
		u2 attributes_count = Swap( fi.attributes_count );
			D.print( fmt(",%x", attributes_count) );

		p += 8;
		if (attributes_count > 0)
		{
			D.print(",");
			for (int j = 0; j < attributes_count; j++)
			{
				D.setcol(CLMN_NAMES);
				//D.setlevel(2);
				D.print(fmt("[%x]", j));
				D.setcol(CLMN_CODE);

				//D.print("\n\t\t");
				D.print( "{" );
				ConstantValue_attribute &cva = *(ConstantValue_attribute *)p;
				u2 attribute_name_index = Swap( cva.attribute_name_index );
					D.print( fmt("%x", attribute_name_index) );
				u4 attribute_length = Swap( cva.attribute_length );
					D.print( fmt(",%x", attribute_length) );
				u2 constantvalue_index = Swap( cva.constantvalue_index );
					D.print( fmt(",%x", constantvalue_index) );
				D.print("}");
			}
			p += sizeof(ConstantValue_attribute);
		}

		D.print("}");

		int ilen = p-p0;
		D.advance( ilen );
	}

	D.close();
	return 1;
}*/







I_Front* CreateFE_JAVA(const I_DataSourceBase* pRaw)
{
	return new FrontentJava_t(*pRaw);
}

void JAVA_RegisterFormatters(I_ModuleEx &rMain)
{
	rMain.RegisterFormatterType(_PFX("FORMAT_JAVA_CLASS"));
}








#pragma once


typedef enum  
{
	OBJID_NULL,
	OBJID_OP,		//1
	OBJID_FIELD,	//2	structure field
	OBJID_TYPEOBJ,	//3
	OBJID_GLOBOBJ,	//4
	OBJID_5,		//5
	OBJID_6,		//6
	OBJID_7,		//7
	OBJID_8,		//8
	OBJID_9,		//9
	OBJID_10,		//10
	OBJID_11,		//11
	OBJID_12,		//12
	OBJID_13,
	OBJID_14,
	OBJID_15 = 15,	//the last one!!!
		OBJID__TOTAL,
	//these are not used in object's flags, but serve to identify objects (e.g. in save/restore)
	OBJID_TYPE_SIMPLE = OBJID__TOTAL,
	OBJID_TYPE_PTR,
	OBJID_TYPE_IMP,
	OBJID_TYPE_EXP,
	OBJID_TYPE_THISPTR,
	OBJID_TYPE_VPTR,
	OBJID_TYPE_ARRAY,	//20	
	OBJID_TYPE_TYPEDEF,
	OBJID_TYPE_CODE,
	OBJID_TYPE_STRUC,
	OBJID_TYPE_CLASS,
	OBJID_TYPE__UNION,
	OBJID_TYPE_PROC,
	OBJID_TYPE_FUNCDEF,
	OBJID_TYPE_FUNC,
	OBJID_TYPE_SEG,
	//OBJID_TYPE_SEGREF,
	OBJID_TYPE_MODULE,	//30
	OBJID_TYPE_STRUCVAR,
	OBJID_TYPE_BITSET,
	OBJID_TYPE_ENUM,
	OBJID_TYPE_ARRAY_INDEX,
	//OBJID_TYPE_BIT,
	OBJID_TYPE_PROXY,
	OBJID_TYPE_PROJECT,
/*	OBJID_TYPE_VFTABLE,
	OBJID_TYPE_VBTABLE,
	OBJID_TYPE_LVFTABLE,
	OBJID_TYPE_CVFTABLE,*/	//40	
	OBJID_TYPE_THUNK,
	OBJID_TYPE_REF,
	OBJID_TYPE_RREF,
	OBJID_TYPE_NAMESPACE,
	OBJID_TYPE_CONST,
	OBJID_TYPE_STRUCLOC,
/*	OBJID_TYPE_RTTI_TD = 0x7F,//RTTI type descriptor
	OBJID_TYPE_RTTI_BCD,//RTTI base class descriptor
	OBJID_TYPE_RTTI_CHD,//RTTI class hierarchy descriptor
	OBJID_TYPE_RTTI_BCA,//RTTI Base class Array
	OBJID_TYPE_RTTI_COL,//RTTI Complete Object Locator
*/	OBJID_TYPE_PAIR,
#if(!NEW_LOCAL_VARS)
	OBJID_TYPE_UNIONLOC,
#endif
	__OBJID_TYPE__LAST = 0xFF

} ObjId_t; //16


#define	OBJ_TYPE		0x000F	//Mask for ObjId_t in obj's flags

//fields

#define	FLD_OBJTYPE_MASK		OBJ_TYPE
#define	FLD_ATTRIB_MASK			0x00000FF0	//256 values for AttrIdEnum
#define	FLD_ATTRIB_OFFS			4

#define	FLD_FORMAT_MASK			0x00003000	//4 values for format, base on type (like hex,dec,oct,bin for int)	TODO!
#define	FLD_FORMAT_OFFS			12
#define FLD_CONST				0x00004000	//data is constant

//#define FLD_CLONED				0x00010000	//extra field (exported shared code)
//#define FLD_MASTER				0x00020000	//shared field location (see extra fields in seg)
#define	FLD_HARDNAMED			0x00040000	//automatically named (from a import/export symbol), the name cannot be changed
#define	FLD_UGLY_NAME			0x00080000	//the field's name is considered inappropriate for HL SRC context. The associated GLOB may(!) have an alternative (pretty) name in PrettyFieldNames map of the module

#define FLD__1					0x00100000
#define	FLD__2					0x00200000
#define	FLD__3					0x00400000
#define	FLD__4					0x00800000

#define FLD_EXPORTED			0x01000000	//exported
#define	FLD_HIER_PUBLIC			0x04000000
#define	FLD_EOS					0x08000000	//end-of-structure fake field markup

#define	FLD_COLLAPSED			0x40000000
#define FLD_TEMP				0x80000000	//temporary


// typerefs

//#define TYP_USERDEF	0x1000	//defined with user input (not from script)
#define	TYP_ID_MASK				OBJ_TYPE		//0x000F
//#define	TYPEOBJ_ID_OVERIDE		0x00100000		//types created in DC-context and having negative ObjIds

#define TYP_ENUM				0x00000100		//the structure is a enum
#define TYP_HAS_NMAP			0x00000200
#define TYP_HAS_TMAP			0x00000400
#define TYP_NO_REF				0x00000800		//no reference counting
#define	TYP_EXPORTED			0x00001000		//exported types (used in other modules by establishing the proxies)
#define	TYP_UGLY_NAME			0x00002000		//the type has also an inbread name in a map of UglyTypeNames map of a module
#define TYP_PRETTY_NAME			0x00004000
//#define	TYPEOBJ_CLASS		0x00000000		//indicator of a pure struc with member list (as user data); not valid for strucs derivatives

//struc/union/strucvar-specific
#define STRU__TAG_MASK		0xFFFF0000
#define STRU__TAG_SHFT		16


//funcdef - specifics (globs)
#define	FDEF_USERCALL		0x00010000	//non-standard (user) calling convention
#define FDEF_PURGE			0x00020000	//function cleans up stack arguments on exit (as with stdcall)
#define FDEF_VARIARDIC		0x00040000	//for funcs with var args
#define	FDEF_INTRINSIC		0x00080000	//intrinc func
#define FDEF__FMOD_MASK		0x00F00000
#define FDEF_STATIC			0x00100000	//marked as a static function (from symbols)
#define FDEF_VIRTUAL		0x00200000	//marked as a virtual function (from symbols)
#define FDEF_CONST			0x00400000	//marked as a const function (from symbols)
#define FDEF_THISCALL		0x00800000	//marked as thiscall (may not actually be)
#define FDEF_DATA_DONE		0x08000000		//indicates if data reconstruction has been performed in analysis pipeline (skip at re-analysis)
#define FDEF_STATUS_MASK	0xF0000000	//mask for func decompilation status
#define FDEF_DEFINED		0x10000000	//STUBS only: if a stub was never defined - a proto dlg will come up
#define FDEF_DC_FINISHED_OK	0x10000000	//analisys was finished ok
#define FDEF_DC_PHASE0		0x20000000	//func was purged before re-decompiling
#define FDEF_DC_PHASE1		0x30000000	//analysis started and is in progressing UP TO InitialStage_t
#define FDEF_DC_PHASE2		0x40000000	//analysis is progresseing AFTER InitialStage_t
#define FDEF_DC_ERROR		0xF0000000	//an error occured at initial phase, function integrity compromised
//#define	TYP_FDEF_PSNEG			0xC0000000	//function has negative pstack change
//#define	TYP_FDEF_PSNRESET		0xD0000000	//program stack did not reset at ret
//#define	TYP_FDEF_PSNCONV		0xE0000000	//program stack did not converged
//#define	TYP_FDEF_FSNCONV		0xF0000000	//FPU stack top did not converged

#define	GLB_PRETTY_NAME		TYP_PRETTY_NAME		//for scoped fields (or top-levels with ugly names) in SRC context - indicates a pretty name in PretyyFieldsNames map 


//#define	FDEF_UPDCALLERS		0x0000F000	//callers updation needed
//#define FDEF_CHGARGLIST		0x00001000	//func's interface(arglist,retlist) changed, calls must be updated
//#define FDEF_CHGRETLIST		0x00002000	//func's interface(arglist,retlist) changed, calls must be updated
//#define FDEF_CHGSAVREGS		0x00004000	//func's savregs changed, callers must be updated
//#define FDEF_CHGFRAME			0x00008000	//func's frame(stackout,fpuout) changed, callers must be updated
//#define FDEF_REANLZ			0x00010000	//reanalizing

// ops

#define	OPND_TYPE			OBJ_TYPE	//0x000F
//#define	OPND_HIDDEN		0x00000010	//not dead, but also not actual when output collapsed!
#define OPND_XINEX			0x00000020	//possibly has non-registered x-ins
#define	OPND_XOUTEX			0x00000040	//possibly has non-registered x-outs
#define	OPND_NO_REDUCE		0x00000080
#define	OPND_POST			0x00000100	//possible post increment
#define	OPND_INROW			0x00000200
//#define OPND_SEL			0x00000100	//mark as selected
//#define	OPND_OPSEG		0x0000F000

//for save/load only
#define OPND_EX_RESERVED	0x0000FFFF
#define OPND_EX_RI			0x00010000	//2
#define OPND_EX_DISP		0x00040000
#define OPND_EX_DISP0		0x00080000
#define OPND_EX_XIN			0x00100000
#define OPND_EX_LOCALREF	0x00200000
#define OPND_EX_LABELREF	0x00400000

// paths

#define PATH_TYPE_MASK		0x00FFFFFF
#define	PATH_ANALIZED		0x01000000
#define	PATH_INVERTED		0x02000000
//for save/load only
#define PATH_EX_RESERVED	0x0FFFFFFF
#define	PATH_EX_HAS_OPS		0x10000000
#define	PATH_EX_HAS_REFS	0x20000000
//#define	PATH_EX_HAS_ANCHOR	0x40000000


enum FILEID_t
{
	FILEID_NULL,//some file
	FILEID_MODULE,
	FILEID_FOLDER,
	FILEID_RESOURCES,
	FILEID_TYPES,
	FILEID_NAMES,
	FILEID_TEMPL,
	FILEID_EXPORTS,
	FILEID_IMPORTS,
	//...
	FILEID_FILE,	//FileDef_t
	FILEID_STUBS,
	FILEID__TOTAL
};

enum DATAID_t
{
	DATAID_NULL,
	DATAID_SOURCE,
	DATAID_LEECH
};





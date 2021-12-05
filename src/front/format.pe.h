#pragma once

#include "interface/IADCFront.h"
#include "shared.h"

class I_Module;

class PE_FormatterType : public I_FormatterType
{
	bool mb64bit;
public:
	PE_FormatterType(bool b64bit)
		: mb64bit(b64bit)
	{
	}
	virtual const char *name() const { return mb64bit ? "PE64" : "PE32"; }
	virtual void createz(I_SuperModule &r, unsigned long nSize);
};

class COFF_FormatterType : public I_FormatterType
{
public:
	COFF_FormatterType(){}
	virtual const char *name() const { return _PFX("COFF32"); }
	virtual void createz(I_SuperModule &, unsigned long /*nSize*/)
	{
		assert(0);
	}
};

/*class CodeX8632bitType : public I_FormatterType
{
public:
	CodeX8632bitType(){}
	virtual const char *name() const { return _PFX("X86_IA32"); }
	virtual void createz(I_SuperModule &, unsigned long nSize)
	{
		assert(0);//? mr.CreateCode(new IA32Code_t(pSelf, true));
	}
};*/

void PE_RegisterFormatters(I_ModuleEx &);
I_Front *CreateIA32Front(const I_DataSourceBase *, const I_Main*);
I_Front *CreateIA64Front(const I_DataSourceBase *, const I_Main*);
I_Front *CreateIA16Front(const I_DataSourceBase *, const I_Main*);
void PE_createStructures(I_Module &);
void PE_declareDynamicTypes(I_ModuleEx &);
void CORTable_declField(I_Module &, unsigned i, int rows);

class PE_Strucs_t
{
protected:
	I_SuperModule &mr;
	bool	m64bit;
protected:
	PE_Strucs_t(I_SuperModule &r)
		: mr(r),
		m64bit(false)
	{
	}
	void createStructures();
	void createStructures_dotNET();
	void declareDynamicTypes_dotNET();
	void processLoadCOMTable();
protected:
	HTYPE type(OpType_t i){ return mr.type(i); }
	HTYPE type(HNAME n){ return mr.type(n); }
	HTYPE ptrOf(HTYPE t){ return mr.ptrOf(t, m64bit ? I_Module::PTR_64BIT : I_Module::PTR_32BIT); }
	//HTYPE ptrOrRvaOf(HTYPE t){ return m64bit ? mr.ptrOf(t) : mr.type(TYPEID_DWORD); }
	HTYPE arrayOf(HTYPE t, unsigned n){ return mr.arrayOf(t, n); }
	HTYPE arrayOfIndex(HTYPE t, HTYPE i){ return mr.arrayOfIndex(t, i); }
	HTYPE enumOf(HTYPE t, OpType_t i){ return mr.enumOf(t, i); }
	HTYPE pointerType() const { return mr.type(m64bit ? TYPEID_QWORD : TYPEID_DWORD); }
	//HTYPE impPointerType() const { return mr.impOf(mr.ptrOf(nullptr)); }
	HTYPE impPointerType() const { return mr.impOf(mr.type("__imp_ptr")); }
};










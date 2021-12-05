#pragma once

#include "interface/IADCFront.h"

class I_Module;

class MSDOSExecutableType : public I_FormatterType
{
	HTYPE	mpe;
public:
	MSDOSExecutableType(){}
	virtual const char *name() const { return "MSDOS.EXE"; }
	virtual void createz(I_SuperModule &r, unsigned long dataSize);
private:
	void declareDynamicTypes(I_ModuleEx &);
	void createStructures(I_Module &);
};

void EXE_RegisterFormatters(I_ModuleEx &);






#pragma once

#include "shared/front.h"
#include "interface/IADCMain.h"

class FrontMain_t : public I_FrontMain
{
public:
	FrontMain_t(){
	}
	virtual const char *name() const;
	virtual void RegisterTypes(I_ModuleEx &);
	virtual const char *RecognizeFormat(const I_DataSourceBase &, const char *ext);
	virtual I_Front *CreateFrontend(const char *, const I_DataSourceBase *, const I_Main*);
	static const char *name2() { return FENAME_PFX FRONT_NAME; }
};


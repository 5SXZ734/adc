#pragma once

#include "shared/front.h"

void ICO_RegisterFormatters(I_ModuleEx&);
void HSPICE_RegisterFormatters(I_ModuleEx &);

I_Front* CreateFE_SILOS(const I_DataSourceBase* aRaw);




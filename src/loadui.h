#pragma once

#include "qx/IGui.h"


My::IGui* CreateADBGui(int& argc, char** argv, My::IUnk* pICore);
My::IGui* CreateADCGui(int& argc, char** argv, My::IUnk* pICore);

void UnloadGui();

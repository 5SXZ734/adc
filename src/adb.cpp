#include "adb.h"


int main(int argc, char ** argv) 
{
	int iExitStatus(-666);
	try {

		ADB_t adb(argc, argv);

		Main_t* pMain(adb.createMain(argc, argv));
		//pMain->setTimeout(10000);

		My::IGui* pIGui(adb.startGUI(argc, argv, *pMain));
		if (pIGui)
			pIGui->SendEvent(adcui::MSGID_SHOW, NULL);

		adb.hello(*pMain);

		iExitStatus = adb.run(*pMain);

		if (pIGui)
			pIGui->Release();

		delete pMain;
	}
	catch (int e)
	{
		iExitStatus = e;
	}
	catch (...)
	{
	}
	return iExitStatus;
}


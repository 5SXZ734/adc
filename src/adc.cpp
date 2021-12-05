#include "adc.h"

#define NO_TRY	1

int main(int argc, char** argv)
{
	int iExitStatus(-777);
#if(!NO_TRY)
	try
#endif
	{
		ADC_t adb(argc, argv);

		Main_t* pMain(adb.createMain(argc, argv));
		//pMain->setTimeout(10000);

		My::IGui* pIGui(adb.startGUI(argc, argv, *pMain));
		if (pIGui)
			pIGui->SendEvent(adcui::MSGID_SHOW, nullptr);

		adb.hello(*pMain);

		iExitStatus = adb.run(*pMain);

		if (pIGui)
			pIGui->Release();

		delete pMain;
	}
#if(!NO_TRY)
	catch (int e)
	{
		std::cerr << "(!) Exception caught, code=" << e << std::endl;
		iExitStatus = e;
	}
	catch (...)
	{
		std::cerr << "(!) Unhandled exception caught" << std::endl;
	}
#endif
	return iExitStatus;
}


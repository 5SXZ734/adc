#pragma once

#include "adb.h"

#include "dc/main_ex.h"

#define PRODUCT_NAME_EX			"Andrey's Decompiler"
#define PRODUCT_CODENAME_EX		"ADC"

//=================================================== (ADCGuiThread)
class ADCGuiThread : public ADBGuiThread
{
public:
	ADCGuiThread(int argc, char** argv)
		: ADBGuiThread(argc, argv)
	{
	}

protected:
	virtual IGui* NewApplication(IUnk* pICore)
	{
		return CreateADCGui(m_argc, m_argv, pICore);
	}
};


//=================================================== (ADC_t)
class ADC_t : public ADB_t
{
public:
	ADC_t(int argc, char** argv)
		: ADB_t(argc, argv)
	{
#ifdef _DEBUG
#if(0)
		void AVL_test();
		AVL_test();
		throw(0);
#endif
		if (startup.bSymParseTest)
		{
			extern void SymParseTest();
			SymParseTest();
			throw(0);
		}
#endif
	}

	virtual const char* productName() const { return PRODUCT_NAME_EX; }
	virtual const char* productCodeName() const { return PRODUCT_CODENAME_EX; }

	virtual Main_t* createMain(int argc, char** argv) override
	{
		return new Mainx_t(argc, argv, productName(), productCodeName(), G_Version.asString(), COMPANY_NAME, startup);
	}

	virtual ADCGuiThread* newGUI(int argc, char** argv) override
	{
		return new ADCGuiThread(argc, argv);
	}
};



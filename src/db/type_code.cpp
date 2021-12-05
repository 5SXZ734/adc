#include "type_obj.h"
#include "type_code.h"
#include "type_alias.h"
#include "main.h"
#include "info_proj.h"

TypeCode_t::TypeCode_t()
	: mpDevice(nullptr)
{
}

TypeCode_t::~TypeCode_t()
{
	assert(!mpDevice);
}

class DeviceFailure_t : public std::exception
{
public:
	DeviceFailure_t(){}
protected:
	virtual const char * what() const throw (){
		return "CDT-device failure";//context dependet type
	}
};

int TypeCode_t::unassemble(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc)
{
	int iSize(0);
	try
	{
		iSize = dis()->Unassemble(is, base, va, desc);
	}
	catch (const DataAccessFault_t & e)
	{
		MAIN.printError() << "data access fault @" << ProjectInfo_t::VA2STR(va, base != 0, base) << ", (code=" << e.code() << ")" << std::endl;
		return 0;// throw;
	}
	catch (const std::exception &e)
	{
		MAIN.printError() << e.what() << " @" << ProjectInfo_t::VA2STR(va, base != 0, base) << std::endl;
	}
	return iSize;
}

int TypeCode_t::print(DataStream_t &is, ADDR64 base, ADDR va, IOutpADDR2Name *cb, ins_desc_t &desc)
{
	return dis()->Print(is, base, va, cb, desc);
}

int TypeCode_t::skip(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc)
{
	return dis()->Skip(is, base, va, desc);
}

int TypeCode_t::generate(DataStream_t &is, ADDR64 base, ADDR va, ins_desc_t &desc, IPCode_t &pcode, const I_FrontDC &rFrontDC)
{
	int len(dis()->Generate(is, base, va, desc, pcode, rFrontDC));
	if (len <= 0)
		return 0;
	return len;
}

I_Code *TypeCode_t::dis()
{
	if (!mpDevice)
		mpDevice = MAIN.getCodeType(name()->c_str());
		//return mpFrontSeg->typeSeg()->frontEnd()->getDisassembler();
	if (!mpDevice)
		throw DeviceFailure_t();
	return mpDevice;
}

void TypeCode_t::releaseDevice()
{
	if (mpDevice)
	{
		MyString s;
		namex(s);
		MAIN.releaseFrontend(s.c_str());
		mpDevice = nullptr;
	}
}

void TypeCode_t::aka(MakeAlias &buf) const
{
	buf.forTypeCode(ID());
}

/////////////////////////////////////////////// TypeThunk_t

void TypeThunk_t::aka(MakeAlias &buf) const
{
	buf.forTypeThunk(mpCode);
}



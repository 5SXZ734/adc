#pragma once

#include "interface/IADCMain.h"
#include "PE.h"
#include "wintypes.h"

template <typename MyPE_t>
class COFFSymbolProcessor : DataStream_t
{
	const MyPE_t& mrpe;
	adcwin::DWORD mTotal;
	adcwin::DWORD mCur;
public:
	COFFSymbolProcessor(const MyPE_t& pe)
		: DataStream_t(pe.dataSource(), OFF_NULL),//invalid at start
		mrpe(pe),
		mTotal(0),
		mCur(0)
	{
		adcwin::DWORD fp0(mrpe.ImageFileHeader().PointerToSymbolTable);
		if (fp0 != 0)
		{
			mTotal = mrpe.ImageFileHeader().NumberOfSymbols;//numEntries
			seek(fp0);
		}
	}
	bool processNext(I_ModuleCB& rICb, unsigned& progress)//I_ModuleCB::Dump_D_Flags
	{
		using namespace adcwin;
		if (mCur >= mTotal)
			return false;

		assert(!isAtEnd());
		adcwin::IMAGE_SYMBOL_ENTRY r;
		OFF_t oLower(tell());
		read(r);//advance a stream's pointer

		if (r.TypeLo == IMAGE_SYMBOL_TYPE_FUNCTION && r.StorageClass == IMAGE_SYM_CLASS_STATIC && r.SectionNumber > 0)
		{
			if (r.SectionNumber - 1 < mrpe.ImageFileHeader().NumberOfSections)
			{
				rICb.selectFile("from_COFF", nullptr);
				const IMAGE_SECTION_HEADER& ish(mrpe.ImageSectionHeader(r.SectionNumber - 1));
				adcwin::DWORD va(mrpe.RVA2VA(ish.VirtualAddress + r.Value));//+ ish.PointerToRawData);
				if (r.prime.Zeroes == 0)
				{
					OFF_t oName(mrpe.StringTableEntryAtOffset2(r.prime.Offset));
					rICb.dump(va, oName, 0, 0);
				}
				else
				{
					OFF_t oName(oLower/*pSymb.lowerBound()*/ + offsetof(adcwin::IMAGE_SYMBOL_ENTRY, prime.Name));
					unsigned len((unsigned)strnlen(r.prime.Name, sizeof(r.prime.Name)));
					rICb.dump(va, oName, len, 0);//r.prime.Name
				}
			}
		}

		/*BYTE NumberOfAuxSymbols(r.NumberOfAuxSymbols);//r is gonna change below!
		for (adcwin::DWORD j(0); j < NumberOfAuxSymbols; j++, i++, pSymb++)
		{
		//CHECK(i == 4872)
		//STOP
		}*/

		mCur++;
		//pSymb++;
		progress = TProgress(mCur, mTotal);
		return true;
	}
};





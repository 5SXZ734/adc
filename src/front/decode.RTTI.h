#pragma once

#include "wintypes.h"
#include "interface/IADCFront.h"

template <typename T_PE>
class RTTI_Decoder_MSVC_t
{
	typedef typename adcwin::DWORD DWORD;
	typedef typename T_PE::XWORD XWORD;

protected:
	// RTTI support: MSVC
	struct __RTTIClassHierarchyDescriptor {
		DWORD signature;
		DWORD attributes;
		DWORD numBaseClasses;
		DWORD pBaseClassArray;//VA for 32bit, RVA for 64bit(!)
	};
	struct __PMD
	{
		int mdisp;  //member displacement
		int pdisp;  //vbtable displacement
		int vdisp;  //displacement inside vbtable
	};
	struct __RTTIBaseClassDescriptor {
		DWORD pTypeDescriptor;//VA for 32bit, RVA for 64bit(!)
		DWORD numContainedBases;
		__PMD where;
		DWORD attributes;
		DWORD pClassHierarchyDescriptor;//VA for 32bit, RVA for 64bit(!)	//missing in older versions?
	};
	struct _TypeDescriptor
	{
		XWORD pVFTable;
		XWORD spare;
		char name[1];
	};

	const T_PE& mrPE;

public:
	RTTI_Decoder_MSVC_t(const T_PE& r)
		: mrPE(r)
	{
	}

	bool dumpClassHierachyMsvc(const __RTTIClassHierarchyDescriptor& rchd, int level, I_Front::IDumpClassHierarchy* pI)
	{
		if (rchd.numBaseClasses == 0)
			return false;
		DataFetchPtr_t<DWORD> ppbcd(mrPE.dataSource(), mrPE.__VA2FO(rchd.pBaseClassArray));
		if (rchd.numBaseClasses > 1)
		{
			++ppbcd;//first class is always itself (processed in the parent)
			for (DWORD i(1); i < rchd.numBaseClasses; i++, ppbcd++)
			{
				DataFetch2_t<__RTTIBaseClassDescriptor> bcd(mrPE.dataSource(), mrPE.__VA2FO(*ppbcd));
				OFF_t otd(mrPE.__VA2FO(bcd.pTypeDescriptor));
				//DataFetch2_t<_TypeDescriptor> td(dataSource(), otd)
				assert(!bcd.where.pdisp && !bcd.where.vdisp && !bcd.attributes);;
				pI->add(level, otd + offsetof(_TypeDescriptor, name), *ppbcd, bcd.where.mdisp, false/*bcd.where.pdisp, bcd.where.vdisp, bcd.attributes*/);
				DataFetch2_t<__RTTIClassHierarchyDescriptor> chd2(mrPE.dataSource(), mrPE.__VA2FO(bcd.pClassHierarchyDescriptor));
				if (!dumpClassHierachyMsvc(chd2, level + 1, pI))
					return false;
			}
		}
		return true;
	}

	bool dumpClassHierachyMsvc(OFF_t fo, I_Front::IDumpClassHierarchy* pI)
	{
		DataFetch2_t<__RTTIClassHierarchyDescriptor> aRoot(mrPE.dataSource(), fo);
		return dumpClassHierachyMsvc(aRoot, 0, pI);
	}

	bool dumpClassHierachyMsvc64(const __RTTIClassHierarchyDescriptor& rchd, int level, I_Front::IDumpClassHierarchy* pI)
	{
		if (rchd.numBaseClasses == 0)
			return false;
		DataFetchPtr_t<DWORD> pbcd(mrPE.dataSource(), mrPE.__RVA2FO(rchd.pBaseClassArray));
		++pbcd;//first class is always itself (processed in the parent)
		for (DWORD i(1); i < rchd.numBaseClasses; i++, pbcd++)
		{
			DataFetch2_t<__RTTIBaseClassDescriptor> bcd(mrPE.dataSource(), mrPE.__RVA2FO(*pbcd));
			OFF_t otd(mrPE.__RVA2FO(bcd.pTypeDescriptor));
			//DataFetch2_t<_TypeDescriptor> td(dataSource(), otd);
			assert(!bcd.where.pdisp && !bcd.where.vdisp && !bcd.attributes);
			pI->add(level, otd + offsetof(_TypeDescriptor, name), *pbcd, bcd.where.mdisp, false/*bcd.where.pdisp, bcd.where.vdisp, bcd.attributes*/);
			DataFetch2_t<__RTTIClassHierarchyDescriptor> chd2(mrPE.dataSource(), mrPE.__RVA2FO(bcd.pClassHierarchyDescriptor));
			if (!dumpClassHierachyMsvc64(chd2, level + 1, pI))
				return false;
		}
		return true;
	}

	bool dumpClassHierachyMsvc64(OFF_t fo, I_Front::IDumpClassHierarchy* pI)
	{
		DataFetch2_t<__RTTIClassHierarchyDescriptor> aRoot(mrPE.dataSource(), fo);
		return dumpClassHierachyMsvc64(aRoot, 0, pI);
	}
};


///////////////////////////////////////////////////// (RTTI_Decoder_GCC_t)

template <typename T_PE>
class RTTI_Decoder_GCC_t
{
	typedef typename T_PE::XWORD XWORD;

protected:
	//RTTI support: GCC
	struct __class_type_info
	{
		XWORD vptr;
		XWORD type_name;
	};

	struct __si_class_type_info
	{
		XWORD vptr;
		XWORD type_name;
		XWORD base_type;
	};

	struct __base_class_type_info
	{
		XWORD	class_type_info;
		unsigned int	flags : 8;
		int				offset : 24;
		unsigned		__unknown;
	};

	struct __vmi_class_type_info
	{
		XWORD	vptr;
		XWORD	type_name;
		int		flags;
		int		base_count;
		__base_class_type_info base_info[1];//base_count
	};

	const T_PE& mrPE;
	ADDR mVTableAP;
	I_Front::IDumpClassHierarchy* mpI;

public:
	RTTI_Decoder_GCC_t(const T_PE& r, ADDR rvaTI, ADDR vaVT, I_Front::IDumpClassHierarchy *pI)
		: mrPE(r),
		mVTableAP(0),
		mpI(pI)
	{
		if (vaVT != 0)
		{
			XWORD vaTI(mrPE.imageBase() + rvaTI);
			OFF_t oVT(mrPE.__RVA2FO(vaVT));
			//try to figure out where a 'vtable address point' is (right after a VA to type_info?)
			DataFetchPtr_t<XWORD> da3(mrPE.dataSource(), oVT);//@ vtable (not vtable AP!)
			for (size_t i(0); i < 100 && da3; i++, ++da3)//sanity limit
			{
				if (*da3 == vaTI)
				{
					mVTableAP = ADDR(vaVT + (i + 1) * sizeof(XWORD));//assume a next entry
					break;
				}
			}
		}
	}

	bool dump_64(ADDR rvaTypeInfo, int level = 0)
	{
		OFF_t oType(mrPE.__RVA2FO(rvaTypeInfo));

		//try to figure out what kind of inheritance is that. Check for a single one first
		DataFetch2_t<__si_class_type_info> cti2(mrPE.dataSource(), oType);
		if (mrPE.VA2FO(cti2.base_type) != OFF_NULL)//is this OK?
			return dump_SI64(rvaTypeInfo, level);
		//try the multiple one
		//const __vmi_class_type_info *pcti3 = (const __vmi_class_type_info *)pcti2;
		//if (VA2FO(pcti3->class_type_info) == OFF_NULL)
		//return false;
		return dump_VMI64(rvaTypeInfo, level);
	}

	bool dump_SI(ADDR rvaTypeInfo, int level = 0)
	{
		OFF_t oType(mrPE.__RVA2FO(rvaTypeInfo));

		DataFetch2_t<__si_class_type_info> cti(mrPE.dataSource(), oType);
		//DataFetch2_t<__class_type_info> cti2(dataSource(), __VA2FO(cti.class_type_info));//base class
		//0x01: class has non-diamond repeated inheritance
		//0x02: class is diamond shaped
		mpI->add(level, mrPE.__VA2FO(cti.type_name), mrPE.VA2RVA(cti.base_type), 0, false);// 0, 0, 0);
		//__RTTIClassHierarchyDescriptor *pchd2 = (__RTTIClassHierarchyDescriptor *)__RVA2FP(pbcd->pClassHierarchyDescriptor);
		//	if (!dumpClassHierachyMsvc64(pchd2, level + 1))
		//	return false;
		return true;
	}

	bool dump_SI64(ADDR rvaTypeInfo, int level = 0)
	{
		OFF_t oType(mrPE.__RVA2FO(rvaTypeInfo));

		DataFetch2_t<__si_class_type_info> cti(mrPE.dataSource(), oType);
		mrPE.__VA2FO(cti.vptr);//will throw if not OK

		OFF_t oBase(mrPE.__VA2FO(cti.base_type));
		ADDR rvaBase(mrPE.VA2RVA(cti.base_type));

		DataFetch2_t<__class_type_info> cti1(mrPE.dataSource(), oBase);//base class
		mpI->add(level, mrPE.__VA2FO(cti1.type_name), rvaBase, 0, false);// 0, 0, 0);
		return dump_64(rvaBase, level + 1);
	}
	
	bool dump_VMI(ADDR rvaTypeInfo, int level = 0)
	{
		OFF_t oType(mrPE.__RVA2FO(rvaTypeInfo));

		DataFetch2_t<__vmi_class_type_info> chd(mrPE.dataSource(), oType);
		return false;
	}

	bool dump_VMI64(ADDR rvaTypeInfo, int level = 0)
	{
		OFF_t oType(mrPE.__RVA2FO(rvaTypeInfo));
		XWORD vaType(mrPE.imageBase() + rvaTypeInfo);

		DataFetch2_t<__vmi_class_type_info> cti(mrPE.dataSource(), oType);//can be __class_type_info
		mrPE.__VA2FO(cti.vptr);//will throw if not OK
		//0x01: class has non-diamond repeated inheritance
		//0x02: class is diamond shaped
		if (cti.flags & ~3)
			return false;//may happen if __class_type_info
		if (cti.base_count & ~0xFF)
			return false;//too many bases

		//		const char *type_name = (const char *)__VA2FP(pcti->type_name);
		//		mpI->add(level, type_name, pcti->class_type_info, 0, 0, 0, 0);
		if (cti.base_count == 0)
			return false;
		DataFetchPtr2_t<__base_class_type_info> pbcti(mrPE.dataSource(), oType + offsetof(__vmi_class_type_info, base_info));
		for (int i(0); i < cti.base_count; i++, pbcti++)
		{
			enum { e_virtual = 1, e_mublic = 2 };
			if (pbcti->offset == 0 || mVTableAP != 0)
			{
				bool bVirtual((pbcti->flags & e_virtual) != 0);
				int offset(pbcti->offset);
				if (offset < 0)
				{
					assert(bVirtual);
					OFF_t oVTableAP(mrPE.__RVA2FO(mVTableAP));
					DataFetch_t<XWORD> da3(mrPE.dataSource(), oVTableAP + offset);
					offset = int(da3.get());//this shuold apply to the starter class (most derived)
				}

				XWORD vaTypeBase(pbcti->class_type_info);
				OFF_t oTypeBase(mrPE.__VA2FO(vaTypeBase));
				ADDR rvaTypeBase(mrPE.VA2RVA(vaTypeBase));

				DataFetch2_t<__class_type_info> cti2(mrPE.dataSource(), oTypeBase);
				mpI->add(level, mrPE.__VA2FO(cti2.type_name), rvaTypeBase, offset, bVirtual);// 0, 0, 0);
				if (dump_64(rvaTypeBase, level + 1))
					continue;
			}
			fprintf(stdout, "[RTTI] Warning: base class type info entry skipped at index %d (check vtable entry as a static object in class)\n", i);
		}
		return true;
	}
};


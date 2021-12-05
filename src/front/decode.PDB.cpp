#include "PDB.h"

#include <assert.h>
#include <iostream>
#include <fstream>
#include <inttypes.h>

#include "decode.PDB.h"
#include "decode.pe.h"
#include "shared.h"
#include "cv/cvinfo.h"


using namespace adcwin;
using namespace PDB;

#define FOLDER_FROM	"__from_PDB"

static std::string adjusted(const std::string& s0)
{
	std::string s;
	int level(0);
	for (size_t i(0); i < s0.length(); i++)
	{
		char c(s0[i]);
		if (c == '`')
		{
			level++;
			if (i < s0.length() - 1 && isdigit(s0[i + 1]))
				s.append("__l");//is what M$ doing?
		}
		else if (c == '\'')
		{
			if (--level < 0)
			{
				STOP//"`dynamic initializer for 'gCustTypes''"
			}
		}
		else
			s += c;
	}
	return s;
}

////////////////////////////////////////// CTypeFactory

CTypeFactory::CTypeFactory(const pdb::MyPdbReader &reader)
	: mrPdbReader(reader),
	mrTpiStrm(mrPdbReader.Stream(pdb::PDB_STREAM_TPI))
{
}

void CTypeFactory::process(I_Module &rICb, OFF_t oLeaf)
{
CHECK(oLeaf == 0x200818)
STOP
	pdb::MyPdbReader::TypeDesc<pdb::TypeRecord> a(mrTpiStrm, oLeaf);
	if (a.length > 0)
	{
		switch (a.leaf)
		{
		case pdb::LF_CLASS:
		case pdb::LF_STRUCTURE:
		case pdb::LF_CLASS_WTF:
		case pdb::LF_STRUCTURE_WTF:
		{
			processStruct(oLeaf, rICb);
			break;
		}
		case pdb::LF_ENUM:
		{
			processEnum(oLeaf, rICb, false);
			break;
		}
		case pdb::LF_UNION:
		{
			processUnion(oLeaf, rICb);
			break;
		}
		case pdb::LF_PROCEDURE://global
		{
			assert(0);
			//processProcedure(oLeaf, rICb);
			break;
		}
		case pdb::LF_MFUNCTION://class method
		{
			assert(0);
			//processMethod(oLeaf, rICb);
			break;
		}
		default:
			STOP
			break;
		}
	}
}

void CTypeFactory::processArgslist(OFF_t oLeaf, I_Module &rICb)
{
	using namespace pdb;
	DataStream_t ds(mrTpiStrm, oLeaf);//lfArgList
	ds.forward<TypeRecord>();

	unsigned long count(ds.read<unsigned long>());
	for (unsigned long i(0); i < count; i++)
	{
		CV_typ_t typ(ds.read<CV_typ_t>());
		rICb.declField(nullptr, fetchType(typ, rICb));
	}
}

void CTypeFactory::processProcedure(OFF_t oLeaf, I_Module &rICb, const std::string& name0)
{
	using namespace pdb;

	//assert(name.find('`') == std::string::npos);
	std::string name(adjusted(name0));

	if (rICb.NewScope(name.c_str(), SCOPE_FUNC))
	{
		OFF_t oProc(oLeaf);
		oProc += sizeof(uint16_t);//skip reclen

		MyPdbReader::TypeDesc<lfProc> aProc(mrTpiStrm, oProc);

		if (aProc.rvtype)
			rICb.declField(nullptr, fetchType(aProc.rvtype, rICb), ATTRF_RETVAL);

		if (aProc.arglist)
			processArgslist(typeOffsetFromIndex(aProc.arglist), rICb);

		rICb.Leave();
	}
}

void CTypeFactory::processFunction(OFF_t oLeaf, I_Module &rICb, const std::string& name0)
{
	pdb::MyPdbReader::TypeDesc<pdb::TypeRecord> a(mrTpiStrm, oLeaf);
	//assert(name.find('`') == std::string::npos);
	std::string name(adjusted(name0));

	if (a.length > 0)
	{
		if (a.leaf == pdb::LF_PROCEDURE)
		{
			processProcedure(oLeaf, rICb, name);
		}
		else if (a.leaf == pdb::LF_MFUNCTION)
		{
			processMethod(oLeaf, rICb, name);
		}
		else
			ASSERT0;
	}
}

HTYPE CTypeFactory::processProcedureType(OFF_t oLeaf, I_Module &rICb)
{
	OFF_t oProc(oLeaf);
	oProc += sizeof(uint16_t);//skip reclen

	using namespace pdb;
	MyPdbReader::TypeDesc<lfProc> aProc(mrTpiStrm, oProc);
	unsigned flags(0);

	HTYPE hRetVal(aProc.rvtype ? fetchType(aProc.rvtype, rICb) : HTYPE());
	HTYPE hArgs(aProc.arglist ? processArgslistType(typeOffsetFromIndex(aProc.arglist), rICb, nullptr) : HTYPE());
	return rICb.funcTypeOf(hRetVal, hArgs, flags);
}

HTYPE CTypeFactory::processMethodType(OFF_t oLeaf, I_Module &rICb)
{
	OFF_t oProc(oLeaf);
	oProc += sizeof(uint16_t);//skip reclen

	using namespace pdb;
	MyPdbReader::TypeDesc<lfMFunc> aProc(mrTpiStrm, oProc);
	unsigned flags(0);

	HTYPE hRetVal(aProc.rvtype ? fetchType(aProc.rvtype, rICb) : HTYPE());

	HTYPE hThisPtr(aProc.thistype ? fetchType(aProc.thistype, rICb) : HTYPE());

	HTYPE hArgs(aProc.arglist ? processArgslistType(typeOffsetFromIndex(aProc.arglist), rICb, hThisPtr) : HTYPE());
	return rICb.funcTypeOf(hRetVal, hArgs, flags);
}

void CTypeFactory::processVariable(pdb::CV_typ_t typind, I_Module &rICb, const std::string& name0)
{
	//assert(name.find('`') == std::string::npos);
	std::string name(adjusted(name0));
	rICb.declField(name.c_str(), fetchType(typind, rICb));
}

inline bool endsWith(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size())
		return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

static std::string trimSubScope(const std::string& s, const std::string& scope)//the name is expected to start with '::'
{
	size_t m(2), n(0);
	while ((n = s.find("::", n + 1)) != std::string::npos)//start with the next one
	{
		if (endsWith(scope, s.substr(0, n)))
			m = n + 2;//skip scope delim
	}//match as many sections as possible
	return s.substr(m);
}

void CTypeFactory::processMethod(OFF_t oLeaf, I_Module &rICb, const std::string& name0)
{
CHECK(oLeaf == 0xc5348)
STOP
	/*** lfMFunc ***/
	using namespace pdb;

	OFF_t oProc(oLeaf);
	oProc += sizeof(uint16_t);//skip reclen

	std::string name(adjusted(name0));
	MyPdbReader::TypeDesc<lfMFunc> aProc(mrTpiStrm, oProc);
	if (aProc.classtype)
	{
		AttrScopeEnum attr(AttrScopeEnum::null);
		if (!aProc.thistype)
			attr = AttrScopeEnum::ATTRP_STATIC;

		//create a class (if doesn't exist)
		if (rICb.EnterScope(fetchType(aProc.classtype, rICb), 0))
		{
			if (!name.empty())
			{
				//if name of the method provided (supposed to be demangled), verify it is prefixed with a class name (not a case with lambdas!)
				std::string scope;
				fetchSymbolName(aProc.classtype, scope);
				if (name.find(scope) != 0)//name not prefixed with scope?
				{
					//BEWARE of this situation:
					//name: "`TypesViewModel_t::sort'::`2'::less_than_key::compare0"
					//scope: "TypesViewModel_t::sort::__l2::less_than_key"
					//   Looks like M$ lost track of consistency.

					name = scope + "::" + trimSubScope(name, scope);//check if the a is prefixed with sub-scope (relative scope) and return a joint absolute name
				}
			}
			rICb.Leave();
		}

		if (rICb.NewScope(name.c_str(), SCOPE_FUNC, attr))
		{
			if (aProc.rvtype)
				rICb.declField(nullptr, fetchType(aProc.rvtype, rICb), ATTRF_RETVAL);

			if (aProc.thistype)
				rICb.declField(nullptr, fetchType(aProc.thistype, rICb), ATTRF_THISPTR);

			if (aProc.arglist)
				processArgslist(typeOffsetFromIndex(aProc.arglist), rICb);

			rICb.Leave();
		}
	}
	else
		ASSERT0;
}

HTYPE CTypeFactory::processArgslistType(OFF_t oLeaf, I_Module &rICb, HTYPE hArg)
{
	//fprintf(stderr, "[PDB] Test: @TPI=%" PRIx64 "\n", oLeaf);
	using namespace pdb;
	DataStream_t ds(mrTpiStrm, oLeaf);//lfArgList
	ds.forward<TypeRecord>();

	std::list<HTYPE> stack;
	if (hArg)
		stack.push_back(hArg);
	
	unsigned long count(ds.read<unsigned long>());
	for (unsigned long i(0); i < count; i++)
	{
		CV_typ_t typ(ds.read<CV_typ_t>());
		hArg = fetchType(typ, rICb);
		if (!hArg)
		{
			if (typ == 0 && i == count - 1)
				break;
			fprintf(stderr, "Error (PDB): arglist processing failed for arg=#%d @TPI:%" PRIx64 "\n", (int)i, oLeaf);
			break;
		}
		stack.push_back(hArg);
	}

	HTYPE hArgs(nullptr);
	while (!stack.empty())
	{
		hArg = stack.back();
		stack.pop_back();
		if (!hArgs)
			hArgs = hArg;
		else
			hArgs = rICb.pairOf(hArg, hArgs);
	}
	return hArgs;
}

HTYPE CTypeFactory::processStruct(OFF_t oLeaf, I_Module &rICb)
{
	using namespace pdb;
	HTYPE hType(nullptr);
	DataStream_t ds(mrTpiStrm, oLeaf);
	ds.forward<uint16_t>();//reclen
	uint16_t leaf(ds.read<uint16_t>());
	bool bWTF(leaf == LF_CLASS_WTF || leaf == LF_STRUCTURE_WTF);
	if (!bWTF)//skipped for this kind of records?
		ds.forward<unsigned short>();//count
	pdb::CV_prop_t property(ds.read<CV_prop_t>());//property
	if (bWTF)
		ds.forward<uint16_t>();//?
	CV_typ_t field(ds.read<CV_typ_t>());//field
	ds.forward<CV_typ_t>();//derived
	ds.forward<CV_typ_t>();//vshape
	if (bWTF)
	{
		ds.forward<uint16_t>();//something unknown
		uint16_t xz2(ds.read<uint16_t>());
		if (xz2 & 0x8000)
			ds.forward<uint16_t>();
	}
	else
	{
		uint16_t size(ds.read<uint16_t>());//data
		if (size >= LF_NUMERIC)
		{
			value_t v(size);
			skipNumeric(size, ds, v);//primitive types are embeded into the record
		}
	}
	std::string name;
	fetchGlobalName(ds, name, property.fwdref == 0 ? oLeaf : 0);
	if (!name.empty())
	{
//CHECK(name == "tagCY")
//STOP
		if (name == "::<unnamed-tag>")
		{//make it unique
			name += '<';
			name += HexNumberToString(oLeaf);
			name += '>';
		}
		if ((hType = rICb.NewScope(name.c_str(), leaf == LF_CLASS ? SCOPE_CLASS : SCOPE_STRUC)) != HTYPE())
		{
			if (!property.fwdref)
			{
				registerType(oLeaf, hType, name.c_str());
				OFF_t oFields(mrPdbReader.TypeOffsetFromIndex(field));
				processFields(oFields, rICb, false);
			}
			rICb.Leave();
		}
	}
	return hType;
}

void CTypeFactory::registerType(OFF_t oLeaf, HTYPE hType, const char *typeName, bool bEnum)
{
	if (hType)
	{
CHECK(oLeaf == 0xacfd0)
STOP
		if (!bEnum)
			mTypes.insert(std::make_pair(oLeaf, hType));
		else
			mTypesEnum.insert(std::make_pair(oLeaf, hType));
	}
	if (typeName && typeName[0])
	{
		std::pair<std::map<std::string, OFF_t>::iterator, bool> ret;
		ret = mTypeNames.insert(std::make_pair(std::string(typeName), oLeaf));
		if (!ret.second && oLeaf != ret.first->second)
			//NOTE: IPI MAY CONTAIN MULTIPLE REFERENCES TO THE SAME TYPES
			fprintf(stdout, "[PDB] Warning: duplicated type record @TPI:%" PRIx64 " (%" PRIx64 ") - %s\n", oLeaf, ret.first->second, typeName + (typeName[0] == ':' ? 2 : 0));//skip "::"
	}
}

HTYPE CTypeFactory::processUnion(OFF_t oLeaf, I_Module &rICb)
{
CHECK(oLeaf == 0x81430)
STOP
	using namespace pdb;
	DataStream_t o(mrTpiStrm, oLeaf);
	o.skip<TypeRecord>();
	//MyPdbReader::TypeDesc<lfUnion> aUnion(mrTpiStrm, o.current());
	//o.forward(offsetof(lfUnion, data));

	o.skip<unsigned short>();//count
	CV_prop_t property(o.read<CV_prop_t>());
	CV_typ_t field(o.read<CV_typ_t>());

	uint16_t size(o.read<uint16_t>());
	if (size >= LF_NUMERIC)
	{
		value_t v(size);
		skipNumeric(size, o, v);
	}

	std::string s;
	if (!fetchGlobalName(o, s, property.fwdref == 0 ? oLeaf : 0))
		return nullptr;

	HTYPE hType;
	if ((hType = rICb.NewScope(s.c_str()/*, SCOPE_ UNION*/)) != HTYPE())
	{
		if (!property.fwdref)
		{
			registerType(oLeaf, hType, s.c_str());
			OFF_t oFields(mrPdbReader.TypeOffsetFromIndex(field));
			processFields(oFields, rICb, true);
		}
		rICb.Leave();
	}
	return hType;
}

void CTypeFactory::processFields(OFF_t oFields, I_Module &rICb, bool bUnion)
{
	using namespace pdb;
	if (oFields == 0)
		return;//no fields

	rICb.selectFile(nullptr);//reset whatever file the owner is in, field types may reside in different ones

CHECK(oFields == 0x00000000001d661c)
STOP
	DataStream_t o(mrTpiStrm, oFields);

	uint16_t reclen(o.read<uint16_t>());

	OFF_t oEnd(oFields + reclen);
	o.forward<uint16_t>();//length

	while (o.current() < oEnd)
	{
		OFF_t oLeaf(o.current());
CHECK(oLeaf == 0xc6de0)
STOP
		DataFetch_t<uint16_t> leaf(mrTpiStrm, oLeaf);
		switch (leaf)
		{
		case LF_BCLASS:
		{
			assert(!bUnion);
			o.forward<uint16_t>();//leaf
			o.forward<uint16_t>();//attr
			uint32_t index(o.read<uint32_t>());
			int uOffset1(fetchOffset(o));
			AttrIdEnum eAttr(ATTRC_HEIR);
			//if (uOffset1 >= uOffset)
				//uOffset += rICb.skip(uOffset1 - uOffset);
			//if (uOffset1 >= 0)
				rICb.declField(nullptr, fetchType(index, rICb), eAttr, uOffset1);
			o.align<uint32_t>();
			break;
		}
		case LF_VBCLASS:
		case LF_IVBCLASS:
		{
			o.forward<uint16_t>();//leaf
			o.forward<uint16_t>();//attr
			uint32_t index(o.read<uint32_t>());//type index of direct virtual base class
			uint32_t vbptr(o.read<uint32_t>());// type index of virtual base pointer
			o.skip(sizeof(uint16_t));// virtual base pointer offset from address point
			o.skip(sizeof(uint16_t));// followed by virtual base offset from vbtable
			break;//?
		}
		case LF_MEMBER:
		case LF_MEMBER_ST:
		{
			o.forward<uint16_t>();//leaf
			o.forward<uint16_t>();//attr
			uint32_t index(o.read<uint32_t>());//type index

			int uOffset1(fetchOffset(o));
			if (rICb.cp() != uOffset1)
				rICb.setcp(uOffset1);//after U-field?

			std::ostringstream ss2;
			if (!mrPdbReader.fetchName(o, ss2))
				break;
			//o = p.current();

			//int dBytes(uOffset1 - uOffset);
			//if (dBytes >= 0)
			{
				//keep track of current byte offset
				/*if (dBytes > 0)
				{
					uOffset += rICb.skip(dBytes);
					uBitOffset = 0;
				}*/

				HTYPE hType(nullptr);
				OFF_t oType(mrPdbReader.TypeOffsetFromIndex(index));
				if (oType != 0)
				{
					//DataFetch2_t<TypeRecord> h(mrTpiStrm, oType);
					DataStream_t o2(mrTpiStrm, oType);
					o2.forward<uint16_t>();//reclen
					if (o2.read<uint16_t>() == LF_BITFIELD)//leaf
					{
						CV_typ_t type(o2.read<CV_typ_t>());
						uint8_t length(o2.read<uint8_t>());
						uint8_t position(o2.read<uint8_t>());

						//keep track of current bit offset
						//int skipBits((int)position > uBitOffset);
						//if (skipBits > 0)
							//uBitOffset += rICb.skipBits(skipBits);

						hType = fetchBasicType((unsigned short)type, rICb);
						if (hType)
						{
							if (length > 1)
								hType = rICb.arrayOf(hType, length);
							try
							{
								rICb.declBField(ss2.str().c_str(), hType, ATTR_NULL, position);
							}
							catch(int)
							{
							}
							//uBitOffset += length;
						}
						o.align<uint32_t>();
						continue;
					}
					//else
						//uBitOffset = 0;
				}

				hType = fetchType(index, rICb);

				if (bUnion)
					rICb.declUField(ss2.str().c_str(), hType, ATTR_NULL);
				else
					rICb.declField(ss2.str().c_str(), hType, ATTR_NULL);
			}

			o.align<uint32_t>();
			break;
		}
		case LF_METHOD:
		case LF_METHOD_ST:
		{
			o.forward(offsetof(lfMethod, Name));
			mrPdbReader.skipName<char>(o);
			o.align<uint32_t>();
			break;
		}
		case LF_ONEMETHOD:
		case LF_ONEMETHOD_ST:
		{
			o.forward<uint16_t>();//leaf
			uint16_t attr(o.read<uint16_t>());
			uint32_t index(o.read<uint32_t>());//type index
			const CV_fldattr_t &_attr(*(CV_fldattr_t *)&attr);
			if (_attr.mprop == CV_MTintro || _attr.mprop == CV_MTpureintro)
				o.forward<uint32_t>();//vbaseoff
			mrPdbReader.skipName<char>(o);
			o.align<uint32_t>();
			break;
		}
		case LF_VFUNCTAB:
		{
			assert(!bUnion);
			o.forward<uint16_t>();//leaf
			o.skip<uint16_t>();//2 bytes
			//o.forward(sizeof(lfVFuncTab));
			uint32_t index(o.read<uint32_t>());//type index

			HTYPE hType(fetchType(index, rICb));

			rICb.declField("vfptr", hType, ATTR_NULL, 0);

			o.align<uint32_t>();
			break;
		}
		case LF_NESTTYPE:
		case LF_NESTTYPE_ST:
		{
			o.forward(offsetof(lfNestType, Name));
			mrPdbReader.skipName<char>(o);
			o.align<uint32_t>();
			break;
		}
		case LF_STMEMBER:
		case LF_STMEMBER_ST:
		{
			o.forward(offsetof(lfSTMember, Name));
			mrPdbReader.skipName<char>(o);
			o.align<uint32_t>();
			break;
		}
		case LF_INDEX:
		{
			o.forward(sizeof(lfIndex));
			o.align<uint32_t>();
			break;
		}
		default:
			assert(0);
		}
	}
}

HTYPE CTypeFactory::fetchType(unsigned index, I_Module &rICb)
{
	using namespace pdb;
	HTYPE hType;
	OFF_t oLeaf(mrPdbReader.TypeOffsetFromIndex(index));
	if (oLeaf != 0)
	{
CHECK(oLeaf == 0xacfd0)
STOP
		std::map<OFF_t, HTYPE>::const_iterator i(mTypes.find(oLeaf));
		if (i != mTypes.end())
		{
			//assert(i->second == fetchType1(oLeaf, rICb));//a conflict is possible with pre-created types
			return i->second;
		}
		hType = fetchType1(oLeaf, rICb);
		registerType(oLeaf, hType, nullptr);
	}
	else
		hType = fetchBasicType(index, rICb);
	return hType;
}

size_t CTypeFactory::fetchGlobalName(DataStream_t &ds, std::string &s, OFF_t oLeafId)
{
	std::ostringstream ss;
	ss << "::";//should be in global scope
	size_t ret(mrPdbReader.fetchName(ds, ss));
	if (ret != 0)
	{
#if(0)
		//type names are not unique in PDB, provide a way to fetch types by leaf_id by appending special suffixes
		if (oLeafId != 0)
		{
			//if (ret != 0)
			ss << '\t';//DUB_SEPARATOR
			ss << oLeafId;
			ss << '\r';//DUB_TERMINATOR
		}
#endif
		s = ss.str();
		//assert(s.find('`') == std::string::npos);
		//std::string name(adjusted(name0));
	}
	return ret;
	//return s.length();
}

HTYPE CTypeFactory::processPtr(OFF_t oLeaf, I_Module &rICb)
{
	using namespace pdb;
CHECK(oLeaf == 0x16cbdc)
STOP

	DataStream_t o(mrTpiStrm, oLeaf);
	o.forward(sizeof(TypeRecord));

	uint32_t utype(o.read<uint32_t>());
	lfPointerBody::lfPointerAttr attr;
	o.read(attr);
	I_Module::PTR_TYPE_t eType(I_Module::PTR_AUTO);
	if (attr.size == 4)
		eType = I_Module::PTR_32BIT;
	else if (attr.size == 8)
		eType = I_Module::PTR_64BIT;
	if (eType == I_Module::PTR_AUTO)
	{
		switch (attr.ptrtype)
		{
		case CV_PTR_NEAR: // 16 bit pointer
			eType = I_Module::PTR_16BIT;
			break;
		case CV_PTR_NEAR32: // 32 bit pointer
			eType = I_Module::PTR_32BIT;
			break;
		case CV_PTR_64: // 64 bit pointer
			eType = I_Module::PTR_64BIT;
			break;
		default:
			assert(0);//LATER
			return nullptr;
		}
	}

	HTYPE pPointee(fetchType(utype, rICb));
	I_Module::PTR_MODE_t eMode(I_Module::PTR_MODE_PTR);
	if (pPointee)
	{
		switch (attr.ptrmode)
		{
		case CV_PTR_MODE_REF:
		//case CV_PTR_MODE_LVREF:
			eMode = I_Module::PTR_MODE_REF;
			break;
		case CV_PTR_MODE_RVREF:
			eMode = I_Module::PTR_MODE_RVREF;
			break;
		case CV_PTR_MODE_PMEM:
			eMode = I_Module::PTR_MODE_PMEM;//data member
			break;
		case CV_PTR_MODE_PMFUNC:
			eMode = I_Module::PTR_MODE_PMFUNC;//member func
			break;
		}
	}
	HTYPE hPtr(rICb.ptrOf(pPointee, eType, eMode));
	if (attr.isconst)
		return rICb.constOf(hPtr);
	return hPtr;
}

bool CTypeFactory::fetchSymbolName(unsigned index, std::string& name)
{
	OFF_t oLeaf(mrPdbReader.TypeOffsetFromIndex(index));
	using namespace pdb;
	DataStream_t ds(mrTpiStrm, oLeaf);
	TypeRecord h;
	ds.read(h);
	switch (h.leaf)
	{
	case LF_CLASS:
	case LF_CLASS_ST:
	case LF_STRUCTURE:
	case LF_STRUCTURE_ST:
	case LF_CLASS_WTF:
	case LF_STRUCTURE_WTF:
	{
		bool bWTF(h.leaf == LF_CLASS_WTF || h.leaf == LF_STRUCTURE_WTF);
		if (!bWTF)
			ds.forward<unsigned short>();//count
		ds.forward<CV_prop_t>();//property
		ds.forward<CV_typ_t>();//field
		ds.forward<CV_typ_t>();//derived
		ds.forward<CV_typ_t>();//vshape
		if (bWTF)
			ds.forward<CV_typ_t>();//something unknown!
		uint16_t size(ds.read<uint16_t>());//data
		if (size >= LF_NUMERIC)
		{
			value_t v(size);
			skipNumeric(size, ds, v);//primitive types are embeded into the record
		}
		fetchGlobalName(ds, name);
		break;
	}
	default:
		break;
	}
	return true;
}

HTYPE CTypeFactory::fetchType1(OFF_t oLeaf, I_Module &rICb)
{
CHECK(oLeaf == 0xf7570)
STOP
	using namespace pdb;

	//DataFetch2_t<TypeRecord> h(mrTpiStrm, oLeaf);
	DataStream_t o(mrTpiStrm, oLeaf);
	TypeRecord h;
	o.read(h);

	switch (h.leaf)
	{
	case LF_CLASS:
	case LF_CLASS_ST:
	case LF_STRUCTURE:
	case LF_STRUCTURE_ST:
	case LF_CLASS_WTF:
	case LF_STRUCTURE_WTF:
		return processStruct(oLeaf, rICb);
	case LF_UNION:
	case LF_UNION_ST:
		return processUnion(oLeaf, rICb);
	case LF_ENUM:
	case LF_ENUM_ST:
		return processEnum(oLeaf, rICb, true);
	case LF_POINTER:
		return processPtr(oLeaf, rICb);
	case LF_ARRAY:
	case LF_ARRAY_ST:
	{
		uint32_t elemtype(o.read<uint32_t>());
		uint32_t idxtype(o.read<uint32_t>());
		uint16_t length(o.read<uint16_t>());//in BYTES!
		if (length < LF_NUMERIC)
			return rICb.arrayOf(fetchType(elemtype, rICb), length, true);//length is in bytes
		value_t v;
		skipNumeric(length, o, v);
		if (v.ui64h == 0 && v.i32 > 0)
			return rICb.arrayOf(fetchType(elemtype, rICb), v.ui32, true);//length is in bytes
		return nullptr;
	}
	case LF_MODIFIER:
	{
		uint32_t type(o.read<uint32_t>());
		union { uint16_t u16; CV_modifier_t modifier; } u;
		u.u16 = o.read<uint16_t>();
		HTYPE hType(fetchType(type, rICb));
		if (hType && u.modifier.MOD_const)
			return rICb.constOf(hType);
		return hType;
	}
	case LF_PROCEDURE:
		return processProcedureType(oLeaf, rICb);
	case LF_MFUNCTION:
		return processMethodType(oLeaf, rICb);
	case LF_VTSHAPE:
		break;
	default:
		STOP
		break;
	}

	assert(h.leaf != LF_BITFIELD);
	return nullptr;
}

HTYPE CTypeFactory::fetchBasicType(unsigned short index, I_Module &rICb)
{
	using namespace pdb;
	const I_Module::PTR_MODE_t ePtr = I_Module::PTR_MODE_PTR;
	const I_Module::PTR_TYPE_t ePtr32 = I_Module::PTR_32BIT;
	const I_Module::PTR_TYPE_t ePtr64 = I_Module::PTR_64BIT;

	switch (index)
	{
//      Special Types
	case T_VOID: break;
	case T_HRESULT: return rICb.type(TYPEID_ULONG);
	case T_32PVOID: return rICb.ptrOf(fetchBasicType(T_VOID, rICb), ePtr32, ePtr);
	case T_64PVOID: return rICb.ptrOf(fetchBasicType(T_VOID, rICb), ePtr64, ePtr);


	//boolean types
	case T_BOOL08: return rICb.type(TYPEID_BOOL);
	case T_32PBOOL08: return rICb.ptrOf(fetchBasicType(T_BOOL08, rICb), ePtr32, ePtr);	// 32 bit pointer to 8 bit boolean
	case T_64PBOOL08: return rICb.ptrOf(fetchBasicType(T_BOOL08, rICb), ePtr64, ePtr);  // 64 bit pointer to 8 bit boolean


//      Character types
	case T_CHAR: return rICb.type(TYPEID_CHAR8);	// 8 bit signed
	case T_UCHAR: return rICb.type(TYPEID_UINT8);	// 8 bit unsigned
	case T_WCHAR:
	case T_CHAR16: return rICb.type(TYPEID_CHAR16);
	case T_CHAR32: return rICb.type(TYPEID_CHAR32);
	case T_32PCHAR: return rICb.ptrOf(fetchBasicType(T_CHAR, rICb), ePtr32, ePtr);	// 32 bit pointer to 8 bit signed
	case T_64PCHAR: return rICb.ptrOf(fetchBasicType(T_CHAR, rICb), ePtr64, ePtr);	// 64 bit pointer to 8 bit signed
	case T_32PUCHAR: return rICb.ptrOf(fetchBasicType(T_UCHAR, rICb), ePtr32, ePtr);	// 32 bit pointer to 8 bit unsigned
	case T_64PUCHAR: return rICb.ptrOf(fetchBasicType(T_UCHAR, rICb), ePtr64, ePtr);	// 64 bit pointer to 8 bit unsigned

//      really a character types
	case T_RCHAR: return rICb.type(TYPEID_UINT8);	// 8 bit unsigned
	case T_32PRCHAR: return rICb.ptrOf(fetchBasicType(T_RCHAR, rICb), ePtr32, ePtr);	// 32 bit pointer to a real char
	case T_64PRCHAR: return rICb.ptrOf(fetchBasicType(T_RCHAR, rICb), ePtr64, ePtr);	// 64 bit pointer to a real char


	//integer types
	case T_INT1: return rICb.type(TYPEID_INT8);
	case T_INT2:
	case T_SHORT: return rICb.type(TYPEID_SHORT);
	case T_INT4:
	case T_LONG: return rICb.type(TYPEID_LONG);
	case T_INT8:
	case T_QUAD:
		//case T_LONGLONG:
		return rICb.type(TYPEID_LONGLONG);

	case T_UINT1: return rICb.type(TYPEID_UINT8);
	case T_UINT2:
	case T_USHORT: return rICb.type(TYPEID_USHORT);

//      32 bit int types
	case T_ULONG:
	case T_UINT4: return rICb.type(TYPEID_ULONG);
	case T_32PUINT4: return rICb.ptrOf(fetchBasicType(T_UINT4, rICb), ePtr32, ePtr);   // 32 bit pointer to 32 bit unsigned int
	case T_64PUINT4: return rICb.ptrOf(fetchBasicType(T_UINT4, rICb), ePtr64, ePtr);   // 64 bit pointer to 32 bit unsigned int

	case T_UINT8:
	case T_UQUAD:
		//case T_ULONGLONG:
		return rICb.type(TYPEID_ULONGLONG);

//      32 bit real types
	case T_REAL32: return rICb.type(TYPEID_FLOAT);	// 32 bit real
    case T_32PREAL32: return rICb.ptrOf(fetchBasicType(T_REAL32, rICb), ePtr32, ePtr);  // 32 bit pointer to 32 bit real
    case T_64PREAL32: return rICb.ptrOf(fetchBasicType(T_REAL32, rICb), ePtr64, ePtr);  // 64 bit pointer to 32 bit real

//      64 bit real types
	case T_REAL64: return rICb.type(TYPEID_DOUBLE);	// 64 bit real
	case T_32PREAL64: return rICb.ptrOf(fetchBasicType(T_REAL64, rICb), ePtr32, ePtr);	// 32 bit pointer to 64 bit real
	case T_64PREAL64: return rICb.ptrOf(fetchBasicType(T_REAL64, rICb), ePtr64, ePtr);	// 64 bit pointer to 64 bit real

	case T_32PWCHAR: return rICb.ptrOf(fetchBasicType(T_WCHAR, rICb), ePtr32, ePtr);
	case T_64PWCHAR: return rICb.ptrOf(fetchBasicType(T_WCHAR, rICb), ePtr64, ePtr);

	case T_32PINT4: return rICb.ptrOf(fetchBasicType(T_INT4, rICb), ePtr32, ePtr);
	case T_64PINT4: return rICb.ptrOf(fetchBasicType(T_INT4, rICb), ePtr64, ePtr);

	case T_32PUQUAD: return rICb.ptrOf(fetchBasicType(T_UQUAD, rICb), ePtr32, ePtr);
	case T_64PUQUAD: return rICb.ptrOf(fetchBasicType(T_UQUAD, rICb), ePtr64, ePtr);

	case T_32PQUAD: return rICb.ptrOf(fetchBasicType(T_QUAD, rICb), ePtr32, ePtr);
	case T_64PQUAD: return rICb.ptrOf(fetchBasicType(T_QUAD, rICb), ePtr64, ePtr);

	case T_32PUSHORT: return rICb.ptrOf(fetchBasicType(T_USHORT, rICb), ePtr32, ePtr);
	case T_64PUSHORT: return rICb.ptrOf(fetchBasicType(T_USHORT, rICb), ePtr64, ePtr);

	case T_32PLONG: return rICb.ptrOf(fetchBasicType(T_LONG, rICb), ePtr32, ePtr);
	case T_64PLONG: return rICb.ptrOf(fetchBasicType(T_LONG, rICb), ePtr64, ePtr);

	case T_32PULONG: return rICb.ptrOf(fetchBasicType(T_ULONG, rICb), ePtr32, ePtr);
	case T_64PULONG: return rICb.ptrOf(fetchBasicType(T_ULONG, rICb), ePtr64, ePtr);

	default:
		STOP
		break;
	}
	return nullptr;
}

int CTypeFactory::fetchOffset(DataStream_t &o)
{
	using namespace pdb;
	uint16_t offset(o.read<uint16_t>());
	//DataFetch_t<uint16_t> offset(mrTpiStrm, o);//or type
	//o += offset.size();
	if (offset < LF_NUMERIC)
		return (int)offset;
	value_t v;
	if (!skipNumeric(offset, o, v) || v.i32 < 0)//no 64-bit offsets or large 32s!
		return -1;
	return v.i32;
}


void CTypeFactory::processEnumFields(OFF_t oFields, int count, I_Module &rICb)
{
	using namespace pdb;
	DataStream_t o(mrTpiStrm, oFields);
	o.skip<TypeRecord>();

	for (int i(0); i < count; i++)
	{
		uint16_t leaf(o.read<uint16_t>());
		if (leaf != LF_ENUMERATE)
			break;
		CV_fldattr_t attr(o.read<CV_fldattr_t>());
		(void)attr;
		uint16_t value(o.read<uint16_t>());//or type

		bool bOk(true);
		value_t v(value);
		if (value >= LF_NUMERIC)
			bOk = skipNumeric(value, o, v);

		std::ostringstream ss2;
		if (!o.fetchString(ss2))
			break;
		if (bOk)
			rICb.declEField(ss2.str().c_str(), v.i32);
		o.align<uint32_t>();
	}
}

HTYPE CTypeFactory::processEnum(OFF_t oLeaf, I_Module &rICb, bool bRef)
{
	using namespace pdb;

	HTYPE hType(nullptr);
	std::map<OFF_t, HTYPE>::const_iterator it(mTypesEnum.find(oLeaf));
	if (it != mTypesEnum.end())
		hType = it->second;

	DataStream_t o(mrTpiStrm, oLeaf);
	o.skip<TypeRecord>();

    unsigned short count(o.read<unsigned short>());//count
    CV_prop_t property(o.read<CV_prop_t>());
    uint32_t utype(o.read<uint32_t>());
    CV_typ_t field(o.read<CV_typ_t>());

	if (!hType)
	{
		std::string s;
		if (!fetchGlobalName(o, s, property.fwdref == 0 ? oLeaf : 0))
			return HTYPE();

		//CHECK(ss.str() == "TYPE_ENUM_e")
		//STOP

		if ((hType = rICb.NewScope(s.c_str(), SCOPE_ENUM)) != HTYPE())
		{
			if (!property.fwdref)
			{
				registerType(oLeaf, hType, s.c_str(), true);//(!) don't register enum targets
				processEnumFields(mrPdbReader.TypeOffsetFromIndex(field), count, rICb);
			}
			rICb.Leave();
		}
	}

	if (bRef)
	{
		OpType_t tid(TYPEID_INT);
		if (!property.fwdref)
		{
			switch (utype)
			{
			case T_CHAR:
				tid = TYPEID_CHAR; break;
			case T_RCHAR:
			case T_UCHAR:
				tid = TYPEID_UINT8; break;
			case T_WCHAR:
			case T_USHORT:
				tid = TYPEID_USHORT; break;
			case T_CHAR32:
			case T_UINT4:
				tid = TYPEID_UINT; break;
			case T_UQUAD:
				tid = TYPEID_ULONGLONG; break;
			default:
				assert(utype == T_INT4);
				break;
			}
		}
		hType = rICb.enumOf(hType, tid);
	}
	return hType;
}

bool CTypeFactory::skipNumeric(unsigned short value, DataStream_t &o, value_t &v)
{
	switch (value)
	{
	case pdb::LF_CHAR:
		v.i32 = o.read<char>();
		break;
	case pdb::LF_SHORT:
		v.i32 = o.read<short>();
		break;
	case pdb::LF_USHORT:
		v.ui32 = o.read<unsigned short>();
		break;
	case pdb::LF_LONG:
		v.i32 = o.read<long>();
		break;
	case pdb::LF_ULONG:
		v.ui32 = o.read<unsigned long>();
		break;
	case pdb::LF_QUADWORD:
		v.i64 = o.read<long long>();
		return false;//not supported yet?
	case pdb::LF_UQUADWORD:
		v.ui64 = o.read<unsigned long long>();
		return false;//not supported yet?
	default:
		assert(0);
		break;
	}
	return true;
}



////////////////////////////////////////////////////////////////////////// CIpiTypeGenerator

CIpiTypeGenerator::CIpiTypeGenerator(const pdb::MyPdbReader &reader)
	: CTypeFactory(reader),
	mit(mrPdbReader)
{
}

bool CIpiTypeGenerator::process(I_Module &rICb)//, unsigned &)//progress)
{
	if (!mit)
		return false;

	if (mit.leafType() == pdb::LF_UDT_MOD_SRC_LINE)
	{
		std::ostringstream ss1;
		if (mit.source(ss1))
		{
			MyString s(ss1.str());
//CHECK(s.endsWith("\\pe.h"))
//STOP
			rICb.selectFile(s.c_str(), FOLDER_FROM);
		}
		else
			rICb.selectFile(nullptr);//default one
		OFF_t oLeaf(typeOffsetFromIndex(mit.typeIndex()));
		//OFF_t oLeaf2(mit.ITypeOffsetFromIndex());
//CHECK(oLeaf == 0x4c0b8)
//STOP
		CTypeFactory::process(rICb, oLeaf);
	}
	//progress = mit.progress();
	return ++mit;
}

/*HTYPE CIpiTypeGenerator::NewScope(const char *typeName, SCOPE_enum eScope, const char *fieldName, AttrIdEnum attr, I_Module &rICb, OFF_t oLeaf)
{
	HTYPE h(CTypeFactory::NewScope(typeName, eScope, fieldName, attr, rICb, oLeaf));
	if (h)
		registerTypeName(typeName, oLeaf);
	return h;
}*/









template <typename T_PE>
CSymbolProcessor<T_PE>::CSymbolProcessor(const T_PE& rpe)
	: mrpe(rpe),
	mpIPdb(nullptr),
	mpPdbReader(nullptr),
	mpTypeFactory(nullptr),
	mpIpiTypeGenerator(nullptr),
	mpModuleSymbolProcessor(nullptr),
	mpPublicSymbolProcessor(nullptr),
	mPhase(0)
{
}

template <typename T_PE>
CSymbolProcessor<T_PE>::~CSymbolProcessor()
{
	delete mpPublicSymbolProcessor;
	delete mpModuleSymbolProcessor;
	delete mpTypeFactory;
	if (mpIPdb)
		mpIPdb->release();
	delete mpPdbReader;
}

template <typename T_PE>
void CSymbolProcessor<T_PE>::createProcessors(bool bNoTypeInfo, bool bNoPubSyms)
{
	//	dumpLines();
		//return;

	// *** TPI stream can be missing or incomlete
	if (!bNoTypeInfo && mpPdbReader->StreamTpiPtr())
	{
		if (mpPdbReader->StreamIpiPtr())
		{
			mpIpiTypeGenerator = new CIpiTypeGenerator(*mpPdbReader);
			mpTypeFactory = mpIpiTypeGenerator;//weak!
		}
		else
			mpTypeFactory = new CTypeFactory(*mpPdbReader);
	}
	
	// *** Per-object file CodeView symbols may be stripped or incomplete (/PDBSTRIPPED)
	mpModuleSymbolProcessor = new CModuleInfoWalker(*mpPdbReader);
	/*if (!(*mpModuleSymbolProcessor))
	{
			delete mpModuleSymbolProcessor;
			mpModuleSymbolProcessor = nullptr;
			//if no modinfo available, run through symrec stream
	}*/
	
	if (!bNoPubSyms)
	{
		const pdb::DbiStreamHeader& dbi(mpPdbReader->DebugInfoHeader());
		if (dbi.SymRecordStream != 0xFFFF)//even this one can be off-limits
			mpPublicSymbolProcessor = new CCodeViewSymbolWalker(mpPdbReader->Stream(dbi.SymRecordStream), OFF_NULL, false);//do not check modstrm signature
	}
}

template <typename T_PE>
int CSymbolProcessor<T_PE>::createA(const std::string& pdbPath0)
{
	int aVersion(2);

	ADCPE::PDB20_t aPdb20;
	ADCPE::PDB70_t aPdb70;

	std::string pdbPath(pdbPath0);
	if (!mrpe.CheckProgramDatabase20(aPdb20, pdbPath))
	{
		if (!mrpe.CheckProgramDatabase70(aPdb70, pdbPath))
			return 0;
		aVersion = 7;
		if (pdbPath.empty())
			pdbPath = aPdb70.path;
	}
	else if (pdbPath.empty())
		pdbPath = aPdb20.path;

	size_t n(pdbPath.rfind(DIR_SEP_WIN));//only windows path expected
	std::string pdbName(pdbPath.substr(n + 1));
	std::string pdbDir(pdbPath.substr(0, n));
#ifdef WIN32
	std::transform(pdbDir.begin(), pdbDir.end(), pdbDir.begin(), ::tolower);
#endif

	std::string myDir(mrpe.dataSource().modulePath());
	//size_t n(s2.rbegin('\\'));
	//if (n != std::string::npos)
	myDir.resize(myDir.rfind(DIR_SEP));
#ifdef WIN32
	std::transform(myDir.begin(), myDir.end(), myDir.begin(), ::tolower);
#endif
	std::string sPdbPath2((myDir != pdbDir) ? myDir + DIR_SEP + pdbName : "");

	I_DataSourceBase* pIPdb(nullptr);
#ifdef WIN32//cannot use a windows path to open a file on UNIX
	pIPdb = mrpe.dataSource().openFile(pdbPath.c_str());
	if (pIPdb)
	{
		mpPdbReader = new pdb::MyPdbReader(*pIPdb, aVersion);

		bool bOk(true);
		if (aVersion == 2)
		{
			if (!mpPdbReader->checkSignature(aPdb20))
				bOk = false;
		}
		else if (aVersion == 7)
			if (!mpPdbReader->checkSignature(aPdb70))
				bOk = false;
		if (bOk)
		{
			mpIPdb = pIPdb;
			printMsg(true, aVersion);
			return 1;
		}
		delete mpPdbReader;
		mpPdbReader = nullptr;
	}
#endif
	//try a pdb next to the binary, as this one may have been copied to
	I_DataSourceBase* pIPdb2(sPdbPath2.empty() ? nullptr : mrpe.dataSource().openFile(sPdbPath2.c_str()));
	if (pIPdb2)
	{
		mpPdbReader = new pdb::MyPdbReader(*pIPdb2, aVersion);
		if (pIPdb)//if both pdbs are mismatched: prefer the one next to the executable
			pIPdb->release();
		mpIPdb = pIPdb2;
		if (aVersion == 2)
		{
			if (!printMsg(mpPdbReader->checkSignature(aPdb20), aVersion))
			{
				DWORD sig;
				DWORD age(mpPdbReader->getIDinfo(sig));
				fprintf(stdout, "\t%08X, age %d\n", aPdb20.Signature, aPdb20.Age);
				fprintf(stdout, "\t%08X, age %d (PDB)\n", sig, age);
			}
		}
		else if (aVersion == 7)
		{
			if (!printMsg(mpPdbReader->checkSignature(aPdb70), aVersion))
			{
				GUID guid;
				unsigned long age(mpPdbReader->getIDinfo(guid));
				fprintf(stdout, "\t%s, age %d\n", guid_to_string(aPdb70.guidSig).c_str(), (int)aPdb70.age);
				fprintf(stdout, "\t%s, age %d (PDB)\n", guid_to_string(guid).c_str(), (int)age);
			}
		}
		return 1;
	}
	if (pIPdb)
	{
		mpPdbReader = new pdb::MyPdbReader(*pIPdb, aVersion);
		mpIPdb = pIPdb;
		printMsg(false, aVersion);
		return 1;
	}
	return -1;
}

template <typename T_PE>
void CSymbolProcessor<T_PE>::fetchOwnerFile(I_Module& rICb, uint16_t seg, uint32_t off)
{
	std::string s2;
	std::ostringstream ss2;
	if (mpModuleSymbolProcessor->sourceName(seg, off, ss2))
		s2 = ss2.str();
	if (!s2.empty())
		rICb.selectFile(s2.c_str(), FOLDER_FROM);
}

template <typename T_PE>
int CSymbolProcessor<T_PE>::fetchGlobalName(DataStream_t& aStrm, std::string& s)
{
	std::ostringstream ss;
	ss << "::";
	mpPdbReader->fetchName(aStrm, ss);
	s = ss.str();
	if (!s.empty())
	{
		//CHECK(s == "main$dtor$0")
		//STOP
		if (s.front() != '~' && s.front() != '`')
		{
			for (size_t i(0); i < s.length(); i++)
				if (s[i] == '?' || s[i] == '$')
					return 1;//mangled
		}
		return 2;//demangled
	}
	return 0;
}

template <typename T_PE>
bool CSymbolProcessor<T_PE>::processNextType(I_ModuleCB& rICb, unsigned&)
{
	// reconstruct user types from PDB
	if (mPhase == 0)
	{
		if (mpIpiTypeGenerator)
		{
			rICb.resetProgress("Type Info", mpIpiTypeGenerator->progress());
			if (mpIpiTypeGenerator->process(rICb))
				return true;//continue the phase
		}
		mPhase++;
	}
	return false;//go to next phase
}

template <typename T_PE>
bool CSymbolProcessor<T_PE>::processNext(I_ModuleCB& rICb, unsigned& progress)//module symbols
{
	if (!mpPdbReader)
		return false;

	if (mPhase < 1)
	{
		if (processNextType(rICb, progress))
			return true;
	}

	if (mpModuleSymbolProcessor && *mpModuleSymbolProcessor)//is at end?
	{
		try
		{
#if(0)
			while (mpModuleSymbolProcessor->streamId() != 14)
				if (!mpModuleSymbolProcessor->nextModule())
					return false;//stop
#endif
//static int zz = 0;

			rICb.resetProgress("Debug Symbols", mpModuleSymbolProcessor->progress());
			OFF_t oLeaf(mpModuleSymbolProcessor->current());
CHECK(oLeaf == 0xd4)
STOP
			std::string sModule(mpModuleSymbolProcessor->moduleName());
			rICb.selectFile(sModule.empty() ? nullptr : sModule.c_str(), FOLDER_FROM);

			OFF_t oSkip;
			if (processSymbol(rICb, mpModuleSymbolProcessor->symbol(), mpModuleSymbolProcessor->dataStream(), oSkip))
			{
				assert(oSkip > mpModuleSymbolProcessor->current());
				mpModuleSymbolProcessor->setCurrent(oSkip);
				assert(mpModuleSymbolProcessor->symbol() == pdb::S_END);//will be skipped by next statement
			}

			++(*mpModuleSymbolProcessor);
			//if (++zz > 100)
			//return false;
			return true;
		}
		catch (int)
		{
			//mPhase = -1;//terminate process
		}
	}

	if (mpPublicSymbolProcessor && *mpPublicSymbolProcessor)
	{
		rICb.resetProgress("Public Symbols", mpPublicSymbolProcessor->progress());
		processPublicSymbol(rICb, mpPublicSymbolProcessor->symbol(), mpPublicSymbolProcessor->dataStream());
		++(*mpPublicSymbolProcessor);
		return true;
	}

	return false;//this gonna terminate the run() loop
}

template <typename T_PE>
void CSymbolProcessor<T_PE>::processPublicSymbol(I_ModuleCB& rICb, pdb::SYM_ENUM_e symb, const DataStream_t& aSrc)
{
	if (symb == pdb::S_PUB32 || symb == pdb::S_PUB32_ST)
	{
		pdb::PUBSYM32 aSym;
		DataStream_t aStrm(aSrc);
		aStrm.read<pdb::PUBSYM32>(aSym);
		if (/*aSym.pubsymflags.fFunction &&*/ aSym.seg != 0)
		{
			aStrm.seek(aSrc.current() + offsetof(pdb::PUBSYM32, name));
			std::ostringstream s;
			mpPdbReader->fetchName(aStrm, s);
			uint32_t iSeg(aSym.seg - 1);
			if (iSeg < mrpe.ImageFileHeader().NumberOfSections)
			{
				const IMAGE_SECTION_HEADER& ish(mrpe.ImageSectionHeader(iSeg));
				DWORD va(mrpe.RVA2VA(ish.VirtualAddress + aSym.off));
CHECK(va == 0x5f4ce134)
STOP
				rICb.dump(va, s.str().c_str(), 0);
			}
			else
			{
				fprintf(stdout, "Error (PDB): Ivalid section index (%d) for pubic symbol @%" PRIx64 ", stream #%d\n",
					iSeg, aSrc.tell(), (int)mpPdbReader->DebugInfoHeader().SymRecordStream);
			}
		}
	}
	else
	{
		STOP
	}
}

template <typename T_PE>
bool CSymbolProcessor<T_PE>::processSymbol(I_ModuleCB& rICb, pdb::SYM_ENUM_e e, const DataStream_t& aSrc, OFF_t& oNext)//return true if oNext is set (for skip support)
{
	switch (e)
	{
	case pdb::S_GPROC32:
	case pdb::S_LPROC32:
	case pdb::S_GPROC32_ST:
	case pdb::S_LPROC32_ST:
	{
		DataStream_t aStrm(aSrc);
		pdb::PROCSYM32 aSym;
		aStrm.read(aSym);
		aStrm.backward(1);//get to the name
		std::string sName;//demangled
		std::string sNameq;//qualified
		int iNameq(fetchGlobalName(aStrm, sNameq));
		if (iNameq == 2)
			std::swap(sName, sNameq);
		//std::cout << s.str() << std::endl;
		//CHECK(sName == "ADCGuiThread")
		//STOP
		if (aSym.seg != 0)
		{
			const IMAGE_SECTION_HEADER& ish(mrpe.ImageSectionHeader(aSym.seg - 1));
			DWORD va(mrpe.RVA2VA(ish.VirtualAddress + aSym.off));
CHECK(va == 0x5F4973B6)
STOP
			if (rICb.EnterSegment(nullptr, va))//default rangeseg
			{
				SAFE_SCOPE_HERE(rICb);

				fetchOwnerFile(rICb, aSym.seg, aSym.off);

				rICb.declPField(sNameq.c_str());//create a procedure here first

				//if (rICb.NewScope(rICb.declField(sNameq.c_str()), SCOPE_PROC) || rICb.EnterScope(va))//failed if the proc exists, should enter it anyway
				{
					//SAFE_SCOPE_HERE(rICb);
					if (aSym.typind)
					{
						CTypeFactory& tp(*mpTypeFactory);
						tp.processFunction(tp.typeOffsetFromIndex(aSym.typind), rICb, sName);//only demangled names
					}
				}
			}
		}

		oNext = aSym.pEnd;//S_END - skip func contents
		return true;
	}
	break;

	case pdb::S_LDATA32:
	case pdb::S_GDATA32:
	case pdb::S_LDATA32_ST:
	case pdb::S_GDATA32_ST:
	{
		DataStream_t aStrm(aSrc);
		pdb::DATASYM32 aSym;
		aStrm.read(aSym);
		aStrm.backward(1);//get to the name
		std::string sName;
		int iNameq(fetchGlobalName(aStrm, sName));//qualified
		if (aSym.seg != 0)
		{
			const IMAGE_SECTION_HEADER& ish(mrpe.ImageSectionHeader(aSym.seg - 1));
			DWORD va(mrpe.RVA2VA(ish.VirtualAddress + aSym.off));
CHECK(va==0x5f4ce0f8)
STOP

			if (rICb.EnterSegment(nullptr, va))//default rangeseg
			{
				//if (rICb.NewScope(mr.declField(iNameq == 1 ? sName.c_str() : nullptr), SCOPE_PROC))
				{
					fetchOwnerFile(rICb, aSym.seg, aSym.off);
					//rICb.dump(va, ss.str().c_str(), 0);

					if (aSym.typind)
					{
						CTypeFactory& tp(*mpTypeFactory);
						tp.processVariable(aSym.typind, rICb, sName.c_str());
					}

					//	rICb.Leave();
				}

				rICb.Leave();
			}
		}
	}
	break;

	case pdb::S_COMPILE2:
	case pdb::S_COMPILE3:
	case pdb::S_OBJNAME:
	case pdb::S_ENVBLOCK:
	case pdb::S_FRAMEPROC:
	case pdb::S_BUILDINFO:
	case pdb::S_THUNK32:
	case pdb::S_END:
	case pdb::S_UDT:
		break;
	default:
		break;
	}

	return false;
}

template <typename T_PE>
void CSymbolProcessor<T_PE>::printModuleSymbols()
{
	for (PDB::CModuleInfoWalker i(*mpPdbReader); i; ++i)
	{
		pdb::SYM_ENUM_e e(i.symbol());
		if (e == pdb::S_GPROC32 || e == pdb::S_LPROC32)
		{
			DataStream_t aStrm(i.dataStream());
			pdb::PROCSYM32 aSym;
			aStrm.read(aSym);
			aStrm.backward(1);//get to the name
			std::ostringstream s;
			aStrm.fetchString(s);
			std::cout << s.str() << std::endl;

			/*if (aSym.seg != 0)
			{
				const IMAGE_SECTION_HEADER& ish(mrpe.ImageSectionHeader(aSym.seg - 1));
				DWORD va(mrpe.RVA2VA(ish.VirtualAddress + aSym.off));
				rICb.dump(va, s.str().c_str(), 0);
			}*/

			assert(aSym.pEnd > i.current());
			i.setCurrent(aSym.pEnd);//S_END
			assert(i.symbol() == pdb::S_END);//will be skipped
		}

		//std::cout << std::endl;



	}
}

template <typename T_PE>
void CSymbolProcessor<T_PE>::dumpLines()
{
	/*for (pdb::MyPdbReader::ModInfoIterator i(*mpPdbReader); i; ++i)
	{
		std::ostringstream s;
		i.moduleName(s);
		std::cout << s.str() << std::endl;
		PDB::CCodeViewSymbolWalker j(i);
		for (; j; ++j)
		{
		}

		for (pdb::MyPdbReader::C13LineInfoIterator j(i); j; ++j)
		{
			STOP
			if (j.sig == pdb::DEBUG_S_FILECHKSMS)
			{
				for (pdb::MyPdbReader::FileChecksumIterator k(j); k; ++k)
				{
					std::ostringstream os;
					k.fetchName(os);
					rICb.dumpSrc(os.str().c_str(), 0);
				}
			}
		}
	}*/
}

template <typename T_PE>
bool CSymbolProcessor<T_PE>::printMsg(bool bMatched, int ver)
{
	fprintf(stdout, "Program DataBase (PDB %d.0) found at %s\n", ver, mpPdbReader->dataSource().modulePath());
	if (!bMatched)
		fprintf(stdout, "Warning: PDB file is mismatched. Symbols may reside at wrong addresses!\n");
	return bMatched;
}







///////////////////////////////////////////////////////////////////// CSymbolProcessorOld

template <typename T_PE>
CSymbolProcessorOld<T_PE>::CSymbolProcessorOld(const T_PE& rpe)
	: CSymbolProcessor<T_PE>(rpe),
	//mpTypeFactory(nullptr),
	mpPublicProcessor(nullptr)
{
}

template <typename T_PE>
CSymbolProcessorOld<T_PE>::~CSymbolProcessorOld()
{
	//delete mpTypeFactory;
	delete mpPublicProcessor;
}

/*template <typename T_PE> bool CSymbolProcessorOld<T_PE>::processNextType(I_ModuleCB& rICb, unsigned&)// progress)
{
	// reconstruct user types from PDB
	if (mpTypeFactory)
	{
		rICb.resetProgress("Type Info", mpTypeFactory->progress());
		if (mpTypeFactory->process(rICb))//, progress))
			return true;
		delete mpTypeFactory;
		mpTypeFactory = nullptr;
	}
	return false;
}*/

template <typename T_PE>
bool CSymbolProcessorOld<T_PE>::processNext(I_ModuleCB& rICb)//, unsigned&)// progress)//global symbols
{
	// reconstruct user types from PDB
	if (this->mpIpiTypeGenerator)
	{
		rICb.resetProgress("Type Info", this->mpIpiTypeGenerator->progress());
		if (this->mpIpiTypeGenerator->process(rICb))//, progress))
			return true;
		delete this->mpIpiTypeGenerator;
		this->mpIpiTypeGenerator = nullptr;
		this->mpTypeFactory = nullptr;
	}

	if (mpPublicProcessor)
	{
		if (*mpPublicProcessor)
		{
			OFF_t oCur(mpPublicProcessor->current());
			rICb.resetProgress("Debug Symbols", TProgress(oCur, mpPublicProcessor->upper()));
			try
			{
				assert(mpPublicProcessor->dataStream().current() == oCur);
				this->processPublicSymbol(rICb, mpPublicProcessor->symbol(), mpPublicProcessor->dataStream());
				//progress = TProgress(oCur, mpPublicProcessor->upper());
				++(*mpPublicProcessor);
				return true;
			}
			catch (int)
			{
			}
		}
		delete mpPublicProcessor;
		mpPublicProcessor = nullptr;
	}
	return false;
}

template <typename T_PE>
void CSymbolProcessorOld<T_PE>::initPdb0()
{
	if (this->mpPdbReader)
	{
		if (this->mpPdbReader->StreamIpiPtr())
			this->mpTypeFactory = new CIpiTypeGenerator(*this->mpPdbReader);
		if (this->mpPdbReader->GlobalsSream().size() > 0)
			mpPublicProcessor = new PDB::CCodeViewSymbolWalker(this->mpPdbReader->GlobalsSream(), 0, this->mpPdbReader->GlobalsSream().size());
	}
}




// Instantiations...

#include "format.pe.h"

template class CSymbolProcessor<PE2_t<PE32_types_t>>;
template class CSymbolProcessor<PE2_t<PE64_types_t>>;

template class CSymbolProcessorOld<PE2_t<PE32_types_t>>;
template class CSymbolProcessorOld<PE2_t<PE64_types_t>>;
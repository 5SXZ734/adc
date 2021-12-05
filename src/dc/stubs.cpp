#include "stubs.h"
#include "prefix.h"
#include <assert.h>
#include <fstream>

#include "front/front_IA.h"
#include "type_funcdef.h"
#include "info_func.h"
#include "sym_parse.h"
#include "dump_proto.h"
#include "ui_main_ex.h"
#include "info_dc.h"
#include "db/script.h"
#include "shared/misc.h"
#include "qx/MyStringList.h"



#ifdef WIN32
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif

#define EATWSPACE(arg)	while ((*arg) && isspace(*arg)) (arg)++;
#define EATNSPACE(arg)	while ((*arg) && !isspace(*arg)) (arg)++;



static int cutCommentOff(char *buf)
{
	char *p = buf;
	while (*p)
	{
		if (*p == ';')
		{
			//cut off trailing whitespaces
			while (p > buf && isspace(*(p - 1)))
				p--;
			*p = 0;
			return 1;
		}
		p++;
	}

	return 0;
}

////////////////////////////////////////// StubParser_t

StubParser_t::StubParser_t(MyPath &rPath)
	: mrPath(rPath)
{
	m_fs.open(mrPath.Path());
	if (m_fs.fail())
	{
		mrPath.SetExt("proto");
		m_fs.open(mrPath.Path());
		if (m_fs.fail())
		{
			//PrintError() << "Could not open stubs file: " << mrPath.Path() << std::endl;
			return;
		}
	}

	//run();
	operator++();
}

StubParser_t::~StubParser_t()
{
	m_fs.close();
}

StubParser_t& StubParser_t::operator ++()
//void StubParser_t::run()
{
	mKey.clear();
	mValue.clear();
	//mAtAddr = 0;
	//mRefMode = REFMODE__BAD;

	while (!m_fs.eof())
	{
		std::string s;
		std::getline(m_fs, s);

		if (!s.empty())
			handleLine(&(*s.begin()));//ignore command execution

		if (!mKey.empty())
			break;
	}

	return *this;
}

int StubParser_t::handleLine(char *buf)
{
	//already exists
	char *p1 = buf;
	EATWSPACE(p1);//eat white
	if (*p1 == 0)
		return 1;//empty line
	char *p2 = p1;

	cutCommentOff(p1);//cut comment off
	if (CheckEmpty(p1))
		return 1;

	if (!parseStubOut(p1))
		return 0;

	return 1;
}

#define EAT2SYM(arg, sym)	while ((*arg) && (*arg != sym)) arg++;

bool StubParser_t::parseStubOut(char *p1)
{
	char *p2 = p1;

	/*if (*p2 == '.')//directive?
	{
		char *p3(++p2);
		EATNSPACE(p3);
		if (strncasecmp(p2, "include", p3 - p2) == 0)
		{
			EATNSPACE(p3);
			MyPath fp(p3);
			StubParser_t parser(mrStubs, fp, mPhase);
			parser.run();
			return true;
		}
		else
		{
			fprintf(stderr, "Error: unknown directive: %s\n", std::string(p2, p3 - p2).c_str());
			return false;
		}
	}*/
	
	if (*p2 == '[')//module begin
	{
		++p2;
		EATWSPACE(p2);
		char *p3(p2);
		EAT2SYM(p3, ']');
		while (p3 > p2 && isspace(*p3)) p3--;
		msModule = std::string(p2, p3 - p2);
		return true;
	}

	while ((*p2) && (*p2 != '=')) p2++;
	if (*p2 == 0)
		fprintf(stderr, "Error: No value in function definition\n");//empty entry - no value

	char *p3 = p2 + 1;
	while ((*p3) && isspace(*p3)) p3++;
	//?	if (*p3 == 0)
	//?		fprintf(STDERR, "Warning: Empty function definition value (%s)\n", p1);

	do { *p2 = 0; p2--; } while (isspace(*p2));
	assert(p2 >= p1);//valid entry name

	p2 = p3;
	while (*p2) p2++;
	while ((p2 > p3) && isspace(*p2)) { *p2 = 0; p2--; }

	mKey = p1;
	mValue = p3;
	return true;
}

//////////////////////////////////////// StubSet

bool StubComparator::operator()(const node_t *a, const node_t *b) const {
	return a->name < b->name;
}
//bool StubComparator::operator()(const node_t *a, const MyString &b) const { return a->name < b; }

StubSet::~StubSet()
{
	while (!empty())
	{
		node_t *pn(*begin());
		erase(begin());
		delete_node(pn);
	}
}

void StubSet::dump(std::ostream &os, int &index) const
{
	TProtoDumper<ProtoImpl4Sym_t> proto(os);
	for (const_iterator i(begin()); i != end(); ++i)
	{
		node_t *node(*i);
		if (index >= 0)
			os << "[" << index++ << "]\t";
		SymParsePrintProto(node, os);
		//SymParsePrintFlat(node, os);
		os << std::endl;
	}
}

node_t *StubSet::findProto(const MyStringEx& aName) const
{
	node_t t(NODE_NULL);
	t.setName(aName[0]);
	StubSet::const_iterator j(find(&t));
	if (j == end())
		return nullptr;
	return *j;
}

node_t* StubSet::findProtoEx(MyStringEx& aName) const
{
	MyString s(aName[1]);
	if (!s.isEmpty())
	{
		unsigned ordinal(atoi(s.c_str()));
		std::map<unsigned, CxxSymbMapCIt>::const_iterator i(mOrdMap.find(ordinal));
		if (i != mOrdMap.end())
		{
			aName.set(0, i->second->first);
			return i->second->second;
		}
		std::map<unsigned, MyString>::const_iterator j(mCSymb.find(ordinal));
		if (j != mCSymb.end())
			aName.set(0, j->second);
	}
	return nullptr;
}

/////////////////////////////////////////StubPool_t

void StubPool_t::dump(std::ostream &os) const
{
	int index(0);
	StubSet::dump(os, index);
	for (PerModuleStubs::const_iterator i(mPerModule.begin()); i != mPerModule.end(); ++i)
	{
		os << "\n[" << i->first << "]\n";
		const StubSet &aSet(i->second);
		aSet.dump(os, index);
	}
}

bool StubPool_t::hasModule(MyString module) const
{
#ifdef WIN32
	module = module.lower();
#endif
	PerModuleStubs::const_iterator i(mPerModule.find(module));
	if (i == mPerModule.end())
		return false;
	return true;
}

StubSet& StubPool_t::assureModule(MyString module)
{
#ifdef WIN32
	module = module.lower();
#endif
	std::pair<PerModuleStubs::iterator, bool> ret;
	ret = mPerModule.insert(std::make_pair(module, StubSet()));
	return ret.first->second;
}


StubSet& StubPool_t::findModule(MyString module)
{
#ifdef WIN32
	module = module.lower();
#endif
	PerModuleStubs::iterator it(mPerModule.find(module));
	if (it == mPerModule.end())
		throw(-1);
	return it->second;
}

node_t *StubPool_t::findProto(MyString symbol, MyString module) const
{
	assert(!module.isEmpty());
#ifdef WIN32
	module = module.lower();
#endif
	PerModuleStubs::const_iterator i(mPerModule.find(module));
	if (i == mPerModule.end())
	{
		//i = mPerModule.insert(std::make_pair(module, StubSet())).first;
		return nullptr;//for WIN32, module is expected to be in lowercase
	}
	const StubSet &aSet(i->second);
	return aSet.findProto(symbol);
}


node_t *StubPool_t::findProtoEx(MyString symbol, MyString module) const
{
	node_t *pn(findProto(symbol, module));
	if (!pn)
		pn = StubSet::findProto(symbol);
	return pn;
}


REFMODE_t StubParser_t::parseAtAddr(const char *p1, ADDR &atAddr, ADDR64 ib)
{
	REFMODE_t refMode(REFMODE_DIRECT);
	std::string s1(p1);
	size_t n(s1.find_last_of('@'));
	if (n == std::string::npos)
		return REFMODE__BAD;

	std::string s4(s1.substr(n + 1));
	if (!s4.empty())
	{
		if (s4[0] == '[')
		{
			s4.erase(0, 1);
			refMode = REFMODE_GLOBAL_PTR;
		}
		else if (s4[0] == '{')
		{
			s4.erase(0, 1);
			refMode = REFMODE_UNK_PTR;
		}
		if (refMode == REFMODE_GLOBAL_PTR)
		{
			if (!s4.empty() && s4[s4.length() - 1] == ']')
				s4.resize(s4.length() - 1);
		}
		else if (refMode == REFMODE_UNK_PTR)
		{
			if (!s4.empty() && s4[s4.length() - 1] == '}')
				s4.resize(s4.length() - 1);
		}
	}
	value_t v;
	if (StrHex2Int(s4.c_str(), v))
	{
		v.ui64 -= ib;
		if (v.ui64h != 0)
			return REFMODE__BAD;
		atAddr = v.i32;
		s1.resize(n);
	}
	else
		return REFMODE__BAD;

	return refMode;
}

/*void StubMgr_t::loadz(MyString path, int pass)
{
	if (pass == 0)
		clear();//?

	//mPath = path;

	StubParser_t it(path);
	//parser.run();

	for (; it; it++)
	{
		process(it.key(), it.value(), pass);
	}

	sort();
}*/

/*void StubMgr_t::sort()
{
	std::sort(begin(), end(), comp_t());
}*/

bool StubInfo_t::SaveOne(const Stub_t& aStub) const
{
	if (mrMain.options().nProtoMode < 2)
		return false;
	if (!mrDC.stubsPtr())
		return false;
	MyString path(ModulePath());
	assert(!IsPhantomModule(ModulePtr()));
	assert(!path.empty());
	MyPath fi(path);
	fi.SetExt("proto");
	std::string sPath(fi.Path());
	std::ofstream ofs;
	ofs.open(sPath.c_str(), std::ios_base::app);//append
	if (ofs.fail())
	{
		PrintError() << "Could not open .PROTO file: " << path << std::endl;
		return false;
	}

	const StubMgr_t& stubs(dc().stubs());
	if (dumpz(aStub, ofs))
		ofs << std::endl;

	ofs.close();
	return true;
}

bool StubInfo_t::SaveStubs(MyString path) const
{
	if (mrMain.options().nProtoMode < 2)
		return false;
	if (!mrDC.stubsPtr())
		return false;
	if (path.isEmpty())
		path = ModulePath();
	assert(!IsPhantomModule(ModulePtr()));
	assert(!path.empty());
	MyPath fi(path);
	fi.SetExt("proto");
	std::string sPath(fi.Path());
	std::ofstream ofs;
	ofs.open(sPath.c_str());
	if (ofs.fail())
	{
		PrintError() << "Could not open .PROTO file: " << path << std::endl;
		return false;
	}

	StubMgr_t &stubs(dc().stubs());
//	if (!mbDirty)
//		return;
	//stubs.dump(ofs);
	//if (!empty())
	{
		ofs << ";[stubs]" << std::endl;
		//MyString module;
		for (StubCIt it(stubs.begin()); it != stubs.end(); it++)
		{
			if (dumpz(it->second, ofs))//, module))
				ofs << std::endl;
		}
		ofs << std::endl;
	}

	//ImpLookupMap lImp;
	//BuildImpLookupMap(lImp);

	//append dirty stubs from phantom binaries (only if used)
	std::string module;
	const FoldersMap &m(mrProject.rootFolder().children());
	for (FoldersMap::const_iterator i(m.begin()); i != m.end(); i++)
	{
		CFolderRef rFolder(*i);
		if (!IsPhantomFolder(rFolder))
			continue;
		Dc_t *pDC2(DcFromFolder(rFolder));
		if (!pDC2 || !pDC2->stubsPtr())
			continue;
		StubMgr_t &stubs2(pDC2->stubs());
		for (StubCIt it(stubs2.begin()); it != stubs2.end(); it++)
		{
			const Stub_t &aStub(it->second);
			if (/*aStub.isMismatched() ||*/ aStub.isModified())
			{
				assert(0);
				/*if (DumpStubImported(aStub, ofs, module, lImp))//module will be updated
					ofs << std::endl;*/
			}
		}
		ofs << std::endl;
	}

	ofs.close();
	stubs.setDirty(false);
	fprintf(stdout, "PROTO file updated: %s\n", fi.Path().c_str());
	return true;
}

bool StubInfo_t::LoadStubs(MyString path)//const MyString &path)
{
	//load stubs associated with a front end
	//build an import lookup map
	//ImpLookupMap lImp;
	//BuildImpLookupMap(lImp);

/*zz
	MyString frontName(mrFrontDC.Name());
	MyPath path3(mrMain.frontPath(frontName));
	MyPath path1(mrMain.exePath());
	path1.SetName(path3.Name());
	path1.SetExt("proto");	
	
	for (StubParser_t i(path1); i; ++i)
	{
		ProcessStub(i.key(), i.value(), 0, lImp);
	}*/

	
	//load stubs associated with a project
	MyPath myPath;
	if (path.empty())
	{
		myPath = MyPath(ModulePath());
		myPath.SetExt("proto");
	}
	else
		myPath.SetPath(path);
	//stubs.loadz(path2, 1);//do not clear

	for (StubParser_t j(myPath); j; ++j)
	{
		ProcessStub(j.key(), j.value(), 0);// , lImp);
	}

	return true;
}

MyString StubInfo_t::ValueFromStub(const Stub_t &a) const
{
CHECK(a.atAddr() == 0x1001094)
STOP

	/*if (a.value().empty())
	{
		//DcInfo_t DI(mrDcRef);
		FieldPtr pField(a.field());
		if (pField)
		{
			GlobPtr ifDef(IsCFuncOrStub(pField));
			if (!ifDef)
				ifDef = IsPtrToCFunc(pField);
			if (ifDef)
				return FuncDefToStubStr(ifDef);
		}
	}*/
	return a.value();
}

MyString StubInfo_t::name(const Stub_t&a) const
{
	Locus_t loc;
	//DcInfo_t dcInfo(mrDcRef);
	FieldPtr pField(FindFieldRef(a.atAddr(), REFMODE_DIRECT, nullptr));
	if (pField)
	{
		//FileInfo_t DI(mrDcRef);
		return FieldName(pField);
	}
	return MyString();
}

/*StubIt StubInfo_t::FindStubByNameIt(const char *stub_name) const
{
	StubMgr_t &rSelf(mrDC.stubs());
	if (!rSelf.empty())
	{
		assert(stub_name);
		for (StubIt i(rSelf.begin()); i != rSelf.end(); i++)
		{
			if (MyString(stub_name) == name(i->second))
				return i;
		}
	}
	return rSelf.end();
}*/

Stub_t*	StubInfo_t::FindStub(ADDR addr) const
{
	if (mrDC.stubsPtr())
		return mrDC.stubs().findStub(addr);
	return nullptr;
}

const Stub_t* StubInfo_t::TouchStub(ADDR atAddr, const MyString& sValue)
{
	if (mrDC.stubsPtr())
		return mrDC.stubs().touchStub(atAddr, sValue);
	return nullptr;
}

const Stub_t* StubInfo_t::MakeStub(ADDR atAddr, REFMODE_t refMode, const MyString& sValue)
{
	Stub_t& aStub = InsertStub(atAddr, refMode, sValue);
	aStub.setModified(true);
	return &aStub;
}

/*StubIt StubInfo_t::FindStubIt(ADDR addr) const
{
	StubMgr_t &rSelf(mrDC.stubs());
	return rSelf.find(addr);
}*/

FieldPtr StubInfo_t::FindFieldRef(ADDR atAddr, REFMODE_t refMode, CTypePtr pScope) const
{
	FieldPtr pField(nullptr);
	if (refMode != REFMODE_UNK_PTR)
	{
		if (!IsPhantomModule(mrDC.module()))
		{
			Locus_t loc;
			if (FindFieldInSubsegs(PrimeSeg(), atAddr, loc))
			{
				pField = loc.asProc();
				if (!pField)
					pField = loc.field0();
			}
		}
		else
			pField = Field(PrimeSeg(), atAddr);
	}
	else
	{
		STOP//assert(0);
	}
	return pField;
}

Stub_t &StubInfo_t::InsertStub0(StubMgr_t &rSelf, ADDR atAddr, REFMODE_t refMode, const MyString &value)
{
	return rSelf.insert(atAddr, refMode, value);
}

Stub_t &StubInfo_t::InsertStub(ADDR atAddr, FieldPtr pField, REFMODE_t refMode, const MyString &value)
{
	FolderPtr pFolder(AssureFolderOfKind(FTYP_PROTOTYPES));
	if (!pFolder->hasPvt())
		pFolder->SetPvt(new FileStubs_t);

	StubMgr_t &rSelf(mrDC.stubs());
//CHECK(atAddr == 0x1001094)
//STOP
	
	//this yet may be an imported symbol
	if (pField && pField->isTypeImp())
	{
		GlobPtr ifDef(IsPtrToCFunc(pField));
		if (ifDef)
		{
			Stub_t &aStub(InsertStubFromField(atAddr, pField));
			if (!value.empty())
			{
				//DcInfo_t DI(mrDcRef);
				if (FuncDefToStubStr(ifDef) != value)
				{
					aStub.setValue2(value);
					//aStub.setModified();
				}
			}
			return aStub;
		}
	}

	return InsertStub0(rSelf, atAddr, refMode, value);
}

Stub_t &StubInfo_t::InsertStub(ADDR atAddr, REFMODE_t refMode, const MyString &value)
{
	//DcInfo_t dcInfo(mrDcRef);
	Locus_t loc;
	FieldPtr pField(FindFieldInSubsegs(PrimeSeg(), atAddr, loc));
	return InsertStub(atAddr, pField, refMode, value);
}

Stub_t&	StubInfo_t::InsertStubFromField(ADDR atAddr)
{
	Locus_t loc;
	FieldPtr pField(FindFieldInSubsegs(PrimeSeg(), atAddr, loc));
	assert(pField);
	return InsertStubFromField(atAddr, pField);
}

Stub_t&	StubInfo_t::InsertStubFromField(ADDR atAddr, FieldPtr pField)
{
	StubMgr_t &rSelf(mrDC.stubs());
	return rSelf.insertStubFromField(atAddr, pField);
}

bool StubInfo_t::AddStub(ADDR atAddr, REFMODE_t refMode, const char *value)
{
	FolderPtr pFolder(AssureFolderOfKind(FTYP_PROTOTYPES));
	if (!pFolder->hasPvt())
	{
		pFolder->SetPvt(new FileStubs_t);
		//pFolder->fileStubs()->setStubsPtr(&mrDC.stubs());
	}

	StubMgr_t &stubs(mrDC.stubs());
	if (stubs.update(atAddr, refMode, value))
		return true;

	if (atAddr == 0)
		return false;

	Stub_t &rStub(InsertStub(atAddr, REFMODE_t(refMode & REFMODE__MASK), value));
	if (refMode & STUB_MODIFIED)
		rStub.setModified(true);
	return true;
}

/*void StubInfo_t::BuildImpLookupMap(ImpLookupMap &lImp) const
{
	for (auto i(mrDC.m_imp.begin()); i != mrDC.m_imp.end(); i++)
	{
		ADDR impVA(i->first);
		const Dc_t::ImpEntry_t &rr(i->second);
		FieldPtr pImpField(FindGlobal(impVA));
		if (pImpField->name())
		{
			FolderPtr pExpFolder(rr.pModule->typeModule()->folderPtr());
			lImp.insert(std::make_pair(pImpField->name()->c_str(), std::make_pair(pImpField->address(), pExpFolder)));
		}
	}
}*/


bool StubInfo_t::ProcessStub(const char* key, const char* value, int phase)// , const ImpLookupMap& lImp)
{
	//process only symbols imported by this module, skip the others

	ADDR atAddr(0);
	MyString sModuleName;
	REFMODE_t refMode(REFMODE__BAD);
/*zz	if (*key != '@')//import name?
	{
		atAddr = nameToField(key, sModuleName, lImp);
		if (!atAddr)
		{
			//PrintError() << "Unrecognized key in stub specification: " << p1 << std::endl;
			//return false;
			return true;//no such name in the front end, just ignore it;
		}
		//mrStubs.InsertStubFromField(atAddr);
		if (!sModuleName.empty())
			refMode = REFMODE_DIRECT;
		else
			refMode = REFMODE_GLOBAL_PTR;
		if (phase == 1)//will have to save it
			refMode = REFMODE_t(refMode | STUB_MODIFIED);
		//return true;
	}
	else*/
		refMode = StubParser_t::parseAtAddr(key, atAddr, ImageBase());

CHECK(atAddr == 0x206AD6C8)
STOP

	if (refMode == REFMODE__BAD)
		PrintError() << "Incorrect format in function profile: " << key << std::endl;

	//this yet may be an imported function
	bool bRet(false);
/*zz	if (!sModuleName.empty())
	{
		NameRef_t aName;//temporary stuff
		aName.setPvt0((char *)key);
		Field_t aField(atAddr, 0);//avoid objid increment
		aField.setName(&aName);
		aField.setOwnerComplex(PrimeSeg());
		assert(aField.address() == atAddr);//aField.setOffset(atAddr);
		FieldPtr pExpField(ToExportedField(&aField));
		aName.clearPvt();
		aField.setName(nullptr);

		if (pExpField)
		{
			if (!OfTheSameModule(USERFOLDER(pExpField), FindModuleFolder(sModuleName)))
				return false;
			Dc_t *pDC2(DcFromFolder(*USERFOLDER(pExpField)));
			DcInfo_t dcInfo2(*pDC2);
			bRet = dcInfo2.AddStub(pExpField->address(), refMode, value);
		}
		else
		{
			//ASSERT0
			PrintError() << "Imported symbol not registered in module " << sModuleName << ": " << key << std::endl;
			return false;
		}
	}
	else*/
	{
		bRet = AddStub(atAddr, refMode, value);
	}

	if (!bRet && phase == 1)
	{
		PrintError() << "Function profile in script already exist: " << key << std::endl;
		return false;
	}

	return true;
}


ADDR StubInfo_t::nameToField(const char *name, MyString &sModuleName, const ImpLookupMap &lImp) const
{
#if(1)
	ImpLookupMap::const_iterator i(lImp.find(name));
	if (i == lImp.end())
		return 0;
	TypePtr iModule(ModuleOf(i->second.second));
	sModuleName = ModuleTitle(iModule);// ->typeModule()->title();
	return i->second.first;

#else
/*	FieldPtr pField(FindGlobalByName(name));
	if (pField && pField->isTypeImp())
	{
		STOP
	}*/

	//const Dc_t &dc(mrDcRef);
	Module_t &aBin(mrDC.binary());
	OFF_t oModule(OFF_NULL);
	ADDR a(mrDC.frontEnd()->getImportPtrByName(name, &oModule));
	assert(!IsNull(oModule));
	FetchStringAt(oModule, sModuleName);
	return a;
#endif
}

/*const char *DcInfo_t::fieldToName(ADDR va, OFF_t *ppModuleName) const
{
	//const Dc_t &dc(mrDcRef);
	Module_t &aBin(mrDC.binary());
	//const Seg_t &rSeg(*dc.primeSeg()->typeSeg());
	SymbolInfo imp;
	if (!mrDC.frontEnd()->getImportInfo(va, imp))
		return nullptr;
	*ppModuleName = imp.pModuleName;
	return imp.pSymbolName;
}*/

/*bool StubInfo_t::DumpStubImported(const Stub_t &aStub, std::ostream &os, std::string &module, const ImpLookupMap &lImp) const
{
	assert(0);
	FieldPtr pField(0);//aStub.field());
	assert(pField && !pField->nameless());
	MyString module0;
	if (!nameToField(pField->name()->c_str(), module0, lImp))
		return false;
	assert(!module0.empty());

//#ifdef WIN32
	//MyString module2(module0.lower());
//#else
	MyString module2(module0);
//#endif

	//const char *pFieldName(fieldToName(a.atAddr(), &pModuleName));
	//assert(pFieldName);//imported by ordinal???
	if (module2 != module)
	{
		module = module2;
		os << "[" << module0 << "]" << std::endl;
	}
	os << pField->name()->c_str();
	MyString s(ValueFromStub(aStub));
	os << " = " << s;
	//os << std::endl;
	return true;
}*/

bool StubInfo_t::dumpz(const Stub_t &rStub, std::ostream &os/*, MyString &module*/) const
{
//CHECK(rStub.mAtAddr == 0x1001094)
//STOP

	//dump a stub of imported function only if it doesn't match the one provided with the front end.
	/*if (rStub.field() && rStub.field()->IsTypeImp())
	{
		if (rStub.isMismatched() || rStub.isModified())
			return DumpStubImported(rStub, os, module);
		return false;
	}
	else if (!rStub.field())//it still can be imported
	{
		Locus_t loc;
		DcInfo_t dcInfo(mrDcRef);
		FieldPtr pField(dcInfo.FindFieldInSubsegs(dcInfo.dc().primeSeg(), rStub.atAddr(), loc));
		if (pField && pField->IsTypeImp())
		{
			if (rStub.isModified())
				return DumpStubImported(rStub, os, module);
			return false;
		}
	}*/

	//os << name(dc); //don't store a name, otherwise expect stubs file to have diffs
	os << "@";
	os << rStub.atAsString(ImageBase());
	MyString s(ValueFromStub(rStub));
	os << " = " << s;
	return true;
}

bool StubInfo_t::DisconnectStub(ADDR atAddr)
{
	StubMgr_t &stubs(dc().stubs());
	return stubs.disconnectStub(atAddr);
}

/*int StubInfo_t::FromStub(const Stub_t *pStub, ProbeEx_t &ctx)
{
	WriteLocker lock;

	FuncProfile_t fp;
	if (!ToFuncProfile(pStub->value(), fp))
		return 0;

	FieldPtr pField(FindFieldRef(pStub->atAddr(), pStub->refMode(), ctx.scope()));

	//StubBase_t stb(pField, refMode, atAddr);

	if (!pField)
	{
		GlobPtr iFunc(ctx.scopeFunc());
		if (!iFunc)
			return 0;
		FuncInfo_t FI(*this, *iFunc);
	}
	else
	{
		GlobPtr ifDef(Func DefAttached(pField));
		if (ifDef)
		{
			FolderPtr pFolder(FolderOf(ifDef));

			FuncInfo_t FI(mrDC, *ifDef, *pFolder->fileDef());

			if (!IsStub(ifDef))
				FI.InitiateRedecompile();

			int iRet(FI.FromFuncProfile(fp));
			if (iRet)
			{
				//redump current container - shuld be at supplied ctx
				Dc_t& rDC0(*ctx.dcRef());
				FileDef_t& rFileDef0(*ctx.fileDef());
				FileInfo_t fileInfo(rDC0, rFileDef0);
				if (iRet == 2)//class membership changed - only header needs to be redumped, the rest - just redraw
				{
					//ProbeEx_t ctx;
					ctx.setFolder(pFolder);
					fileInfo.redump(ctx, REDUMP_H);
				}
				else
					fileInfo.redump(ctx, REDUMP_SRC);
				//guix().GuiOnExprModified();
			}
			//if (!pStub->field())
			{
				//pStub->setField2(pField);
//					pStub->setModified(true);
			}
			return iRet;

			/ *else
			{
				FuncInfo_t FDI(mrDC, *ifDef);
				FuncProfile_t fp2;
				FDI.GetFuncProfile(fp2);
				if (fp2 == fp)
					str = "";
				pStub->setValue(str, false);
				return 1;
			}* /

			//			PrintError() << "Can't modify a stub of decompiled function (" << pField->namexx().c_str() << ")" << std::endl;
			//		return 0;//no point in correcting profiles of precessed functions (except ret values?)
		}
		else
		{
			GlobPtr ifDef(IsPtrToCFunc(pField));
			if (ifDef)
			{
				FuncInfo_t FDI(mrDC, *ifDef);
				FuncProfile_t fp2;
				FDI.GetFuncProfile(fp2);
				/ *pStub->setValue2(str);
				if (fp2 == fp)
					str = "";
				else
					pStub->setModified(true);* /
				return 1;

				//DCI.FuncDefToStubStr(*pfDef);
			}
		}
	}
	/ *else if (pStub->refMode() == REFMODE_GLOBAL_PTR)
	{
	Locus_t loc;
	pField = FindFieldInSubsegs(PrimeSeg(), pStub->atAddr(), loc);
	if (pField && IsTypeImp(pField))
	{
	if (pStub->value() != MyString(str))
	pStub->setModified();
	}
	}* /
	//pStub->setValue2(str);
	//pStub->setModified(true);
	return 1;
}*/

MyString StubInfo_t::FuncDefToStubStr(GlobPtr iSelf) const
{
	FuncProfile_t fp;
	ProtoInfo_t TI(*this, iSelf);// , FileDef_t());
	TI.GetFuncProfile(fp);
	std::ostringstream ss;
	dump_trimmed(fp, ss);
	return ss.str();
}


bool StubInfo_t::CreateFuncProfile(const Stub_t *pStub, FuncProfile_t &si)
{
	assert(pStub);
	if (!ToFuncProfile(pStub->value(), si))
		return false;

#if(0)//pop up at every call location (for testing only)
	if (Project().analyzer())//may not be yet started
	{
		if (stb.field())
			throw StubFault_t(stb.field());
	}
#endif

//	FromFuncProfile(si);
	return true;
}

void StubInfo_t::StubFault(CFieldPtr p) const
{
	throw StubFault_t((FieldPtr)p);
}

const Stub_t *StubInfo_t::FindStubOrThrow(CFieldPtr pField, ADDR curAddr) const
{
	StubBase_t stb(pField, curAddr);
	Stub_t *pStub(FindStub(stb.atAddr()));
	if (!pStub)
	{
		/*if (!pBuf)//no stub supplied
			if (!bUserInput)
				return false;//currently proccessed func!*/

//?		gui().GuiShowFile("?STUBS", "$task", false);//open stubs view at task's top
		StubFault(stb.field());//throwing a reference doesn't work
	}
/*?	if (pStub->refMode() != stb.refMode())
		pStub->setRefMode(stb.refMode());*/

	return pStub;
}

void DcInfo_t::PostMakeStub(MyString key, MyString value, CTypePtr iModule, I_Context *pCtx)
{
	assert(iModule->typeModule());
	MyString sCmd("mkstub");
	if (ModulePtr() != iModule)
		sCmd.append(MyStringf(" -module %s", ModuleTitle(iModule).c_str()));
	sCmd.append(MyStringf(" @%s %s", key.c_str(), value.c_str()));
	adc::CEventCommand *pCmd(new adc::CEventCommand(pCtx, sCmd));
	//context will be established from locus
/*	ProbeEx_t *pCtx(new ProbeEx_t);
	pCtx->add(ModulePtr(), 0, nullptr);//provide DC
	//pCtx->setFolder(pFolder);
	//pCtx->setDcRef(DcPtr());*/
	mrMain.postEvent(pCmd);
}

//args
MyString StubInfo_t::ToString1(const GPRs_t& v) const
{
	MyString s;
	size_t nBraced(v.empty() ? 0 : (v[0].isValid() ? v.size() : v.size() - 1));
	if (nBraced != 1)
		s += '(';

	//args
	int count(0);
	char buf[80];
	for (size_t i(0); i < v.size(); i++)
	{
		const REG_t &r(v[i]);
		if (count > 0)
			s += ',';
		const char *str(mrDC.toRegName(SSID_CPUREG, r.m_ofs, r.m_siz, 0, buf));
		if (!str)
		{
			if (i > 0)
			{
				s += '?';
				count++;
			}
		}
		else
		{
			s.append(str);
			if (i == 0)
				s += '^';//thisptr
			count++;
		}
	}
	if (nBraced != 1)
		s += ')';
	return s;
}

//rets
MyString StubInfo_t::ToString(const GPRs_t& v, const GPRs_t& w) const
{
	MyString s;
	bool bBraced(v.size() + w.size() != 1);
	if (bBraced)
		s += '{';

	//retvals
	char buf[80];
	for (size_t i(0); i < v.size(); i++)
	{
		const REG_t &r(v[i]);
		if (i > 0)
			s += ',';
		const char *str(mrDC.toRegName(SSID_CPUREG, r.m_ofs, r.m_siz, 0, buf));
		if (!str)
		{
			s += '?';
		}
		else
		{
			s.append(str);
			s += '!';//retval!
		}
	}

	//spoiled
	for (size_t i(0); i < w.size(); i++)
	{
		const REG_t &r(w[i]);
		if (i > 0 || !v.empty())
			s += ',';
		const char *str(mrDC.toRegName(SSID_CPUREG, r.m_ofs, r.m_siz, 0, buf));
		if (!str)
			str = "?";
		s.append(str);
	}

	if (bBraced)
		s += '}';
	return s;
}

/*void StubInfo_t::dump(const FuncProfile_t &fp, std::ostream &os, char sep)
{
	os << fp.stackin << sep << fp.stackout << sep;
	int fpuin2(fp.fpuin >> 3);
	int fpuout2(fp.fpuout >> 3);
	os << fpuin2 << sep << fpuout2 << sep;
	os << fp.cpuin.toString() << sep;
	os << std::hex;
	os << std::setfill('0');
	os << std::setw(8) << fp.savedregs << sep;
	os << std::setw(2) << fp.flags << sep;
	os << std::setw(2) << fp.savedflags << sep;
	os << std::dec;
	os << ToString(fp.cpuout);
}*/

//1:stackin stackout, 3:fpuin, fpuout, 5:cpuin, 6:spoiled/rets, 7:flags, 8:spoiledFlags
void StubInfo_t::dump_trimmed(const FuncProfile_t &fp, std::ostream &os, char sep) const
{
	bool B[8];
	//B[7] = (fp.spoiledFlags);
	B[6] = (fp._flags != 0);// || B[7];
	B[5] = (!fp.spoiltRegs.empty() || !fp.cpuout.empty()) || B[6];
	B[4] = (!fp.cpuin.empty()) || B[5];
	B[3] = (fp.fpudiff != 0) || B[4];
	B[2] = (fp.fpuin != 0) || B[3];
	B[1] = (fp.pstackPurge != 0) || B[2];
	B[0] = (fp.stackin != 0) || B[1];

	os << fp.stackin;
	if (B[1])
		os << sep << fp.pstackPurge;
	if (B[2])
		os << sep << (fp.fpuin / FTOP_STEP);
	if (B[3])
		os << sep << (fp.fpudiff / FTOP_STEP);
	if (B[4])
		os << sep << std::hex << ToString1(fp.cpuin) << std::dec;
	if (B[5])
		os << sep << ToString(fp.cpuout, fp.spoiltRegs);
	if (B[6])
		os << sep << std::hex << fp._flags << std::dec;
	//if (B[7])
		//os << sep << std::hex << fp.spoiledFlags << std::dec;
}









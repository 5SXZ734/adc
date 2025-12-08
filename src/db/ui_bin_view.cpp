#include "ui_bin_view.h"


#include "main.h"
#include "dump_visit.h"
#include "proj.h"
#include "names.h"
#include "ui_main.h"
#include "debug.h"
#include "info_module.h"
#include "shared/dump_util.h"
#include "qx/ConvertUTF.h"
#include "qx/MyStringList.h"


class FakeScope_t : public Module_t
{
	TypeObj_t	self;
	Field_t		field;
	NameRef_t	name;
	ADDR		mBaseVA;
	DataSubSource_t	mData;
	static DataSourceNull_t dataNull;
	//DataObj_t	mDataObj;
public:
	FakeScope_t(CTypeBasePtr p0)
		: mBaseVA(0),
		mData(dataNull)
	{
		assert(!p0->typeProxy());
		CTypeBasePtr p(/*ProjectInfo_t::SkipProxy*/(p0));
		self.SetPvt(this);
		field.setType0((TypePtr)p);
		field.setOwnerComplex((TypePtr)&self);
		field.flags() |= FLD_TEMP;//so the name could not be edited
		if (p->typeUnion())
			name.overrideKey((char*)"union");
		else if (p->isEnum())
			name.overrideKey((char*)"enum");
		else if (p->typeStruc())
			name.overrideKey((char *)p->printType());//"struct");
		else
			name.overrideKey((char*)"type");
		field.setName0(&name);
		fields().insert_unique(&field);
		updateView(0, OFF_NULL);

		//mDataObj.setPvt(&dataNull);
		//setDataSource(&mDataObj);
	}
	~FakeScope_t()
	{
		setDataSource(nullptr);
		field.setType0(nullptr);
		name.overrideKey(nullptr);
		field.setName0(nullptr);
		FieldMapIt i(fields().begin());
		fields().take(i);
		self.SetPvt0(nullptr);
	}
	TypePtr typeObj(){ return &self; }
/*	virtual const I_DataSourceBase &dataSourceRef() const {
		return dataSrc;
	}*/
	TypePtr target() const {
		return field.type();
	}
	virtual ADDR base(CTypeBasePtr) const {
		return mBaseVA;
	}
	/*virtual const I_DataSourceBase& dataSourceRef() const {
		return mData;
	}*/
	void update(CTypePtr iSeg, ADDR va, DataPtr pData)//if application site has changed
	{
		Block_t aRaw(iSeg->typeSeg()->rawBlock());
		aRaw.subspace(va - iSeg->base(), -1);//to the upper limit
		//rawBlock() = aRaw;
		ROWID ra(aRaw.lower());
		if (aRaw.empty())
		{
			pData = nullptr;
			ra = OFF_NULL;
			rawBlock().set(0, 0);
		}
		else
			rawBlock().set(0, pData->size());
		
		setDataSource(pData);
		/*if (pData)
			mData = DataSubSource_t(pData->pvt(), aRaw);
		else
			mData = DataSubSource_t(dataNull);*/

		updateView(va, ra);
		auto it = fields().begin();
		fields().take(it);
		field.overrideKey(va);
		fields().insert_unique(&field);
	}

	void updateView(ADDR va, OFF_t ra)
	{
		ROWID vsz(/*(ADDR)*/target()->size());
		if (vsz == 0)// || vsz == OFF_NULL)
			vsz = OPSZ_BYTE;

		setView(0, vsz);

		mSections.reset();
		mSections.add(0, (size_t)vsz, ra, 0, typeObj());
		mBaseVA = va;
	}
};

DataSourceNull_t FakeScope_t::dataNull;

BinViewModel_t::BinViewModel_t(Core_t &rCore, Main_t &rMain, CTypePtr iModule, CTypeBasePtr iScope, bool bStrucDump)
	: mrCore(rCore),
	mrMain(rMain),
	miModule(iModule),
	//mpFolder(miModule ? miModule->typeModule()->folderPtr() : nullptr),
	mpFakeModule(bStrucDump ? new FakeScope_t(iScope) : nullptr),
	mpScopeRoot(mpFakeModule ? mpFakeModule->typeObj() : iScope),
	mpScope(nullptr),
	mDrawObj(mpScopeRoot, rCore, bStrucDump ? nullptr : miModule),
	//mbSyncMode(false),
	miLocked(0)
{
	//mrCore.aquireDumpTarget();
	if (iModule && ProjectInfo_t::IsPhantomModule(iModule))
		mDrawObj.setCompactMode(true);
}

BinViewModel_t::~BinViewModel_t()
{
//?	if (probe() && probe()->owner() == this)
//?		mrCore.setProbe(nullptr);
	delete mpFakeModule;
	//mrCore.releaseDumpTarget();//can't be called in destructor
}

const char *BinViewModel_t::moduleName() const
{
	if (!miModule)
		return ATTIC_NAME;
	return ProjectInfo_t::ModuleTitle(miModule).c_str();
}

/*void BinViewModel_t::reset(Project_t *pProject)
{
WriteLocker lock(this);
mDrawObj.init(pProject);
}*/

/*bool BinViewModel_t::seekAddrIt(adcui::ITER it, ADDR addr)
{
const Seg_t *pSeg(dc()->primeSeg()->find SegAt(addr));
if (!pSeg)
return false;
ROWID r(pSeg->VA 2DA(addr));
return mDrawObj.SeekIt(it, r, 0);
}*/
//int infoIt(){ return mInfoIt; }
/*void clear()
{
reset(nullptr);
}*/

adcui::DUMPOS BinViewModel_t::newPosition()
{
	//assert(miLocked);
	return mDrawObj.NewPosition();
}

void BinViewModel_t::deletePosition(adcui::DUMPOS iPos)
{
	//assert(miLocked);
	mDrawObj.DeletePosition(iPos);
}

adcui::DUMPOS BinViewModel_t::posFromIter(adcui::ITER it)
{
	//assert(miLocked);
	return mDrawObj.PosFromIter(it);
}

adcui::ITER BinViewModel_t::newIterator(adcui::DUMPOS iPos)
{
	//		assert(miLocked);
	DumpVisitor_t binDumper(proj(), mDrawObj);
	return binDumper.NewIterator1(iPos);
}

void BinViewModel_t::deleteIterator(adcui::ITER it, bool bUpdatePos)
{
	//		assert(miLocked);
	mDrawObj.DeleteIterator(it, bUpdatePos);
}

void BinViewModel_t::lockRead(bool bLock)
{
	miLocked += mrMain.lockProjectRead(bLock);//can be recursive
}

void BinViewModel_t::lockWrite(bool bLock)
{
	miLocked += mrMain.lockProjectWrite(bLock);
}

void BinViewModel_t::seekLineIt(adcui::DUMPOS it, int line)
{
	assert(miLocked);//ReadLocker lock(this);
	DumpVisitor_t binDumper(proj(), mDrawObj);
	ROWID da;
	if (line < 0)
	{
		assert(0);
		da = ROWID_END;
	}
	else if (line == 0)
		da = mDrawObj.scopeLowerBound();
	else if (line >= linesNum() - 1)
		da = mDrawObj.scopeLowerBound() + viewExtent() - 1;
	else
	{
		ROWID span(viewExtent());//mrMain.getStartAddr();
		da = mDrawObj.scopeLowerBound() + (ROWID)(((double)line / linesNum()) * span);
	}
	binDumper.SeekIt(it, DA_t(da, 0, 0), nullptr);
}

int BinViewModel_t::lineFromIt(adcui::DUMPOS it)
{
	assert(miLocked);
	ROWID sz(viewExtent());
	int pos = 0;
	if (sz > 0)
	{
		ROWID r(toDA(it).row);
		if (r > mDrawObj.scopeLowerBound())
			r -= mDrawObj.scopeLowerBound();
		else
			r = 0;
		if (r < sz)
			pos = (int)(((double)r / sz) * linesNum());
		else
			pos = linesNum();
	}
	return pos;
}

void BinViewModel_t::copyIt(adcui::DUMPOS itTo, adcui::DUMPOS itFrom)
{
	assert(miLocked);
	DumpVisitor_t binDumper(proj(), mDrawObj);
	binDumper.SeekIt(itTo, mDrawObj.toDA(itFrom), nullptr);
}

void BinViewModel_t::setBitPosition(adcui::DUMPOS itPos)
{
	/*adcui::AutoIter t(this, itPos, false);
	const char *pc(mDrawObj.CellDataIt(mDrawObj.PosFromIter(t), CLMN_DA));
	const char *pdot(strchr(pc, '.'));
	if (pdot)
	{
	int nBit(atoi(++pdot));
	mDrawObj.SetBitPos(itPos, nBit);
	}*/
}

bool BinViewModel_t::backwardIt(adcui::ITER it)
{
	//assert(miLocked);
	DumpVisitor_t binDumper(proj(), mDrawObj);
	return binDumper.BackwardIt(it);
}

bool BinViewModel_t::forwardIt(adcui::ITER it)
{
	//assert(miLocked);
	DumpVisitor_t binDumper(proj(), mDrawObj);
	return binDumper.ForwardIt(it);
}

DA_t BinViewModel_t::toDA(adcui::DUMPOS it)
{
	return mDrawObj.toDA(it);
}

int BinViewModel_t::checkEqual(adcui::DUMPOS it1, adcui::DUMPOS it2)//returns like strcmp
{
	//assert(miLocked);
	DA_t a(toDA(it1));
	DA_t b(toDA(it2));
	if (a < b)
		return -1;
	if (b < a)
		return 1;
	return 0;
}

bool BinViewModel_t::atEndIt(adcui::DUMPOS it)
{
	//assert(miLocked);
	return mDrawObj.IsAtEndIt(it);
}

int BinViewModel_t::lineIt(adcui::DUMPOS it)
{
	//assert(miLocked);
	return mDrawObj.LineIt(it);
}

/*int levelIt(adcui::DUMPOS it)
{
return mDrawObj.LevelIt(it);
}*/

const char * BinViewModel_t::cellDataIt(COLID iCol, adcui::DUMPOS iPos, bool bPlain)
{
	assert(miLocked);//ReadLocker lock(this);

	Probe_t *pCtx(mrCore.getContext());
	if (pCtx)
		pCtx->Release();

	DumpVisitor_t binDumper(proj(), mDrawObj);
	ms = binDumper.CellDataItEx(iPos, iCol, nullptr, nullptr);//probe()
	return ms.c_str();
}

static const char *strtag(const char *s)
{
	while (*s && (unsigned char)*s < 0x80)
		s++;
	return *s ? s : nullptr;
}

#include <codecvt>

void BinViewModel_t::getRowDataIt(adcui::DUMPOS iPos, adcui::IADCTableRow &aTab)
{
	assert(miLocked);//ReadLocker lock(this);
	DumpVisitor_t binDumper(proj(), mDrawObj);
	bool bEditing(mrCore.isEditing());// probe() && probe()->isEditing());

	Probe_t *pCtx(mrCore.getContext());
	if (pCtx)
		pCtx->Release();

	for (COLID iCol(0); (int)iCol < colsNum(); iCol++)
	{
		unsigned flags(colFlags(iCol));
		if (mDrawObj.IsTreeColumn(iCol))
			flags |= adcui::CLMNFLAGS_TREE;

		aTab.setCellFlags(iCol, flags);
		int w(columnWidth(iCol));
		aTab.setCellWidth(iCol, w);

		if (iCol == adcui::IBinViewModel::CLMN_TREEINFO)
		{
			//ms = binDumper.CellDataItEx(iPos, iCol, &mProbe);//pure ascii - colored info lost!
			ms = mDrawObj.CellTreeDataIt(iPos);
			aTab.addTree(iCol, ms.c_str());
			continue;
		}

		if (w <= 0)
		{
			assert(iCol != adcui::IBinViewModel::CLMN_TREEINFO);
			continue;
		}

		ms = binDumper.CellDataItEx(iPos, iCol, pCtx, mrCore.ed());//probe()
CHECK(iCol == CLMN_DA && ms.empty())
STOP
		const char *cellStr(ms.c_str());
			//for register names on margin see: drawFieldExtra(!)

		adcui::Color_t fgnd_color(adcui::COLOR_NULL);
		if (!bEditing && /*m_bSyncMode && */iCol == adcui::IBinViewModel::CLMN_CODE)
		{
			int iSel(checkSelIt(iPos));
			if (iSel == 1)//primary selection
				fgnd_color = adcui::COLOR_DASM_SEL;
			else if (iSel == 2)//secondary selection
				fgnd_color = adcui::COLOR_DASM_SEL_AUX;
		}

		if (fgnd_color != adcui::COLOR_NULL)
			aTab.addColor(iCol, fgnd_color);

		adcui::Color_t iNextColor(adcui::COLOR_NULL);
		while (cellStr && *cellStr)
		{
			std::string s;

			const char *ps;
			if (iNextColor == adcui::COLOR_WSTRING)
			{
				iNextColor = adcui::COLOR_NULL;

				ps = cellStr;
				uint16_t ulen(*(uint16_t *)ps);
				ps += sizeof(ulen);

				const char16_t *wp_from((char16_t *)ps);
				ps += ulen * sizeof(char16_t);
				const char16_t *wp_to((char16_t *)ps);

				std::u16string ws;
				for (const char16_t *wp(wp_from); wp < wp_to; wp++)
				{
					switch (*wp)
					{
					case u'\n': ws += u"\\n"; break;
					case u'\r': ws += u"\\r"; break;
					case u'\t':	ws += u"\\t"; break;
					default:
						ws += *wp;	break;
					}
				}

				const char16_t *sourceStart(ws.c_str());
				const char16_t *sourceEnd(sourceStart + ulen);

				//std::wstring_convert<std::codecvt_utf8_utf16<char16_t>> converter;

				s.resize(ulen * 2);//allow a safety buf
				UTF8 *targetStart((UTF8 *)&(*s.begin()));
				UTF8 *targetEnd(targetStart + s.length());
				ConvertUTF16toUTF8((const UTF16 **)&sourceStart, (const UTF16 *)sourceEnd, &targetStart, targetEnd, strictConversion);
				if (!fgnd_color)
					iNextColor = FROMCOLORID(*ps);//revert
				ps++;
			}
			else
			{
				iNextColor = adcui::COLOR_NULL;

				ps = strtag(cellStr);

				if (ps)
				{
					s = std::string(cellStr, ps - cellStr);
					if (!fgnd_color)
						iNextColor = FROMCOLORID(*ps);//revert
					ps++;
				}
				else
					s = cellStr;
			}

			cellStr = ps;

			if (s.length() > 0)
				aTab.addCell(iCol, s.c_str());

			if (iNextColor != adcui::COLOR_NULL)
				aTab.addColor(iCol, iNextColor);
		}
	}

	ms = binDumper.CellDataItEx(iPos, adcui::IBinViewModel::CLMN_DA, pCtx, mrCore.ed());//probe()
	if (ms.endsWith("!"))
		aTab.setMarginColor(adcui::COLOR_MARGIN_ERROR);

	if (checkTaskTop(iPos))
		aTab.setLineColor(adcui::COLOR_TASK_TOP);
}

void BinViewModel_t::getColumnInfo(adcui::IADCTableRow &aRow, bool bNoData)
{
	for (int col(0); col < colsNum(); col++)
	{
		int w(columnWidth(col));
		if (w > 0)
		{
			if (!bNoData)
				aRow.addCell(col, colName(col));
			//unsigned flags(colFlags(col));
			//if (mDrawObj.IsTreeColumn(col))
			//flags |= adcui::CLMNFLAGS_TREE;
			aRow.setCellFlags(col, colFlags(col));
			aRow.setCellWidth(col, w);
		}
	}
}

int BinViewModel_t::checkSelIt(adcui::DUMPOS it)
{
	assert(miLocked);//ReadLocker lock(this);
	CTypePtr iModule(module());
	if (iModule)//attic types have no module
	{
		int q(mrCore.checkSelection(iModule, mDrawObj.toDA(it)));
		if (q == 1)
			return 1;
		if (q == 2)
			return 2;
	}
	return 0;
}

const char *BinViewModel_t::cellTreeDataIt(COLID col, adcui::DUMPOS it)
{
	assert(miLocked);//ReadLocker lock(this);
	return mDrawObj.CellTreeDataIt(it, col);
}

void BinViewModel_t::invalidate(bool)
{
	//	mDrawObj.Invalidate();
}

int BinViewModel_t::columnWidth(COLID colID)
{
	return mDrawObj.columnWidth(colID);
}

void BinViewModel_t::setColumnWidth(COLID colID, int w)
{
	if (mDrawObj.setColumnWidth(colID, w))
		mrCore.refreshBinaryDump();//dump contents may change when toggling TYPES column
}

////////////////////////////////////////////////////////////////////////////// setCurPosIt()
void BinViewModel_t::setCurPosIt(adcui::DUMPOS iPos, int x)
{
	//assert(!miLocked);//ReadLocker lock(this);
	//WriteLocker lock;
	lockWrite(true);
	{//RAII

		DumpVisitor_t binDumper(proj(), mDrawObj);

		ProbeIn_t aProbe(x);

		binDumper.Probe(aProbe, iPos);

		adcui::AutoIter t(this, iPos, false);

		DumpVisitor_t DV(proj(), mDrawObj);

		FieldPtr pField(DV.fieldAt(mDrawObj.PosFromIter(t)));

		/*const char *pc(mDrawObj.CellDataIt(mDrawObj.PosFromIter(t), CLMN_DA));
		if (pc)
		{
		const char *pdot(strchr(pc, '.'));
		if (pdot)
		{
		int nBit(atoi(++pdot));
		mDrawObj.SetBitPos(iPos, nBit);
		}
		}*/

		Locus_t& aLoc(aProbe.locus());
		assert(aLoc.empty());

		if (DV.DaToLocus(aProbe.da(), aLoc, scope()))
		{
			if (pField)
			{
				if (!aLoc.stripToField(pField))//handle a starting address of a hierarchy
				{
					if (aLoc.field0() && aLoc.field0()->_key() == pField->_key())
						aLoc.setField(pField);//select a correct U-field

#if(0)//wtf? - mig(~140)
					//CHECK(pProbe->field0())
					//STOP
					if (aLoc.field0())//?
					{
						lockWrite(false);
						return;
					}
					aLoc.setField(pField);//a click on a line near the field
#endif
				}
			}
			//aProbe.setFolder(mpFolder);
			Locality_t locality(adcui::CXTID_BINARY, 0);
			if (aProbe.obj())
				locality.obj = 1;
			aProbe.setLocality(locality);
			aProbe.setModule(aLoc.module2());
			OnSetCurPos(aProbe);
		}
	}

	lockWrite(false);
}

bool BinViewModel_t::locusInfo(MyStreamBase &ss, int &x)
{
	assert(miLocked);
	MyStreamUtil ssu(ss);
	Probe_t *pCtx(mrCore.getContext());
	if (!pCtx)
		return false;
	//assert(probe());
	x = pCtx->x();// probe()->x();
	mrCore.releaseContext(pCtx);
	ssu.WriteString(pCtx->locus().toStringSeg(proj()));
	return true;
}

void BinViewModel_t::updateSelection()
{
	/*ReadLocker lock(this);

	Dc_t *pDC(proj()->dc());
	if (pDC && pDC->contextOp())
	{
	FuncDef_t *pfDef(pDC->contextFunc()->fun cdef());
	FuncInfo_t f0(*pDC, *pfDef, pDC->FindFileDefOf(*pfDef));
	mDrawObj.updateVAlist(f0, pDC->contextOp());
	}
	else
	{
	mDrawObj.clearVAlist();
	}*/
}

// <empty>	: entry point
// ~XXXX	: rowid
// .XXXX	: file offset
// $task	: task top
// $sync	: pane sync
// $dbg		: debugger next
// XXXX		: address
// <name>	: object name
bool BinViewModel_t::seekPosIt(const char *pc, adcui::DUMPOS it, adcui::DUMPOS itRef)
{
	assert(miLocked);//ReadLocker lock(this);

	//if (!binaryRef())
	//return false;

	CTypePtr iModule(module());
	if (!iModule)
		return false;

	ModuleInfo_t MI(proj(), *(TypePtr)iModule);

	DumpVisitor_t binDumper(proj(), mDrawObj);
	CTypePtr iFrontSeg(nullptr);

	MyString s;
	if (pc)
		s = MyString(pc).simplifyWhiteSpace();

	if (s.isEmpty())
	{
		ADDR a;
		TypePtr iSeg(MI.FindEntryPoint(a));
		if (!iSeg)
			return false;
		ROWID r(binDumper.VA2DA(iSeg, a));

		//?r += mDrawObj.scopeLowerBound();
		return binDumper.SeekIt(it, DA_t(r, 0, 0), nullptr);
	}

	DA2_t da;
	if (s.startsWith("+"))
	{
		ContextSafe_t<> safe(mrCore);
		if (safe.empty())
			return false;
		s.remove(0, 1);
		da.row = safe.get()->locus().da();
		da.row += strtoull(s.c_str(), nullptr, 16);
		if (!binDumper.SeekIt(it, da, nullptr))
			return false;
		skipEmptyLines(binDumper, it);
		return true;
	}

	if (s.startsWith("~"))//Display Address (DA)
	{
		s.remove(0, 1);
		da.row = strtoul(s.c_str(), nullptr, 16);
	}
	else
	{
		value_t eva;
		if (s.startsWith("."))//Raw Address (RA)
		{//real address
			s.remove(0, 1);
			eva.ui64 = strtoul(s.c_str(), nullptr, 16);
			//da.row = eva.ui64;
			if (!MI.R2D(iModule, (ADDR)eva.ui64, da.row))
				//if (!proj()->ADDR2ROWID(eva, 0, r))
				return false;
		}
		else
		{
			//Virtual Address (VA)

			// special
			if (s == "$sync" || s == "$syncobj" || s == "$syncva")// a responce to pane sync event
			{
				ContextSafe_t<> safe(mrCore);
				if (safe.empty())
					return false;

				/*ADDR va(pCtx->va());
				TypePtr iSeg(pCtx->seg());
				if (iSeg)
				eva.ui64 = iSeg->imageBase() + va;*/
				Probe_t *pProbe(safe.get());
				if (pProbe->module() != iModule)
					if (pProbe->moduleFromLocus() != iModule)
						return false;

				da.row = pProbe->locus().da();
				if (da.row == ROWID_INVALID)
				{
					da.row = pProbe->daFromFieldDecl();
					if (da.row == ROWID_INVALID)
						return false;
				}
				if (s == "$syncobj")
				{
					if (pProbe->obj())
					{
						FieldPtr pField(pProbe->obj()->objField());
						if (pField)
						{
							if (ProjectInfo_t::OwnerSeg(pField->owner()))
								da.row = ProjectInfo_t::VA2DA(pField->owner(), pField->_key());
						}
					}
					else// if (probe())
					{
						//if (!aProbe.empty())
						{
							TypePtr iSeg(pProbe->locus().seg());//from seg
							if (iSeg)
							{
								ADDR va(pProbe->value().ui32);

								Locus_t aLoc2;//target location
								TypePtr iSeg2;
								if ((iSeg2 = MI.VA2Locus(iSeg, va, aLoc2, false)) != nullptr)
								//iSeg = MI.FindSegAt(iSeg, va, iSeg->typeSeg()->affinity());//target seg
								if (iSeg2)
									da.row = ProjectInfo_t::VA2DA(iSeg2, va);
							}
						}
					}
				}

				FieldPtr pField(pProbe->pickedFieldDecl());//field()
				if (!pField && da.row == 0)
					//if (pProbe->addr() == 0)
					return false;//da.line = 1;
				if (!pField)
					pField = pProbe->locus().field0();
				if (!binDumper.SeekIt(it, da, pField))
					return false;
				//if (pField)
				//binDumper.SetAtField(pField);
				skipEmptyLines(binDumper, it);
				return true;
			}
			else if (s == "$task")// a responce to pane sync event
			{
				IAnalyzer *pIAnlz(proj().analyzer());
				if (pIAnlz)
					eva.ui64 = pIAnlz->currentVA();
			}
			else if (s == "$dbg")
			{
				Debugger_t *pDbg(proj().debugger());
				if (pDbg)
					eva.ui64 = pDbg->current();
			}
			else if (s == "$probe")
			{
				ContextSafe_t<> safe(mrCore);
				if (safe.empty())
					return false;

				Probe_t *pProbe(safe.get());
				//if (pProbe)
				if (pProbe)
				{
					CObjPtr pObj(pProbe->obj());// pProbe->obj());
					if (pObj)
					{
						CFieldPtr pField(pObj->objField());
						if (pField)
						{
							TypePtr iSeg(MI.OwnerSeg(pField->owner()));
							if (!iSeg)
								return false;
							eva.ui64 = iSeg->typeSeg()->imageBase(iSeg) + pField->_key();
						}
					}
					else
					{
						adcui::Color_t iColor(pProbe->entityId());//pProbe
						if (iColor == adcui::COLOR_DASM_ADDRESS)
						{
							eva.ui64 = pProbe->value().ui64;//pProbe
						}
						else
						{
							AttrIdEnum attr(ATTR_NULL);
							if (MISC_COLOR_CHECK_RANGE(iColor))
								attr = MISC_COLOR_TO_ATTR(iColor);
							if (!attr)
								return false;
							if (iColor == adcui::COLOR_DASM_NUMBER)
								eva.ui64 = pProbe->value().ui64;//pProbe
							Locus_t l;
							if (!(itRef > size_t(0)) || !MI.DaToLocus(mDrawObj.toDA(itRef), l, module()))
								return false;
							if (!MI.DaFromAttrAtLocus(l, attr, pProbe->value(), da))//pProbe
							{
								if (da.pFolder && ModuleInfo_t::ModuleOf(da.pFolder) != iModule)//jump to another module
								{
									MyString s(FilesMgr0_t::path(da.pFolder));
									MAIN.postEvent(new adc::CEventCommand(MyStringf("show -ask -@%08X %s", (ADDR)da.row, s.c_str())));
									binDumper.SeekIt(it, mDrawObj.toDA(itRef), nullptr);//the current position must stay unchanged
									return true;
								}
								return false;
							}
							if (!binDumper.SeekIt(it, da, nullptr))
								return false;
							skipEmptyLines(binDumper, it);
							return true;
						}
					}
				}
			}
			else//address
			{
				if (proj().checkAutoPrefix(s.c_str()) == 0)
					eva.ui64 = strtoull(s.c_str(), nullptr, 16);
			}
			if (eva.ui64 == 0)
			{
				if (!itRef)// < 0)
					return false;
				Locus_t l;
				if (!MI.DaToLocus(mDrawObj.toDA(itRef), l, module()))
					return false;
				FieldPtr pField(MI.FindFieldByName(s, l));
				if (pField)
					eva.ui64 = pField->_key();
			}
			if (eva.ui64 == 0)
				return false;
			if (!iFrontSeg)
				iFrontSeg = MI.FindFrontSeg();
			if (!iFrontSeg)
				iFrontSeg = iModule;

			const Seg_t &rFrontSeg(*iFrontSeg->typeSeg());
			CTypePtr iSeg(rFrontSeg.findSubseg64(eva.ui64));
			if (!iSeg)
				iSeg = iModule;
			da.row = iSeg->typeSeg()->viewOffsAt(iSeg, (ADDR)(eva.ui64 - iSeg->typeSeg()->imageBase(iSeg)));
			/*Seg_t *pSeg(proj()->findS egAt(eva.ui64, da));
			if (!pSeg)
			return false;
			r = pSeg->VA 2DA(eva.ui64);*/
		}
	}

	if (!binDumper.SeekIt(it, da, nullptr))
		return false;

	if (s.startsWith("$"))
		skipEmptyLines(binDumper, it);

	return true;
}

void BinViewModel_t::skipEmptyLines(DumpVisitor_t &binDumper, adcui::DUMPOS it)
{
	adcui::AutoIter t(this, it, true);
	do {//position exactly at the line containing a call
		const char *pc(mDrawObj.CellDataIt(mDrawObj.PosFromIter(t), CLMN_NAMES));//CLMN_CODE));
		if (pc && pc[0])
			break;
		pc = mDrawObj.CellDataIt(mDrawObj.PosFromIter(t), CLMN_CODE);
		if (pc && pc[0])
			break;
	} while (binDumper.ForwardIt(t));
}

adcui::IADCTextEdit *BinViewModel_t::startEditIt(adcui::DUMPOS, int /*x*/)
{
	Probe_t* pProbe(mrCore.getContext());
	assert(pProbe);
	if (!pProbe->obj())
		if (pProbe->entityId() != adcui::COLOR_ARRAY)
			return nullptr;

	MyString s;
	if (pProbe->obj())
	{
		ProjectInfo_t PI(mrMain.project());
		CFieldPtr pField(pProbe->field());
		/*?if (module())
		{
		ModuleInfo_t MI(PI, *module());
		if (pField)
		s = MI.FieldDisplayName(pField);//FieldName
		}
		if (s.empty())*/
		{
			MyString s0;
			if (pProbe->typeObj())
				s0 = PI.TypeDisplayName(pProbe->typeObj());
			else
				s0 = mrMain.project().fieldDisplayName(pField);
			unsigned n(ProjectInfo_t::ChopName(s0, s));
			if (n > 0)
				s.append(NumberToString(n));
		}
	}
	else if (pProbe->entityId() == adcui::COLOR_ARRAY)
		s = Int2Str(pProbe->value().ui64);
	return mrCore.startEd(s);
}

/*bool BinViewModel_t::setSyncMode(bool bOn)
{
	//if (bOn == mbSyncMode)
	//return false;
	//mbSyncMode = bOn;
	//?		mDrawObj.clearVAlist();
	return true;
}*/

ROWID BinViewModel_t::viewExtent() const
{
	return mDrawObj.rowsNum();
}

/*Complex_t *scopeComplex()
{
Complex_t *pContCur(mDrawObj.sc ope());
if (!pContCur)
pContCur = proj();
return pContCur;
}*/

bool BinViewModel_t::listObjHierarchyAtIt(adcui::DUMPOS it, MyStreamBase &ss)
{
	assert(miLocked);//ReadLocker lock(this);

	DA_t da(mDrawObj.toDA(it));

	CTypePtr pScopeCur(mpScope);
	if (!pScopeCur)
		pScopeCur = module();

	ProjectInfo_t projInfo(proj());
	//if (pScopeCur->parentField())
	//	r += projInfo.VA2DA(pScopeCur, 0);//pScopeCur->parentField()->_key());

	CTypePtr iModule(module());
	if (!iModule)
		return false;

	Module_t &aBin(*iModule->typeModule());

	DA_t da2(da);

	Locus_t l;
	projInfo.terminalFieldAtSeg(iModule, da2, l, aBin.rawBlock());

	MyStreamUtil ssh(ss);
	for (Locus_t::reverse_iterator rit(l.rbegin()); rit != l.rend(); rit++)
	{
		const Frame_t &f(*rit);
		std::string s(projInfo.TypeName(f.cont()));
		for (Locus_t::reverse_iterator ritp(rit); ++ritp != l.rend();)//prepend the name with parent scopes
		{
			s.insert(0, "::");
			const Frame_t &fp(*ritp);
			std::string sp(projInfo.TypeName(fp.cont()));
			if (fp.cont()->typeModule())
				sp = ProjectInfo_t::ModuleTitle(fp.cont());
			s.insert(0, sp);
		}
		if (s.empty())//just a binary?
			s = ProjectInfo_t::ModuleTitle(iModule);
		if (f.cont() == pScopeCur)
			s.insert(0, "-->");//current scope
		ssh.WriteString(s);
	}
	return false;
}

void BinViewModel_t::scopeTo(const char *name)
{
	MyStringList l(MyStringList::split("::", MyString(name)));
	CTypePtr pCont(nullptr);//Complex_t
	ProjectInfo_t PI(proj());
	ROWID viewOffs(0);
	while (!l.empty())
	{
		MyString s(l.front());
		l.pop_front();
		if (pCont)
		{
			pCont = PI.findContByName(pCont, s.c_str(), viewOffs);
			if (!pCont || !pCont->typeStruc())
				break;
		}
		else
		{
			pCont = module();
		}
	}
	if (pCont)
	{
		if (mDrawObj.scopeTo(viewOffs, ProjectInfo_t::ViewSize(pCont)))
		{
			mrCore.refreshBinaryDump();
			mpScope = pCont;
		}
	}
}

TypePtr BinViewModel_t::realScope() const
{
	CTypeBasePtr pCont(mpScope);
	if (!pCont)
	{
		pCont = mpScopeRoot;
		if (!pCont)
		{
			pCont = module();
			if (!pCont)//attic?
				pCont = proj().self();
		}
	}
	if (pCont)
	{
		if (mpFakeModule && pCont == mpFakeModule->typeObj())
			pCont = mpFakeModule->target();
	}
	return (TypePtr)pCont;
}

void BinViewModel_t::scopeName(MyStreamBase& ss)
{
	CTypePtr pCont(realScope());
	if (!pCont)
		return;

	FullName_t aName;
	proj().typeNameScoped(pCont->owner(), CHOP_SYMB, aName);
	if (!pCont->typeModule())
	{
		ProjectInfo_t PI(proj());
		//aName.append(proj().typeName(pCont));
		aName.append(PI.TypeName(pCont, CHOP_SYMB));
	}

	MyString s(moduleName());
	assert(!s.empty());
	s += MODULE_SEP;
	s += aName.join();

	MyStreamUtil ssh(ss);
	ssh.WriteString(s);

	//if (pCont->parentField())
		//ssh.WriteString(PI.FieldName(pCont->parentField()));
}

struct Jump_t
{
	DA_t rTop;
	DA_t rCur;
	int xCur;
	Jump_t(DA_t top, DA_t cur, int x)
		: rTop(top),
		rCur(cur),
		xCur(x)
	{}
	bool operator==(const Jump_t& x) const
	{
		return rTop == x.rTop && rCur == x.rCur && xCur == x.xCur;
	}
};

std::vector<Jump_t> mJumps;

int BinViewModel_t::pushJumpIt(adcui::DUMPOS iTop, adcui::DUMPOS iCur, int x)//MyStreamBase &ss)
{
	mJumps.push_back(Jump_t(toDA(iTop), toDA(iCur), x));
	if (mJumps.size() > 1 && mJumps.back() == mJumps.at(mJumps.size() - 2))
		mJumps.pop_back();

	//ss.Write(&r, sizeof(r));
	//int l(lineIt(it));
	//ss.Write(&l, sizeof(l));
	return (int)mJumps.size();
}

int BinViewModel_t::popJumpIt(adcui::DUMPOS iTop, adcui::DUMPOS iCur, int *x)
{
	if (mJumps.empty())
		return -1;
	DumpVisitor_t binDumper(proj(), mDrawObj);
	const Jump_t &a(mJumps.back());
	if (iTop)
		binDumper.SeekIt(iTop, a.rTop, nullptr);
	if (iCur)
		binDumper.SeekIt(iCur, a.rCur, nullptr);
	if (x)
		*x = a.xCur;
	mJumps.pop_back();
	return (int)mJumps.size();
}

int BinViewModel_t::checkJumpTopIt(adcui::DUMPOS iTop, adcui::DUMPOS iCur)//to avoid duplications
{
	if (!mJumps.empty())
	{
		const Jump_t &a(mJumps.back());
		if (a.rTop.row == toDA(iTop).row &&  a.rCur.row == toDA(iCur).row)
			return 1;
	}
	return 0;
}

/*bool BinViewModel_t::listTypesAtIt(adcui::DUMPOS it, MyStreamBase &ss)
{
	assert(miLocked);//ReadLocker lock(this);
	ModuleInfo_t MI(proj(), *module());

	Locus_t l;
	if (!MI.DaToLocus(mDrawObj.toDA(it), l, module()))
		return false;

	MyStreamUtil ssh(ss);

	MyString sLoc;
	for (Locus_t::const_reverse_iterator rit(l.rbegin()); rit != l.rend(); rit++)
	{
		const Frame_t &f(*rit);
		if (sLoc.empty())
		{
			if (f.field())
				sLoc = MI.FieldName(f.field());
			else
				sLoc = "+" + Int2Str(f.addr(), I2S_HEXC);
		}
		if (!f.cont()->typeModule())
			sLoc.prepend(MyString(MI.TypeName(f.cont())) + ".");
	}

	ssh.WriteString(sLoc);

	TypePtr pType(nullptr);
	int iPtr(0);
	size_t iArray(0);
	if (!l.empty())
	{
		if (l.back().field())
			pType = l.back().field()->type();
		if (pType && pType->typeArray())
		{
			iArray = 1;
			do {
				iArray *= pType->typeArray()->total();
				pType = pType->typeArray()->baseType();
			} while (pType && pType->typeArray());
		}
		if (pType && pType->typePtr())
			iPtr = 1;
	}

	ssh.WriteStringf("%d", iArray);

	MyString sType;
	if (pType)
		sType = MI.TypeName(pType);

	mrCore.dumpTypes(ss, sType);

	//iRet = pProj->ExecuteCommand(cmd, &ss);
	return true;
}*/

void BinViewModel_t::postCommandIt(adcui::DUMPOS it, const char *cmd)
{
	MyArgs2 args(cmd);

	ROWID r(mDrawObj.RowIdIt(it));
	char buf[32];
	sprintf(buf, " ~%#X", (int)r);

	args.InsertAt(1, buf);

	adc::CEventCommand* p(new adc::CEventCommand(args.AsString()));
	p->setContextZ(mrCore.getContext())->Release();
	mrMain.postEvent(p);
}

bool BinViewModel_t::checkTaskTop(adcui::DUMPOS it)
{
	assert(miLocked);
	IAnalyzer *pIAnlz(proj().analyzer());
	if (!pIAnlz)
		return false;
	ModuleInfo_t MI(proj(), *module());
	Locus_t l;
	if (!MI.DaToLocus(mDrawObj.toDA(it), l, module()))
		return false;
	if (pIAnlz->currentVA() != l.back().addr())
		return false;
	//must be positioned at the line containg a code
	const char *pc(cellDataIt(CLMN_CODE, it));
	if (!pc[0])//empty line?
		return false;
	return true;
}

adcui::PixmapEnum BinViewModel_t::pixmapIt(adcui::DUMPOS it)
{
	assert(miLocked);//ReadLocker lock(this);
	Debugger_t *pDbg(proj().debugger());
	if (!pDbg)
		return adcui::PIXMAP_NULL;
	ModuleInfo_t MI(proj(), *module());
	Locus_t l;
	if (!MI.DaToLocus(mDrawObj.toDA(it), l, module()) || !l.back().cont()->typeProc())
		return adcui::PIXMAP_NULL;
	const char *pc(cellDataIt(CLMN_CODE, it));
	if (!pc[0])//no code on a line?
		return adcui::PIXMAP_NULL;

	ADDR a(pDbg->current());
	PVOID pv(Seg_t::ADDR2PV(l.seg(), l.back().addr()));
	if (pDbg->hasUserBreakpointAt(pv))
	{
		if (a == l.back().addr() && pDbg->suspended())
			return adcui::PIXMAP_DBG_BP_NEXT;
		return adcui::PIXMAP_DBG_BP;
	}

	if (a == l.back().addr() && pDbg->suspended())
		return adcui::PIXMAP_DBG_NEXT;

	return adcui::PIXMAP_NULL;
}

adcui::Color_t BinViewModel_t::jumpTarget()
{
	//assert(probe());
	return mrCore.getContext()->entityId();// probe()->entityId();
}

bool BinViewModel_t::setCompactMode(bool bOn)
{
	if (!mDrawObj.setCompactMode(bOn))
		return false;
	mrCore.refreshBinaryDump();
	return true;
}

bool BinViewModel_t::isCompactMode() const
{
	return mDrawObj.isCompactMode();
}

bool BinViewModel_t::setValuesOnlyMode(bool bOn)
{
	if (!mDrawObj.setValuesOnlyMode(bOn))
		return false;
	mrCore.refreshBinaryDump();
	return true;
}

bool BinViewModel_t::isValuesOnlyMode() const
{
	return mDrawObj.isValuesOnlyMode();
}

bool BinViewModel_t::setResolveRefsMode(bool bOn)
{
	if (!mDrawObj.setResolveRefsMode(bOn))
		return false;
	mrCore.refreshBinaryDump();
	return true;
}

bool BinViewModel_t::isResolveRefsMode() const
{
	return mDrawObj.isResolveRefsMode();
}

void BinViewModel_t::tipInfoIt(adcui::DUMPOS itPos, int x, MyStreamBase &ss)
{
	MyStreamUtil ssu(ss);
	assert(miLocked);
	DumpVisitor_t binDumper(proj(), mDrawObj);
	ProjectInfo_t PI(mrMain.project());

	MyString s;
	ProbeIn_t probe(x);

	binDumper.Probe(probe, itPos);
	if (!probe.obj())
	{
		AttrIdEnum attr(ATTR_NULL);
		adcui::Color_t iColor(probe.entityId());
		if (iColor == adcui::COLOR_NULL)
			return;
		if (MISC_COLOR_CHECK_RANGE(iColor))
			attr = MISC_COLOR_TO_ATTR(iColor);

		Locus_t l;
		if (!itPos/* <= 0)*/ || !PI.DaToLocus(mDrawObj.toDA(itPos), l, scope()))
			return;
		DA2_t da;
		if (!PI.DaFromAttrAtLocus(l, attr, probe.value(), da))
			return;

		l.clear();
		if (PI.DaToLocus(da, l, module()) && l.field0())
		{
			s = proj().tipName(l.field0(), module());
		}
		else if (attr == ATTR_OFFS)
		{
			MyStrStream ss;
			DumpUtil_t outp(ss);//false);
			outp.out_value(VALUE_t(OPSZ_QWORD, probe.value()), ss);
			ss.flush(s);
			//s = ss.str();
		}
		ssu.WriteString(s);
		return;
	}

	CFieldPtr pField(probe.field());
	if (pField)
	{
		if (module())
		{
			ModuleInfo_t MI(mrMain.project(), *module());
			s = MI.FieldNameDemagled(pField);
			s = ReplaceAll(s, "<", "&lt;");
			s = ReplaceAll(s, ">", "&gt;");
		}
		if (s.isEmpty())
			s = proj().tipName(pField, module());
		if (pField->name() && pField->isTypeThunk())
			s.append(" <b>(thunk)</b>");
	}
	else
	{
		TypePtr pType(probe.typeObj());
		if (!pType)
			return;
		s = proj().tipName(pType, module());
	}
	if (!s.isEmpty())
	{
		std::ostringstream ss;
		ss << "<p style='white-space:pre'>";
		ss << s;
		ss << "</p>";
		ssu.WriteString(ss.str());
	}
}

bool BinViewModel_t::reloadRawData()
{
	bool bRet(false);
	if (mpFakeModule && !mpFakeModule->target()->typeFuncDef())
	{
		Probe_t* pCtx(mrCore.getContext());
		if (pCtx)
		{
			TypePtr pModule(pCtx->locus().module2());
			if (pModule)
			{
				TypePtr iSeg(pCtx->locus().seg());
				mpFakeModule->update(iSeg, pCtx->locus().va(), pModule->typeModule()->dataSourcePtr0());
				bRet = true;
			}
			mrCore.releaseContext(pCtx);
			mrCore.refreshBinaryDump(0, true);
		}
	}
	return bRet;
}



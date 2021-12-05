#include "dump_target.h"
#include "shared/dump_util.h"
#include "type_seg.h"
#include "info_proj.h"

////////////////////////////////////////////////
// DumpTarget_t

DumpTarget_t::DumpTarget_t(DumpInvariant_t &rInv)
	: m_cols(rInv),
	mpInBuf(nullptr),
	mnInBufSize(0),
	mUpperLimit(0),
	mLastRow(mRows.end())
{
	//mIters.add(nullptr);//first entry represents nil
	mpOStream = new std::ostrstream();
	os() << std::ends;
}

DumpTarget_t::~DumpTarget_t()
{
	mpOStream->freeze(0);
	delete mpOStream;
}

adcui::Color_t DumpTarget_t::fieldColor(CFieldPtr pField)
{
	if (pField->isTypeImp())
		return adcui::COLOR_IMPORT_REF;
	if (pField->isTypeExp())
		return adcui::COLOR_EXPORT_REF;
	if (ProjectInfo_t::IsExported(pField))
		return adcui::COLOR_EXPORTED;
	if (pField->isTypeProc())
		return adcui::COLOR_USER_FUNCTION;
	if (pField->nameless())
		return adcui::COLOR_UNNAMED;
	return adcui::COLOR_NULL;
}

void DumpTarget_t::resetTreeColumn()
{
	m_cols.setTreeColumn(adcui::IBinViewModel::CLMN_TREEINFO);
}

bool DumpTarget_t::isTreeColumn(COLID iCol)
{
	DumpInvariant_t &v(m_cols);
	if (v.treeColumn() == adcui::IBinViewModel::CLMN_TREEINFO //needs to be calculate
		&& Column(v.treeColumn()).width != 0)
	{
		for (COLID i(v.treeColumn() + 1); i < (COLID)v.size(); i++)
		{
			const ColDesc_t &col(Column(i));
			if (!(col.flags & (adcui::CLMNFLAGS_HEADER | adcui::CLMNFLAGS_TRIM)))
				if (col.width != 0)
				{
//					if (i == adcui::IBinViewModel::CLMN_TYPES && col.width < 0 && Column(adcui::IBinViewModel::CLMN_NAMES).width > 0)
	//					i = adcui::IBinViewModel::CLMN_NAMES;
					v.setTreeColumn(i);
					break;
				}
		}
	}
	return v.treeColumn() == iCol;
}

bool DumpTarget_t::begin(ROWID da)
{
//	if (!mpOStream)
	//	return false;//not initialized

	//fprintf(stderr, "dumping at %llX\n", da); fflush(stderr);

	// Check if da gets into the gap
	RowMapCIt it0(mRows.lower_bound(da));
	if (it0 != mRows.end())
	{
		if (it0->first == da)
			return false;//the DA has been already dumped

		RowMapCIt it(it0);//handle U-fields
		if (it != mRows.begin())
		{
			--it;//start with one right below
			ROWID from(it->first);
			unsigned span(0);
			for (;;)
			{
				span = std::max(span, it->second.span);
				if (it == mRows.begin())
					break;
				--it;
				if (it->first != from)
					break;
			}

			if (from + span > da)
				return false;
		}

		/*if (it0 != mRows.begin())
		{
			do {
				--it;
			} while (DumpIterator::IsPosterior(it));
			assert(it->first < da);
			unsigned span(it->second.span);
			if (it->first + span > da)
				return false;
		}*/
	}

	mUpperLimit = ROWID_END;

	os().freeze(false);
	//expandColDataVec();
	return true;
}

void DumpTarget_t::end()
{
	//clearColDataVec();
	mnInBufSize = (int)os().tellp();
	mpInBuf = os().str();

	//?lockRead(false);
}

#if(!NEW_VARSIZE_MAP)
int DumpTarget_t::cachedObjSize(ROWID rowID) const
{
	std::map<ROWID, unsigned>::const_iterator it;
	it = mCodeSzMap.find(rowID);
	if (it == mCodeSzMap.end())
		return -1;
	return it->second;
}

void DumpTarget_t::setCachedObjSize(ROWID rowID, ROWID sz)
{
	std::pair<std::map<ROWID, unsigned>::iterator, bool> it;
	it = mCodeSzMap.insert(std::make_pair(rowID, (unsigned)sz));
//?	assert(it.second);
}
#endif

long DumpTarget_t::MyColData::flush(std::ostream &os)
{
	std::vector<long> pos(size());

	// 1)Save the position of every column's data.
	// 2)Write column's data to stream.
	for (COLID i(0); i < (COLID)size(); i++)
	{
		//ColDesc_t &col(Column(i));
		ColData_t &dat(at(i));
		if (!dat.empty())
		{
			pos[i] = (int)os.tellp();
			os << dat;
			os << std::ends;
		}
	}

	// 3) Remember column directory position.
	long dir_pos((long)os.tellp());

	// 4)Write column directory for the current row.
	for (COLID i(0); i < (COLID)pos.size(); i++)
	{
		//ColDesc_t &col(Column(i));
		long a(pos[i]);
		os.write((char *)&a, sizeof(a));
	}

	// 5)Clear the column's cache.
/*	for (COLID i(0); i < cols().size(); i++)
	{
		//ColDesc_t &col(Column(i));
		ColData_t &dat(coldata(i));
		dat.assign("");
		//col.pos = 0;
	}*/

	return dir_pos;
}

void DumpTarget_t::discardLastRow()
{
	m_data.pop_back();
}

ROWID DumpTarget_t::flushRow(ROWID *prow_beg, bool bIncomplete, unsigned forceRange)
{
	assert(!m_data.empty());

	MyColData &dat(m_data.front());
//	if (bIncomplete)
	//	dat[adcui::IBinViewModel::CLMN_DA].append("!");
	ROWID row_beg(dat.da2);
	*prow_beg = row_beg;
	ROWID row_end(row_beg + dat.span);
//?	assert(!dat[1].empty());

	if (forceRange > 0)
		dat.span = forceRange;

	long lPos(dat.flush(os()));
	if (!AddRow(dat, lPos, bIncomplete))
	{
		m_data.clear();
		return row_beg;
	}
	//mrDumpTarget.clearColDataVec();
	//mrDumpTarget.expandColDataVec();
	m_data.pop_front();
	return row_end;
}

size_t DumpTarget_t::coldata_expand()
{
	m_data.push_back(MyColData(m_cols.size()));
	return m_data.size();
}

bool DumpTarget_t::coldata_pruneEmptyTail()
{
	if (coldata_empty() || !m_data.back().at(adcui::IBinViewModel::CLMN_DA).empty())
		return false;
	discardLastRow();
	return true;
}

void DumpTarget_t::coldata_trimDanglingTail(unsigned uLevel)//remove invalid branches for strucvars
{
	std::list<MyColData>::reverse_iterator i(m_data.rbegin());
	if (i != m_data.rend())
	{
		//last entry may be empty
		std::string &s((*i).at(adcui::IBinViewModel::CLMN_DA));
		if (s.empty())
			i++;
	}

	size_t j = uLevel - 1;
	{
		for (; i != m_data.rend(); i++)
		{
			std::string &s((*i).at(adcui::IBinViewModel::CLMN_TREEINFO));
			if (!(j < s.size()))
				break;
			uint8_t u(s[j] - '0');
			if ((u&(~adcui::ITEM_TREE_MASK)) == adcui::ITEM_CONT_STRUCVAR)
			{
				if (u & (adcui::ITEM_HAS_NEXT_CHILD | adcui::ITEM_HAS_CHILD_HERE))
				{
					if (j + 1 < s.size())
					{
						uint8_t v(s[j + 1] - '0');
						if (!(v & adcui::ITEM_HAS_PARENT))
							u = 0;
						else
						{
							u &= ~adcui::ITEM_HAS_NEXT_CHILD;
							//u = 0;
							s[j] = u + '0';
							break;
						}
					}
					else
						u = 0;
					s[j] = u + '0';
				}
			}
		}
	}
}

void DumpTarget_t::drawRowDA(ROWID rowID, unsigned uBit, unsigned w, char buf[32])
{
	PrintHex((unsigned)rowID, w, buf);
	if (uBit > 0)
		sprintf(buf + strlen(buf), ".%d", uBit);
}

void DumpTarget_t::drawRowID(ROWID rowID, unsigned uBit, unsigned w)
{
	char buf[32];
	drawRowDA(rowID, uBit, w, buf);
	draw(adcui::IBinViewModel::CLMN_DA, buf);
}

void DumpTarget_t::drawOrdinal(TypePtr iSeg, ADDR va)
{
	if (m_cols[adcui::IBinViewModel::CLMN_OFFS].width == 0)
		return;

	char buf[32];
	sprintf(buf, "%u", (unsigned)va);

	draw(adcui::IBinViewModel::CLMN_OFFS, buf);
}

void DumpTarget_t::drawAddressV(TypePtr iSeg, ADDR va)
{
	if (m_cols[adcui::IBinViewModel::CLMN_OFFS].width == 0)
		return;

	char buf[32];
	Seg_t &rSeg(*iSeg->typeSeg());
	rSeg.printVA(iSeg, va, buf);

	draw(adcui::IBinViewModel::CLMN_OFFS, buf);
}

void DumpTarget_t::drawAddressR(OFF_t ra)
{
	if (m_cols[adcui::IBinViewModel::CLMN_FILE].width != 0)
	{
		char buf[9];
		sprintf(buf, "%08X", (unsigned)ra);
		draw(adcui::IBinViewModel::CLMN_FILE, buf);
	}
}

/*void DumpTarget_t::drawIndent(int d)
{
	char buf[32];
	sprintf(buf, "%d", d);
	draw(adcui::IBinViewModel::CLMN_TREEINFO, buf);
}

void DumpTarget_t::drawLevelInfo(const char * levelInfoStr)
{
	ColData_t &dat(coldata(adcui::IBinViewModel::CLMN_TREEINFO));

	int iLevel = (int)strlen(levelInfoStr);
	while (iLevel--)
	{
		dat.append(1, levelInfoStr[iLevel]);
	}
}*/


uint8_t DumpTarget_t::GetLevelInfo(int iLevel) const
{
	//if (m_data.empty())
		//return 0;

	const ColData_t &dat(coldata(adcui::IBinViewModel::CLMN_TREEINFO));
	if (iLevel < (int)dat.size())
		return dat[iLevel] - '0';
	return 0;
}

void DumpTarget_t::SetLevelInfo(int iLevel, uint8_t f)
{
	if (m_data.empty())
		return;

	ColData_t &dat(coldata(adcui::IBinViewModel::CLMN_TREEINFO));

	assert(iLevel >= 0);
	while ((int)dat.length() <= iLevel)
		dat.append("0");

	uint8_t n(dat[iLevel]-'0');
	n |= f;
	dat[iLevel] = n+'0';
}

void DumpTarget_t::draw(COLID iCol, const std::string &s)
{
	if (m_cols[iCol].width == 0)
		return;
	if (m_data.empty())
		return;
	ColData_t &dat(coldata(iCol));
	if (!dat.empty())
	{
		if (dat.back() == '\0')
			dat.resize(dat.length() - 1);
		if (dat.back() == '\n')
			return;//has been finalized
	}
	dat.append(s);
}

void DumpTarget_t::drawCode(MyStrStream &os)
{
	//mos << "codeline";
	os << std::ends;
	std::string s(os.str(), (unsigned)os.pcount());
	draw(adcui::IBinViewModel::CLMN_CODE, s);
	os.freeze(false);
}

void DumpTarget_t::drawData(const MyString &s0, int iColor)
{
	MyStrStream os(isColorsEnabled());
	DumpUtil_t outp(os);
	if (iColor != -1)
		outp.set_color(iColor);
	outp.out_string(s0.c_str());
	if (iColor != -1)
		outp.set_color(adcui::COLOR_POP);
	os << std::ends;
	std::string s;
	os.flush(s);
	draw(adcui::IBinViewModel::CLMN_CODE, s);
}

void DumpTarget_t::drawComment(const MyString &s0, bool bIfEmptyOnly)
{
	assert(!s0.empty());
	MyStrStream os(isColorsEnabled());
	DumpUtil_t outp(os);
	ColData_t &dat(coldata(adcui::IBinViewModel::CLMN_COMMENTS));
	if (dat.empty())
	{
		outp.set_color(adcui::COLOR_DASM_COMMENT);
		outp.out_string("; ");
	}
	else if (!bIfEmptyOnly)
		outp.out_string(" ");
	else
		return;
	outp.out_string(s0);
	//s.append(s0);
	//outp.out_string(s0, adcui::COLOR_DASM_COMMENT);
	os << std::ends;
	std::string s2;
	os.flush(s2);
	draw(adcui::IBinViewModel::CLMN_COMMENTS, s2);
}

#define MAX_BYTES_LEN 16

void DumpTarget_t::drawBytes(const void * pBytes, size_t len)
{
	if (len > 0)
	{
		ColData_t &dat(coldata(adcui::IBinViewModel::CLMN_BYTES));
		for (size_t i(0); i < len; i++)
		{
			char buf[3] = { '?', '?', 0 };
			if (pBytes)
				sprintf(buf, "%02X", ((unsigned char *)pBytes)[i]);
			if (!dat.empty())
				dat += " ";
			dat += buf;
		}
	}
}

void DumpTarget_t::drawBytes(const I_DataSourceBase &aRaw, OFF_t oBytes, size_t len)
{
	if (len > 0)
	{
		ColData_t &dat(coldata(adcui::IBinViewModel::CLMN_BYTES));
		for (size_t i(0); i < len; i++, oBytes++)
		{
			char buf[3] = { '?', '?', 0 };
			//if (pBytes)
			unsigned char ch;
			aRaw.dataAt(oBytes, OPSZ_BYTE, (PDATA)&ch);
			sprintf(buf, "%02X", ch);
			if (!dat.empty())
				dat += " ";
			dat += buf;
		}
	}
}

void DumpTarget_t::drawNameRef(CObjPtr pObj, std::ostream &os)
{
	assert(!pObj->objTypeGlob() || !(pObj->flags() & TYP_NO_REF));
	mObjRefs.push_back(pObj);
	unsigned objid(unsigned(mObjRefs.size() - 1));
	os.put((char)MAKECOLORID(adcui::COLOR_NAME));
	os.write((const char *)&objid, sizeof(objid));
}

CObjPtr DumpTarget_t::objFromId(unsigned objid) const
{
	if (objid < mObjRefs.size())
		return mObjRefs[objid];
	return nullptr;
}


void DumpTarget_t::invalidateRows(ROWID da)
{
	RowMapCIt i(mRows.lower_bound(da));
	while (i != mRows.end() && i->first == da)
	{
		RowMapCIt j(i++);
		mRows.erase(j);
	}
}

void DumpTarget_t::appendRowSpan(unsigned d)
{
	assert(mLastRow != mRows.end());
	mLastRow->second.span += d;
}

bool DumpTarget_t::AddRow(const MyColData &dat, long pos, bool bProblem)
{
//CHECK(row==0x5b86)
//CHECK(row != rowShifted)
//STOP

	ROWID rowShifted(dat.da);
	ROWID row(dat.da2);
	unsigned rowSpan(dat.span);

	int adj(0);
	if (row > rowShifted)
		adj = int(row - rowShifted);
	else if (row < rowShifted)
		adj = -int(rowShifted - row);

//fprintf(stdout, "ROW: %08X\n", (unsigned)rowShifted);

	mUpperLimit = ROWID_END;
	mLastRow = mRows.insert(std::make_pair(rowShifted, T_RowData(pos, rowSpan, adj, dat.bit, bProblem)));
	if (mLastRow == mRows.end())
		return false;

	RowMapCIt itnx(mLastRow);
	if (++itnx != mRows.end())
	{
		assert(itnx->first > mLastRow->first);
		mUpperLimit = itnx->first + itnx->second.adjusted;
#if(0)
		if (rowSpan > 0)
		{
			if (row + rowSpan > mUpperLimit)
			{

				mRows.erase(it);
				return false;
			}
		}
#endif
	}
	return true;
}




#include "dump_visit.h"
#include "dump_cache.h"
#include "dump_bin.h"

#define LINES2DUMP	50

DumpVisitor_t::DumpVisitor_t(Project_t& rProjRef, CACHEbin_t& rCache)
	: ProjectInfo_t(rProjRef),
	mrCache(rCache),
	mrTarget(mrCache.target())
{
}

static dumpstr_t *strtag(dumpstr_t *s)
{
	while (*s && *s < 0x80)
		s++;
	return *s ? s : nullptr;
}

FieldPtr DumpVisitor_t::fieldAt(adcui::DUMPOS it) const
{
	const DumpPositionIt &aPos(mrCache.posIt(it));
	dumpstr_t *pCellData((dumpstr_t *)aPos.CellData(adcui::IBinViewModel::CLMN_NAMES));

	while (pCellData)
	{
		dumpstr_t *ps(strtag(pCellData));
		if (ps)
		{
			int w((int)(ps - pCellData));
			if (*ps == MAKECOLORID(adcui::COLOR_NAME))
			{
				ps++;
				unsigned objid(*(unsigned *)ps);
				CObjPtr pObj(mrTarget.objFromId(objid));
				if (pObj->objField())//scopped names are possible
					return pObj->objField();
				ps += sizeof(objid);
				pCellData = (*ps) ? ps : nullptr;
			}
			else if (*ps == MAKECOLORID(adcui::COLOR_POP))
			{
				pCellData = ++ps;
			}
			else
			{
				pCellData = ++ps;
			}
		}
		else
			pCellData = nullptr;
	}
	return nullptr;
}


bool DumpVisitor_t::SetAtField(FieldPtr)
{
	return false;
}

void DumpVisitor_t::DumpZ(ROWID da, int lines)
{
	DataSourceNull_t dataNull;
	BinDumper_t dumpObj(*this, mrTarget, dataNull);

	if (mrTarget.begin(da))
	{
		//CTypePtr iScope(mrCache.scopeRoot());
		//assert(iScope);

		dumpObj.dump(mrCache.scope(), da, lines);

		mrTarget.end();
	}
}

int DumpVisitor_t::Level(DumpPositionIt &) const
{
	assert(0);
	//mr.NewIterator()
	return 0;
}

bool DumpVisitor_t::Seek(DumpPositionIt &r, DA_t da)
{
	DumpIterator &rIt(r.mIt);

	if (da.row >= mrCache.scopeUppperBound())
	{
		//r.mda = DA_t(mrCache.scopeUppperBound(), 0, 0);
		//return true;
		da.row = mrCache.scopeUppperBound();
		assert(da.row > 0);
		da.row--;
	}

	//attach();
	if (mrTarget.rows().empty())
	{
		DumpZ(da.row, LINES2DUMP);
		//attach();
	}
	else
	{
		//rIt.mRowIt = mrTarget.rows().end();//enable multimap to find the first key in a sequence
		rIt.mRowIt = rIt.lower_bound(da.row, 0);//mrCache.scopeBase());

		if (rIt.isAtBegin())
		{
			if (rIt.mRowIt->first > da.row)
			{
				DumpZ(da.row, LINES2DUMP);
				//attach();
			}
		}
		else if (rIt.isAtEnd() || rIt.mRowIt->first > da.row)
		{
			--rIt.mRowIt;
			rIt.mLine = 0;

			assert(da.row >= rIt.RowID());
			if (da.row >= rIt.RowID() + rIt.row_span())
			{
				DumpZ(da.row, 50);
				//attach();
			}
		}
	}

	if (!rIt.seek(da.row, da.line, 0))//mrCache.scopeBase()))
		return false;

	r.mda = rIt.toDA();

	/*RowMapCIt it = mRowIt;
	for (int i = 0; i < line; i++)
	{
	it--;
	assert(it->first == RowID());
	}*/

	return true;
}

bool DumpVisitor_t::CheckEdgeBelow(const DumpIterator &rIt, ROWID &row, unsigned &span) const
{
	if (rIt.isAtEnd())
		return false;


	//span = rIt.row_ span();
	ROWID row0 = rIt.RowID();

CHECK(row0 == 0x5c42)
STOP

	RowMapCIt it(rIt.mRowIt);
	++it;

	ROWID rowNx(0);//maximum posible (when overflow occurs)
	if (it != mrTarget.rows().end())//rIt.isAtEnd());
	{
		if (R_ROW(it) == row0)
			return false;
		if (R_POSTERIOR(it))
			return false;

/*		while (R_POSTERIOR(it))
			if (++it == mrTarget.rows().end())
				break;//skip posterior lines down

		if (it != mrTarget.rows().end())*/
			rowNx = R_ROW(it);
	}

	//get the span
	span = 0;

	while (it != mrTarget.rows().begin())
	{
		if ((--it)->first != row0)
			break;
		span = std::max(span, it->second.span);
	}
	//} while (R_SPAN(it) == 0);//skip posterior lines up

	row = row0;

	if (rowNx == 0)
		return true;

	//row = R_ROW(it);
	//span = R_SPAN(it);

	//if (span > 0)
	if (row + span < rowNx)
		return true;

	assert(row + span == rowNx);
	return false;
}

bool DumpVisitor_t::Forward(DumpPositionIt &r)
{
	DumpIterator &rIt(r.mIt);

	ROWID rowID(rIt.isAtEnd() ? ROWID(-1) : R_ROW(rIt.mRowIt));

#if(0)
	if (rIt.isAtEnd())
		return false;

	unsigned range = rIt.row_ span();
	//ROWID rowID = rIt.RowID();

//CHECK(rowID == 0x9AC)
//STOP

	rIt.mRowIt++;

	bool bEdge(rIt.isAtEnd());
	if (!bEdge)
	{
		//if (rIt.RowID() != rowID)
		if (range > 0)
			if (rowID + range < rIt.RowID())
				bEdge = true;
	}

	if (bEdge)
#else
	ROWID row;
	unsigned span;
	if (CheckEdgeBelow(r.mIt, row, span))
#endif
	{
		row += std::max(unsigned(1), span);
		if (!(row < mrCache.scopeUppperBound()))
		{
			r.mda = rIt.SetAtEnd();
			return false;
		}

		DumpZ(row, LINES2DUMP);
		//attach();
		//if (IsAtEnd())
			//return false;//?
		//if (!isAttached())
			//return false;//the end?
		if (!rIt.seek(row, 0, 0))//mrCache.scopeBase()))
			return false;
	}
	else
	{
		if (!rIt.isAtEnd())
			++rIt.mRowIt;

		ROWID rowID2(rIt.isAtEnd() ? ROWID(-1) : rIt.RowID());
		if (rowID2 == rowID)
			rIt.mLine++;
		else
			rIt.mLine = 0;
	}

	r.mda = rIt.toDA();
	return true;
}


bool DumpVisitor_t::CheckEdgeAbove(const DumpIterator &rIt) const
{
	ROWID rowID = rIt.RowID();

	bool bEdge(rIt.isAtBegin());
	if (!bEdge)
	{
		RowMapCIt it(rIt.mRowIt);
		do {
			--it;
			if (it->first == rowID)
				return false;
		} while (R_POSTERIOR(it));//rIt.IsPost());//span is always on the pre posterior line
		ROWID rowIDpr = R_ROW(it);//rIt.mRowIt->first;
		unsigned span = R_SPAN(it);//rIt.span();//rIt.row_ span();
		assert(span);
		if (rowID < mrCache.scopeUppperBound())
		{
			//is there a gap between previous cache line and the current one?
			if (rowIDpr + span < rowID)
				bEdge = true;
		}
	}
	return bEdge;
}

bool DumpVisitor_t::Backward(DumpPositionIt &r)
{
	DumpIterator &rIt(r.mIt);

	ROWID rowID = rIt.RowID();

	if (CheckEdgeAbove(r.mIt))
	{
		if (rowID <= mrCache.scopeLowerBound())
			return false;
		--rowID;
		DumpZ(rowID, LINES2DUMP);
		//attach();
		if (!rIt.seek(rowID, (unsigned)-1, 0))// mrCache.scopeBase()))
			return false;
	}
	else
	{
		--rIt.mRowIt;

		if (!rIt.isAtBegin())
		{
			if (rIt.RowID() == rowID)
			{
				--rIt.mLine;
			}
			else
			{
				rIt.mLine = 0;
				RowMapCIt it = rIt.mRowIt;
				while (it != mrTarget.rows().begin())
				{
					--it;
					rowID = it->first;
					if (rowID != rIt.RowID())
						break;
					rIt.mLine++;
				}
			}
		}
		else /*if (mRowIt != drawObj.mRows.end())*/
		{
			rIt.mLine = 0;
		}
	}

	r.mda = rIt.toDA();
	return true;
}




adcui::ITER DumpVisitor_t::NewIterator1(adcui::DUMPOS iPos)
{
	if (!mrCache.ptarget())
		return adcui::ITER(-1);
	DumpPositionIt *pPos(new DumpPositionIt(mrTarget));
	adcui::ITER it(mrCache.mPosVec.newIter(iPos, pPos));

	Seek(*pPos, pPos->da());
	return it;
}

//sets a position (iPos) at specified location (da)
bool DumpVisitor_t::SeekIt(adcui::DUMPOS iPos, DA_t da, FieldPtr pField)
{
	adcui::ITER it(NewIterator1(iPos));
	DumpPositionIt &r(mrCache.Iter2PosIt(it));
	bool bOk(Seek(r, da));
	if (bOk && pField)//position at exact field, if specified
	{
		bOk = false;
		assert(!r.mIt.isAtEnd());
		ROWID row0(r.rowID());
		do {
			if (fieldAt(mrCache.PosFromIter(it)) == pField)
			{
				bOk = true;
				break;
			}
			Forward(r);
		} while (!r.mIt.isAtEnd() && r.rowID() == row0);
		if (!bOk)
			bOk = Seek(r, da);//rollback
	}
	mrCache.DeleteIterator(it, true);
	return bOk;
}

bool DumpVisitor_t::ForwardIt(adcui::ITER it)
{
	DumpPositionIt &r(mrCache.Iter2PosIt(it));
	return Forward(r);
}

bool DumpVisitor_t::BackwardIt(adcui::ITER it)
{
	DumpPositionIt &r(mrCache.Iter2PosIt(it));
	return Backward(r);
}

int DumpVisitor_t::LevelIt(adcui::DUMPOS itID)
{
	DumpPosition *itCell(mrCache.mPosVec[itID]);
	DumpPositionIt &r((DumpPositionIt &)*itCell);
	return Level(r);
}

void DumpVisitor_t::dumpcolorstr(const std::string &s, adcui::Color_t e, std::string &o, bool bProbing)
{
	if (mrTarget.isColorsEnabled())
	{
		if (bProbing)
		{
			o.append(1, (char)MAKECOLORID(adcui::COLOR_DASM_PROBE));
			e = adcui::COLOR_NULL;
		}
		if (e != adcui::COLOR_NULL)
			o.append(1, (char)MAKECOLORID(e));
	}
	o.append(s);
	if (mrTarget.isColorsEnabled())
	{
		if (e != adcui::COLOR_NULL)
			o.append(1, (char)MAKECOLORID(adcui::COLOR_POP));
		if (bProbing)
			o.append(1, (char)MAKECOLORID(adcui::COLOR_POP));
	}
}

static char nibble2hex(uint8_t c)
{
	static const char *tab("0123456789ABCDEF");
	return tab[c & 0xF];
}

static dumpstr_t *strtag2(dumpstr_t *s)
{
	while (*s && *s < 0x80)
		s++;
	return s;
}

std::string DumpVisitor_t::CellDataItEx(adcui::DUMPOS iPos, COLID iCol, const Probe_t *probe, const MyLineEditBase* ped)
{
	std::string s;
	const DumpPositionIt &aPos(mrCache.posIt(iPos));
	dumpstr_t *pCellData(nullptr);
	if (iCol == adcui::IBinViewModel::CLMN_DA)
	{
		if (aPos.mIt.isAtEnd())
			return "";
		char buf[32];
		DA_t da2(aPos.da2());
		DumpTarget_t::drawRowDA(da2.row, da2.bit, mrCache.scopeAddressRangeWidth(), buf);
		if (aPos.hasProblem())
			strcat(buf, "!");
		return buf;
	}
	else
		pCellData = (dumpstr_t *)aPos.CellData(iCol);
	if (pCellData)
	while (*pCellData)
	{
		dumpstr_t *ps(strtag2(pCellData));
		std::string s2((char *)pCellData, ps - pCellData);
		s.append(s2);
		if (*ps == 0)
			break;
		else if (*ps == MAKECOLORID(adcui::COLOR_NAME))
		{
			ps++;
			unsigned objid(*(unsigned *)ps);
			CObjPtr pObj(mrTarget.objFromId(objid));
			//if (!pObj) return s;//?
			assert(pObj);
			bool bProbing(probe && probe->obj() == pObj);
			if (bProbing && probe->da() == aPos.da() && ped)//the only field on current line?
				dumpcolorstr(ped->editName(), adcui::COLOR_CUR_EDIT, s, false);
			else
			{
				adcui::Color_t e(adcui::COLOR_NULL);
				TypePtr pType(nullptr);
				CFieldPtr pField(pObj->objField());
				
				if (!pField)
					pType = pObj->objTypeGlob();

				MyString sn;
				unsigned n(0);
				if (pField)
				{
					n = ProjectInfo_t::ChopName(mrProject.fieldDisplayName(pField), sn);
					if (iCol != adcui::IBinViewModel::CLMN_COMMENTS)
						e = DumpTarget_t::fieldColor(pField);
				}
				else if (pType)
				{
					//n = ProjectInfo_t::ChopName(TypeDisplayName(pType), sn);
					n = ProjectInfo_t::ChopName(mrProject.typeDisplayName(pType), sn);
					if (pType->typeStrucvar())
						bProbing = false;
					e = adcui::COLOR_DASM_TYPE;
				}
				else
				{
					//sn = "?";
					ASSERT0;
				}
				if (mrTarget.isColorsEnabled())
				{
					if (bProbing)//editable background
					{
						s.append(1, (char)MAKECOLORID(adcui::COLOR_DASM_PROBE));
						e = adcui::COLOR_NULL;//reset fgnd color to default
					}
					if (e != adcui::COLOR_NULL)
						s.append(1, (char)MAKECOLORID(e));
					s.append(sn);
					if (e != adcui::COLOR_NULL)
						s.append(1, (char)MAKECOLORID(adcui::COLOR_POP));
					if (n > 0)
					{
						s.append(1, (char)MAKECOLORID(adcui::COLOR_DUP_SUFFIX));
						s.append(NumberToString(n));
						s.append(1, (char)MAKECOLORID(adcui::COLOR_POP));
					}
					if (bProbing)//editable background
						s.append(1, (char)MAKECOLORID(adcui::COLOR_POP));
				}
				else
				{
					s.append(sn);
					if (n > 0)
					{
						s.append(1, CHOP_SYMB);
						s.append(NumberToString(n));
					}
				}
			}
			ps += sizeof(objid);
			pCellData = ps;
		}
		else if (*ps == MAKECOLORID(adcui::COLOR_ARRAY))
		{
			pCellData = ++ps;
			dumpstr_t *ps(strtag2(pCellData));
			if (*ps == MAKECOLORID(adcui::COLOR_POP))
			{
				bool bProbing(probe && probe->entityId() == adcui::COLOR_ARRAY && probe->da() == aPos.da());
				if (bProbing && ped)//probe->isEditing())
					dumpcolorstr(ped->editName(), adcui::COLOR_CUR_EDIT, s, false);
				else
				{
					std::string s3((char *)pCellData, ps - pCellData);
					dumpcolorstr(s3, adcui::COLOR_ARRAY, s, bProbing);
				}
				pCellData = ++ps;
			}
			else
			{
				return s;//?
				assert(0);
			}
		}
		else if (*ps == MAKECOLORID(adcui::COLOR_WSTRING))
		{
			pCellData = ps++;
			char16_t wl(*(char16_t *)ps);
			ps += sizeof(char16_t) * (1 + wl);
			std::string s2((char *)pCellData, ps - pCellData);
			s.append(s2);
			pCellData = ps;
		}
		else
		{
			s.append(1, *ps);
			pCellData += s2.length();
			pCellData = ++ps;
		}
	}
	if (iCol == adcui::IBinViewModel::CLMN_TREEINFO)
		for (size_t i(0); i < s.size(); i++)
			s[i] = nibble2hex(s[i] - '0');//make it printable
	return s;
}

std::string DumpVisitor_t::NameFromObjid(unsigned objid) const
{
	CObjPtr pObj(mrCache.ptarget()->objFromId(objid));
	if (!pObj)
		return "?";
	FieldPtr pField(pObj->objField());
	if (pField)
		return mrProject.fieldDisplayName(pField);
	TypePtr pType(pObj->objTypeGlob());
	assert(pType);
	MyString s;
	ProjectInfo_t::ChopName(TypeDisplayName(pType), s, CHOP_SYMB);
	return s;
}

int DumpVisitor_t::strlenx(dumpstr_t *ps) const
{
	int w(0);
	if (ps)
	for(; *ps; ps++)
	{
		if (*ps < 0x80)
			w++;
		else if (*ps == MAKECOLORID(adcui::COLOR_NAME))
		{
			ps++;
			unsigned objid(*(unsigned *)ps);
			std::string s(NameFromObjid(objid));
			size_t l(s.length());
			w += (int)l;
			ps += sizeof(objid) - 1;
		}
	}
	return w;
}


bool DumpVisitor_t::strscan(ProbeIn_t &probe, strscan_data &r, int x0)
{
	while (r.pCellData)
	{
		dumpstr_t *ps(strtag(r.pCellData));
		if (ps)
		{
			int w((int)(ps - r.pCellData));
			if (*ps == MAKECOLORID(adcui::COLOR_NAME))
			{
				r.mx += w;
				ps++;
				unsigned objid(*(unsigned *)ps);

				std::string s(NameFromObjid(objid));
				int l((int)s.length());
				if (r.mx <= x0 && x0 < r.mx + l + 1)
				{
					CObjPtr pObj(mrTarget.objFromId(objid));
					probe.pickObj((ObjPtr)pObj);
					probe.pickRange(r.mx, unsigned(s.length()));
					return true;
				}
				r.mx += l;
				ps += sizeof(objid);
				r.pCellData = (*ps) ? ps : nullptr;
			}
			else
			{
				if (*ps == MAKECOLORID(adcui::COLOR_POP))
				{
					if (r.mx <= x0 && x0 < r.mx + w + 1)
					{
						adcui::Color_t iEntityId(FROMCOLORID(r.entityId));//revert
						if ((adcui::COLOR_ATTRIB_BEGIN <= iEntityId && iEntityId < adcui::COLOR_ATTRIB_END)
							/*iEntityId == adcui::COLOR_DASM_RVA 
							|| iEntityId == adcui::COLOR_DASM_RELOC 
							|| iEntityId == adcui::COLOR_DASM_OFFS*/
							|| iEntityId == adcui::COLOR_DASM_NUMBER
							|| iEntityId == adcui::COLOR_DASM_ADDRESS
							|| iEntityId == adcui::COLOR_ARRAY)
						{
							std::string s((const char *)r.pCellData, w);
							value_t v;
							if (Str2Int(s.c_str(), v))
							{
								probe.pickRange(r.mx, unsigned(s.length()));
								probe.pickEntityId(iEntityId);
								probe.pickValue(v);
							}
						}
						return true;
					}
					r.pCellData = ++ps;
					r.mx += w;
					return false;
				}
				else if (r.mx <= x0 && x0 < r.mx + w)
					return true;//overflow

				unsigned entityId(*ps);
				r.mx += w;
				ps++;

				strscan_data r2(r.aPos, ps, r.mx);
				r2.entityId = entityId;
				if (strscan(probe, r2, x0))
					return true;
				r.pCellData = r2.pCellData;
				r.mx = r2.mx;
			}
		}
		else
			r.pCellData = nullptr;
	}
	return false;
}

void DumpVisitor_t::Probe(ProbeIn_t &probe, adcui::DUMPOS iPos)
{
	DumpPosition &aPos(*mrCache.mPosVec[iPos]);

#if(0)
	DumpPositionIt it(mrTarget);
	Seek(it, aPos.da());
#else
	adcui::ITER hIt(NewIterator1(iPos));
	DumpPositionIt &it(mrCache.Iter2PosIt(hIt));
#endif

	DA_t da(it.da2());
	da.adjust = -it.adjusted();
	probe.pickDA(da);

	int x0(probe.x());
	int xi(0);//ideal left margin (if there were no shifts)
	int x(0);
	int iLevel(0);
	int visible(0);
	
	for (size_t iCol(0); iCol < mrTarget.cols().size(); iCol++)
	{
		ColDesc_t &aCol(mrTarget.Column(COLID(iCol)));

		if (aCol.flags & adcui::CLMNFLAGS_HEADER)
			continue;

		dumpstr_t *pCellData0((unsigned char *)it.CellData((COLID)iCol));

		bool bLevelCol(iCol == adcui::IBinViewModel::CLMN_TREEINFO);
		if (bLevelCol)
			if (pCellData0)//get indentation depth
				iLevel = (int)strlen((const char *)pCellData0);//no special symbols in the column

		int w(aCol.width);
		if (w <= 0)//is the column hidden?
			continue;

		int x2(x);//column left margin
		if (visible > 0 || !adcui::IADCTableModel::bNo1TextShift)
			x2 += adcui::IADCTableModel::iTextShift;//offset from column's left margin where the text starts (for iCol > 0)

		if (mrTarget.isTreeColumn(COLID(iCol)))
			x2 += iLevel * 2;

		int x3(std::max(x2, xi + w));//column right margin

		if (!(aCol.flags & adcui::CLMNFLAGS_TRIM))
		{
			int w0;
			if (bLevelCol)//no color info in this column
				w0 = iLevel;//already gotten
			else
				w0 = strlenx(pCellData0);
			if (x2 + w0 > x3)
				x3 = x2 + w0;
		}

		if (x2 <= x0 && x0 < x3)
		{
			if (!bLevelCol)
			{
				strscan_data a(aPos, pCellData0, x2);
				strscan(probe, a, x0);
			}
			break;
		}
		visible++;
		x = x3;
		xi += w;
	}

	mrCache.DeleteIterator(hIt, true);
}



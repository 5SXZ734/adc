#pragma once

#include <string>
#include "info_proj.h"
#include "dump_iter.h"

class CACHEbin_t;
class DumpTarget_t;
class Probe_t;
class MyLineEditBase;

class DumpVisitor_t : public ProjectInfo_t
{
	CACHEbin_t &mrCache;
	DumpTarget_t &mrTarget;
public:
	DumpVisitor_t(Project_t&, CACHEbin_t&);

	bool CheckEdgeAbove(const DumpIterator &) const;
	bool CheckEdgeBelow(const DumpIterator &, ROWID &, unsigned &span) const;
	void DumpZ(ROWID, int);
	bool Forward(DumpPositionIt &);
	bool Backward(DumpPositionIt &);
	bool Seek(DumpPositionIt &, DA_t);
	int Level(DumpPositionIt &) const;

	adcui::ITER NewIterator1(adcui::DUMPOS);
	bool SeekIt(adcui::DUMPOS, DA_t, FieldPtr);
	bool ForwardIt(adcui::ITER);
	bool BackwardIt(adcui::ITER);
	int LevelIt(adcui::DUMPOS);

	void Probe(ProbeIn_t &, adcui::DUMPOS);

	std::string CellDataItEx(adcui::DUMPOS iPos, COLID, const Probe_t *, const MyLineEditBase*);

	FieldPtr fieldAt(adcui::DUMPOS) const;
	bool SetAtField(FieldPtr);

private:
	int strlenx(dumpstr_t *) const;
	struct strscan_data
	{
		strscan_data(DumpPosition &pos, const unsigned char *p, int x)
			: aPos(pos), pCellData(p), mx(x), entityId(0)
		{
		}
		DumpPosition &aPos;
		const unsigned char *pCellData;
		int mx;
		unsigned entityId;
	};
	bool strscan(ProbeIn_t &p, strscan_data &, int);
	std::string NameFromObjid(unsigned) const;
	void dumpcolorstr(const std::string &s, adcui::Color_t e, std::string &o, bool probing);
};



#pragma once

#include <iostream>

#include "mem.h"
#include "info_proj.h"
class Seg_t;

class HierPrinter : public ProjectInfo_t
{
public:
	HierPrinter(ProjectInfo_t &);
	void print(std::ostream &);

protected:
	void printSeg(std::ostream &os, TypePtr, int) const;
	void printStruc(std::ostream &os, TypePtr, int) const;
	void printUnion(std::ostream &os, TypePtr, int) const;

private:
	void printContentsSeg(std::ostream &os, TypePtr, int l) const;
	void printContentsStruc(std::ostream &os, TypePtr, int l) const;
	void printContentsUnion(std::ostream &os, TypePtr, int l) const;

	void print(std::ostream &os, const NamesMgr_t &rSelf, int l) const;
	void print(std::ostream &, const TypesMgr_t &, int l) const;
	void print(std::ostream &os, CFieldRef, int l) const;
	void printTypeRef(std::ostream &os, TypePtr, int l) const;

	void print(std::ostream &os, const Obj_t &, int l) const;
	void printType(std::ostream &os, TypePtr, int l) const;


	MyString typeNameEx(TypePtr, char chopSymb = 0) const;//including non-terminal types as well (ptrs,arrays, etc)
};
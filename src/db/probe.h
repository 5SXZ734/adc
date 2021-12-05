#pragma once

#include "qx/MyString.h"
#include "interface/IADCGui.h"
#include "shared/misc.h"
#include "obj.h"
#include "locus.h"

class Core_t;

class MyLineEditBase : public adcui::IADCTextEdit
{
protected:
	Core_t &mrCore;
	MyString sEditName;
public:
	MyLineEditBase(Core_t &);
	const MyString &editName() const { return sEditName; }
	void setEditName(const std::string &s){ sEditName = s; }
protected:
	virtual void readData(MyStreamBase &);
	virtual void writeData(MyStreamBase &);
	virtual int apply();
	int post(const std::string&) const;
};





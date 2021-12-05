#ifndef __ANLZ_H__
#define __ANLZ_H__

#include <list>
#include "qx/IUnk.h"

class MyStreamBase;
class Op_t;

class I_Analizer : public My::IUnk
{
public:
	I_Analizer(){}
	virtual ~I_Analizer(){}
	virtual long writeToDoList(MyStreamBase &ss) = 0;
	virtual int process() = 0;
	virtual bool finished() = 0;
	virtual ADDR currentVA() = 0;
	virtual OpPtr currentOp() = 0;
	virtual Folder_t *currentFile() = 0;
	virtual Field_t *currentOpField() = 0;
	virtual void setCurrentFieldRef(Field_t *) = 0;

	virtual void setContextFile(const char *) = 0;
};



#endif//__ANLZ_H__

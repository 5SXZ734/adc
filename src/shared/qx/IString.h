#ifndef __ISTRING_H__
#define __ISTRING_H__

#include "IUnk.h"

namespace My
{
	class IString : public IUnk
	{
	public:
		virtual void set(const char *) = 0;
		virtual const char *get() = 0;
	};

}//namespace My

#endif//__ISTRING_H__

#ifndef __IUNK_H_INCLUDED__
#define __IUNK_H_INCLUDED__

#include <assert.h>

namespace My
{
	class IUnk
	{
		int	m_nRef;
	public:
		IUnk()
		{
			m_nRef = 1;
		}
		virtual ~IUnk()
		{
		}
		virtual int AddRef()
		{
			m_nRef++;
			return m_nRef;
		}
		virtual int Release()
		{
			assert(m_nRef > 0);
			m_nRef--;
			int nRef = m_nRef;
			if ( m_nRef == 0 )
				delete this; 
			return nRef;
		}
		int RefsNum() const { return m_nRef; }
	};

}//namespace My

#endif//__IUNK_H_INCLUDED__





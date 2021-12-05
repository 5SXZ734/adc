#pragma once

#include <QMap>

#include "qx/IUnk.h"
#include "qx/MyStream.h"

class QString;
class ADCStream;

class SpiceStreamHelper : private MyStreamUtil
{
public:
	using MyStreamUtil::ss;
	using MyStreamUtil::ReadChar;
	using MyStreamUtil::ReadBool;
	using MyStreamUtil::ReadInt;
	using MyStreamUtil::ReadDouble;
	using MyStreamUtil::WriteBool;
	using MyStreamUtil::WriteInt;
	using MyStreamUtil::WriteDouble;
	using MyStreamUtil::WriteStringf;
	using MyStreamUtil::WriteStream;
	using MyStreamUtil::SkipBytes;

	SpiceStreamHelper(MyStreamBase &r)
		: MyStreamUtil(r)
	{
	}
	unsigned ReadString(QString *pqs)
	{
		std::string s;
		unsigned u(MyStreamUtil::ReadString(s));
		if (u > 0)
			*pqs = QString::fromStdString(s);
		return u;
	}

	unsigned ReadString(QString &qs, const char *delims = nullptr, bool bAppendDelim = false)
	{
		std::string s;
		unsigned u(MyStreamUtil::ReadString(s, delims, bAppendDelim));
		if (u > 0)
			qs = QString::fromStdString(s);
		return u;
	}

	unsigned ReadUString(QString &qs, const char *delims = nullptr, bool bAppendDelim = false)
	{
		std::string s;
		if (MyStreamUtil::ReadString(s, delims, bAppendDelim))
		{
			qs = QString::fromUtf8(s.c_str());
			return true;
		}
		return false;
	}

	unsigned ReadUStringAppend(QString &qs, const char *delims = nullptr, bool bAppendDelim = false)
	{
		std::string s;
		if (MyStreamUtil::ReadString(s, delims, bAppendDelim))
		{
			qs.append(QString::fromUtf8(s.c_str()));
			return true;
		}
		return false;
	}

	QString ReadString()
	{
		std::string s;
		MyStreamUtil::ReadString(s, "");
		return QString::fromStdString(s);
	}

	QString ReadUString()
	{
		std::string s;
		MyStreamUtil::ReadString(s, "");
		return QString::fromUtf8(s.c_str());
	}

	bool ReadKeyVal(QString &key, QString &val)
	{
		if (!ReadString(key, "=") || !ReadString(val, "\n"))
			return false;
		return true;
	}
	uint WriteString(const char *p)
	{
		return MyStreamUtil::WriteString(p);
	}
	uint WriteString(const QString &s)
	{
		return MyStreamUtil::WriteString(s.toLatin1().constData());
	}
	uint WriteUString(const QString &s)
	{
		return MyStreamUtil::WriteString(s.toUtf8().constData());
	}
	inline uint WriteString(ADCStream &ss);
};

class ADCStream : public My::Stream0, public SpiceStreamHelper
{
public:
	explicit ADCStream(unsigned chunkSize = 0) 
		: My::Stream0(chunkSize),
		SpiceStreamHelper(reinterpret_cast<MyStreamBase &>(*this))
	{
	}
	ADCStream(MyStreamBase &ss) 
		: My::Stream0((unsigned)-1),
		SpiceStreamHelper(ss)
	{
	}
};

uint SpiceStreamHelper::WriteString(ADCStream &ss)
{
	return MyStreamUtil::WriteString(ss);
}

class SpiceStream2 : public ADCStream
{
public:
	SpiceStream2(MyStreamBase &ss) : ADCStream(ss){}

protected:
	virtual unsigned Read(void *p, unsigned sz){ return ss().Read(p, sz); }
	virtual unsigned Write(void *p, unsigned sz){ return  ss().Write(p, sz); }
	virtual void Reset(bool bClean = false){ return ss().Reset(bClean); }
	virtual char Peek() const { return ss().Peek(); }
	virtual void *Data(unsigned &sz){ return ss().Data(sz); }
	virtual unsigned Skip(unsigned sz, int iReset){ return ss().Skip(sz, iReset); }
	virtual unsigned Tellg() const { return ss().Tellg(); }
	virtual unsigned Tellp() const { return ss().Tellp(); }
	virtual unsigned Rewind(unsigned p){ return ss().Rewind(p); }
};

//#define SS_CAST(ss)	reinterpret_cast<SpiceStreamHelper &>(ss)

#define MY_QSCRIPT_DECL(a)	bool MyScript##a(SpiceStreamHelper &ss)

#define MY_QINT(a)		MY_QSCRIPT_DECL(a){ return readInt(ss, n##a); }
#define MY_QBOOL(a)		MY_QSCRIPT_DECL(a){ return readBool(ss, b##a); }
#define MY_QREAL(a)		MY_QSCRIPT_DECL(a){ return readReal(ss, d##a); }

#define MY1_QINT(a)		int n##a; MY_QSCRIPT_DECL(a){ return readInt(ss, n##a); }
#define MY1_QBOOL(a)		bool b##a; MY_QSCRIPT_DECL(a){ return readBool(ss, b##a); }
#define MY1_QREAL(a)		double d##a; MY_QSCRIPT_DECL(a){ return readReal(ss, d##a); }

#define MY2_QINT(a)		MY_QSCRIPT_DECL(a){ return readInt(ss, o.n##a); }
#define MY2_QBOOL(a)		MY_QSCRIPT_DECL(a){ return readBool(ss, o.b##a); }
#define MY2_QREAL(a)		MY_QSCRIPT_DECL(a){ return readReal(ss, o.d##a); }


#define MY_QSTRING(a)	MY_QSCRIPT_DECL(a){ return readQString(ss, s##a); }
#define MY_QUSTRING(a)	MY_QSCRIPT_DECL(a){ return readQUString(ss, s##a); }
#define MY1_QSTRING(a)	QString s##a; MY_QSCRIPT_DECL(a){ return readQString(ss, s##a); } 
#define MY1_QUSTRING(a)	QString s##a; MY_QSCRIPT_DECL(a){ return readQUString(ss, s##a); } 
#define MY2_QSTRING(a)	MY_QSCRIPT_DECL(a){ return readQString(ss, o.s##a); }
#define MY2_QUSTRING(a)	MY_QSCRIPT_DECL(a){ return readQUString(ss, o.s##a); }

#define MY_QIMPL(a)	add(QString(#a), &MyClass::MyScript##a);
#define MY_QSCRIPT(a)	private: \
	static bool readQString(SpiceStreamHelper &ss, QString &qs){ \
		return (/*SS_CAST*/(ss).ReadString(qs, "\n") > 0); } \
	static bool readQUString(SpiceStreamHelper &ss, QString &qs){ \
		return (/*SS_CAST*/(ss).ReadUString(qs, "\n") > 0); } \
	private: \
	typedef a MyClass; \
	typedef bool (MyClass::*MyScript)(SpiceStreamHelper &); \
	typedef QMap<QString, MyScript> MyScriptMap; \
	typedef MyScriptMap::iterator MyScriptMapIt; \
	MyScriptMap m; \
	void add(QString key, MyScript val){ \
		m.insert(key, val); } \
	static bool readString(SpiceStreamHelper &ss, QString &s){ \
		return (ss.ReadString(s, "\n") > 0); } \
	static bool readInt(SpiceStreamHelper &ss, int &n){ \
		QString s; if (!readString(ss, s)) return false; \
		n = s.toInt(); return true; } \
	static bool readBool(SpiceStreamHelper &ss, bool &b){ \
		QString s; if (!readString(ss, s)) return false; \
		b = (s.toInt() != 0); return true; } \
	static bool readReal(SpiceStreamHelper &ss, double &d){ \
		QString s; if (!readString(ss, s)) return false; \
		d = s.toDouble(); return true; } \
	public: \
	bool parse(SpiceStreamHelper &ss){ \
		QString key; \
		while (ss.ReadString(key, "=")) { \
			MyScriptMapIt it = m.find(key); \
			if (it != m.end()){ \
				MyScript pf = it.value(); \
				if (!(this->*pf)(ss)) return false; \
			} else if (key == "List") { \
				int n = 0; if (!readInt(ss, n)) return false; \
				while (n-- > 0) { QString s; if (!readString(ss, s)) return false; } \
			} else if (key == "Data") { \
				int n = 0; if (!readInt(ss, n)) return false; ss.SkipBytes(n); \
			} \
		} \
		ss.SkipBytes((unsigned)-1); \
		return true; \
	}


#define MY_QSTRING0(a)		MY_QSCRIPT_DECL(a){ return readQString0(ss, s##a); }
#define MY_QUSTRING0(a)		MY_QSCRIPT_DECL(a){ return readQUString0(ss, s##a); }
#define MY1_QSTRING0(a)		QString s##a; MY_QSCRIPT_DECL(a){ return readQString0(ss, s##a); } 
#define MY1_QUSTRING0(a)	QString s##a; MY_QSCRIPT_DECL(a){ return readQUString0(ss, s##a); } 
#define MY2_QSTRING0(a)		MY_QSCRIPT_DECL(a){ return readQString0(ss, o.s##a); }
#define MY2_QUSTRING0(a)	MY_QSCRIPT_DECL(a){ return readQUString0(ss, o.s##a); }
#define MY_QSCRIPT0(a)	private: \
	bool readQString0(SpiceStreamHelper &ss, QString &qs){ \
		return (/*SS_CAST*/(ss).ReadString(qs) > 0); } \
	bool readQUString0(SpiceStreamHelper &ss, QString &qs){ \
		return (/*SS_CAST*/(ss).ReadUString(qs) > 0); } \
	MY_QSCRIPT(a)




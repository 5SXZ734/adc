#pragma once

class MyStreamBase
{
public:
	virtual unsigned Read(void *, unsigned) = 0;
	virtual unsigned Write(void *, unsigned) = 0;
	virtual void Reset(bool = false) = 0;
	virtual char Peek() const = 0;
	virtual void *Data(unsigned &) = 0;
	virtual unsigned Skip(unsigned, int = 0) = 0;
	virtual unsigned Tellg() const = 0;
	virtual unsigned Tellp() const = 0;
	virtual unsigned Rewind(unsigned) = 0;

	int Size() const { return Tellp() - Tellg(); }
};




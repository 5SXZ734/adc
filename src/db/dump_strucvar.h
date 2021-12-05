#pragma once

#include "info_strucvar.h"


class DUMPframe_t;
class BinDumper_t;

class StrucVarDumper_t : public StrucvarTracer0_t
{
	friend class BinDumper_t;
protected:
	BinDumper_t &mrDumper;
private:
	std::list<DUMPframe_t *>& m_frames;
	//int mCompleteStatus;//non-zero if run into a problem
public:
	StrucVarDumper_t(BinDumper_t &, const TypeObj_t &r, unsigned scopeRange, std::list<DUMPframe_t *>&);
	virtual ~StrucVarDumper_t();
	void dump(I_Module&);//no try/catch
protected://StrucvarTracer_t
	virtual void Leave() override;
	virtual POSITION cp() const override;
	virtual void OnField(FieldPtr pField, unsigned range, TypePtr iType) override;
	virtual void OnAlign(ADDR newOffs) override;
	virtual TypePtr newScope(const char *sTypeName, SCOPE_enum e, const char *pFieldName, unsigned range, AttrScopeEnum) override;
	virtual TypePtr newScope(FieldPtr, SCOPE_enum e, const char *pFieldName, unsigned range, AttrScopeEnum) override;
	virtual void leave1() override;
	virtual TypePtr scopeContainer() const override;
	virtual TypePtr scopeSeg() const override;
	virtual ADDR scopeOffs() const override;
	virtual unsigned scopeRange() const override;
	virtual void setScopeRange(unsigned range) override;
	virtual bool isLarge() const override;
	virtual TypesMgr_t &typesMgrNS(TypePtr) const override;
	virtual MemoryMgr_t &memMgrNS() const override;
	virtual void discardLastFrame() override;
protected:
	friend class StrucvarDumperIface_t;
	//TypePtr New Scope(const char *sTypeName, SCOPE_enum e, const char *pFieldName, AttrIdEnum);
	OFF_t cpr() const;
	//void markIncomplete();
	void clearFrames();
protected:
	size_t dataAt(OFF_t off, OFF_t siz, PDATA pDest) const;
	OFF_t size() const;
private:
	std::list<DUMPframe_t*>& frames() const { return m_frames; }
	const DUMPframe_t &top() const { return *m_frames.back(); }
	DUMPframe_t &top(){ return *m_frames.back(); }
	void discardLastRow();
};

class StrucvarDumperIface_t : public StrucvarIface_t
{
	StrucVarDumper_t& mr;
public:
	StrucvarDumperIface_t(StrucVarDumper_t& r)
		: StrucvarIface_t(r),
		mr(r)
	{
	}
protected:
	friend class StrucvarDumperIface_t;
	virtual void Leave() override { mr.Leave(); }
	virtual POSITION cp() const override { return mr.cp(); }
	virtual OFF_t cpr() const override { return mr.cpr(); }
protected://I_DataSource
	virtual size_t dataAt(OFF_t off, OFF_t siz, PDATA pDest) const override { return mr.dataAt(off, siz, pDest); }
	virtual OFF_t size() const override { return mr.size(); }
};


class StrucVarDumperEx_t : public StrucVarDumper_t
{
	std::list<DUMPframe_t *> m_frames;
	ROWID mStartDA;
	ADDR mStartVA;
public:
	StrucVarDumperEx_t(BinDumper_t& dumper, const TypeObj_t& self, unsigned scopeRange);
	~StrucVarDumperEx_t();
	ROWID start();
	void end();
private:
	void EnterNoFlushSection();
	void LeaveNoFlushSection();
};




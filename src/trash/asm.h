#pragma once

#include "unfold.h"

class DisplayAsm_t : public DisplayUnfold_t
{
public:
	DisplayAsm_t(File_t *pFile, Hit_t * pHit)
		: DisplayUnfold_t(pFile, pHit)
	{
	}
	~DisplayAsm_t(){}

	virtual void	Func_Dump(Func_t * pSelf);
	virtual int		GetIndent( Op_t * pOp );

	virtual const char * comment(){ return ";"; }

	virtual int		IsRootVisible(Op_t * pOp){ return 1; }
	virtual int		IsLabelVisible(Field_t * pSelf){ return 1; }
	virtual bool	IsDataVisible( Field_t * pSelf ){ return true; }

	virtual void	OutputLabelDecl(Field_t * pLabel);
	virtual void	OutputInclude(File_t * pFile);
	virtual void	OutputData(Field_t * pData);

	virtual void	drawCodeLineT(Op_t * pOp);
	virtual void	OutputOperand0(Op_t * pOp);

	virtual void	OutputLogoStr(const char * pc);
	virtual void	OutputComment(const char * pc, int b );

	void	OutputAsmLocal(Field_t * pData);
//	void	OutputAsmGlobal(Field_t * pData);

	void			AlignOutput(int endpos);
};




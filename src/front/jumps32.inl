
//included into cpu.cpp

IT_FRM(CONDJMP)
	{OP_REL8},
	{OP_REL32},
IT_FRME

// J O //+0//(OF=1)
IT_BEG3(JO)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_OVERFLOW)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_OF)
IT_END3(JO, CONDJMP)//"Jump Relative if Overflow"

// J N O //+1//(OF=0)
IT_BEG3(JNO)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_NOVERFLOW)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_OF)
IT_END3(JNO, CONDJMP)//"Jump Relative if No overflow"

// J B // J C // J N A E //+2//(CF=1)
IT_BEG3(JB)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_BELOW)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_CF)
IT_END3(JB, CONDJMP)//"Jump Relative if Below"
IT_END6(JC, CONDJMP, JB)//"Jump Relative if Carry"
IT_END6(JNAE, CONDJMP, JB)//"Jump Relative if Neither above nor equal"

// J A E // J N B //+3 //(F_CF=0)
IT_BEG3(JAE)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_NBELOW)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_CF)
IT_END3(JAE, CONDJMP)//"Jump Relative if Above or equal"
IT_END6(JNB, CONDJMP, JAE)//"Jump Relative if Not below"
IT_END6(JNC, CONDJMP, JAE)//"Jump Relative if Not carry"

// J E // J Z //+4//(ZF=1)
IT_BEG3(JE)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_ZERO)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_ZF)
IT_END3(JE, CONDJMP)//"Jump Relative if Equal"
IT_END6(JZ, CONDJMP, JE)//"Jump Relative if Zero"

// J N E // J N Z //+5//(ZF=0)
IT_BEG3(JNE)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_NZERO)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_ZF)
IT_END3(JNE, CONDJMP)//"Jump Relative if Not equal"
IT_END6(JNZ, CONDJMP, JNE)//"Jump Relative if Not zero"

// J B E // J N A //+6//[(CF=1)||(ZF=1)]
IT_BEG3(JBE)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_NABOVE)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_CF|F_ZF)
IT_END3(JBE, CONDJMP)//"Jump Relative if Below or equal"
IT_END6(JNA, CONDJMP, JBE)//"Jump Relative if Not above"

// J A // J N B E //+7 //[(CF=0)&&(ZF=0)]
IT_BEG3(JA)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_ABOVE)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_CF|F_ZF)
IT_END3(JA, CONDJMP)//"Jump Relative if Above"
IT_END6(JNBE, CONDJMP, JA)//"Jump Relative if Neither below nor equal"

// J S //+8 //(SF=1)
IT_BEG3(JS)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_SIGN)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_SF)
IT_END3(JS, CONDJMP)//"Jump Relative if Sign"

// J N S //+9 //(SF=0)
IT_BEG3(JNS)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_NSIGN)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_SF)
IT_END3(JNS, CONDJMP)//"Jump Relative if Not sign"

// J P // J P E //+A//(PF=1)
IT_BEG3(JP)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_PARITY)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_PF)
IT_END3(JP, CONDJMP)//"Jump Relative if Parity"
IT_END6(JPE, CONDJMP, JP)//"Jump Relative if Parity even"

// J N P // J P O //+B //(PF=0)
IT_BEG3(JNP)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_NPARITY)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_PF)
IT_END3(JNP, CONDJMP)//"Jump Relative if Not parity"
IT_END6(JPO, CONDJMP, JNP)//"Jump Relative if Parity odd"

// J L // J N G E //+C //(SF<>OF)
IT_BEG3(JL)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_LESS)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_SF|F_OF)
IT_END3(JL, CONDJMP)//"Jump Relative if Less"
IT_END6(JNGE, CONDJMP, JL)//"Jump Relative if Not greater or equal"

// J G E // J N L //+D//(SF=OF)
IT_BEG3(JGE)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_NLESS)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_SF|F_OF)
IT_END3(JGE, CONDJMP)//"Jump Relative if Greater or equal"
IT_END6(JNL, CONDJMP, JGE)//"Jump Relative if Not less"

// J L E // J N G //+E//((ZF=1) || (SF<>OF))
IT_BEG3(JLE)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_NGREATER)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_ZF|F_SF|F_OF)
IT_END3(JLE, CONDJMP)//"Jump Relative if Less or equal"
IT_END6(JNG, CONDJMP, JLE)//"Jump Relative if Not greater"

// J G // J N L E //+F //((ZF=0) && (SF=OF))
IT_BEG3(JG)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_GREATER)
			OPND3(FOPC_CPUSW, OPTYP_UINT16, F_ZF|F_SF|F_OF)
IT_END3(JG, CONDJMP)//"Jump Relative if Greater"
IT_END6(JNLE, CONDJMP, JG)//"Jump Relative if Not less or equal"

////////////////////////////////////////////////////////

// J E C X Z
IT_FRM(JECXZ)//"Jump Relative if ECX register is 0"
	{OP_REL8},
IT_BEG(JECXZ)
//goto op1 if (ecx==0)
	ACTN1(ACTN_GOTOIF)
		OPND1(OPND_1)
		ACTN1(ACTN_ZERO)
			OPND3(OPC_CPUREG, SIZ(R_ECX), OFS(R_ECX))
IT_END(JECXZ)


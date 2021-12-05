

struct ParseMPL_t : public Parser_t
{
	virtual void ParseLine(const char **str);//comment allready stripped

	ParseMPL_t(Builder_i * pBldr)
		: Parser_t(pBldr) {}
protected:
	out2_t * __isInstruction2(const char **str, ins_t&);
	int ParseData(const char **str, char *token);
	int ParseOperandOut(const char **str, ins_t&);
	int Adjust1(opr_t&, out2_t *);
};


struct outMPL_t : public out2_t
{
	outMPL_t	*next;
	static	outMPL_t	*g_list;

	outMPL_t(char *, int (*)[4], ito_t *, int p = 0);
	virtual out2_t	*List(){ return g_list; }
	virtual out2_t	*Next(){ return next; }
};

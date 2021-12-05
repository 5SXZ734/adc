#include "sym_parse.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <list>
#include <set>
#include "shared/defs.h"
#include "db/mem.h"

/*enum TokenTypeEnum
{
	TOK_NULL,
	TOK_SYMBOL,
	TOK_NUMBER,
	TOK_PUNCT
};

struct token_t
{
	std::string	m_name;
	TokenTypeEnum m_tok;
	token_t(const std::string &s, TokenTypeEnum tok)
		: m_name(s),
		m_tok(tok)
	{
	}
};

class tokenizer_t : public std::list<token_t>
{
public:
	tokenizer_t(){}
	tokenizer_t(const char *str)
	{
		tokenize(str);
	}
	bool next_token(const char **ppc)
	{
		EATWSPACE(*ppc);
		const char *beg = *ppc;
		size_t count(0);
		TokenTypeEnum tok = TOK_NULL;
		for (; beg[count]; count++)
		{
			char c(beg[count]);
			if (!iscsym(c))
			{
				if (ispunct(c))
				{
					if (tok == TOK_NULL)
					{
						tok = TOK_PUNCT;
						count++;
					}
				}
				break;
			}
			if (tok == TOK_NULL)
				tok = isdigit(c) ? TOK_NUMBER : TOK_SYMBOL;
		}
		if (count == 0)
			return false;
		std::string s2(beg, count);
		push_back(token_t(s2, tok));
		*ppc += count;
		return true;
	}
private:
	void tokenize(const char *str0)
	{
		const char *str(str0);
		while (next_token(&str))
		{
		}
	}
};*/


#define EATWSPACE(arg)	while ((*arg) && isspace(*arg)) (arg)++;
#define EATNSPACE(arg)	while ((*arg) && !isspace(*arg)) (arg)++;

static void XASSERT(bool expr, int code = 0)
{
	if (!expr)
		throw(code);
}

enum TokenTypeEnum
{
	TOK_NULL,
	TOK_SYMBOL,
	TOK_SYMBOL_X,		//`name'
	TOK_SYMBOL_Y,		//{name}
	TOK_NUMBER,
	TOK_BRACE_OPEN,		// {
	TOK_BRACE_CLOSE,	// }
	TOK_BRACES,			// {}
	TOK_PARENTH_OPEN,	// (
	TOK_PARENTH_CLOSE,	// )
	TOK_PARENTHESIS,	// ()
	TOK_BRACKET_LEFT,	// [
	TOK_BRACKET_RIGHT,	// ]
	TOK_BRACKETS,		// []
	TOK_COMMA,			// ,
	TOK_PERIOD,			// .
	TOK_ELLIPSES,		// ...
	TOK_SCOPE,			// ::
	TOK_ASTERISK,		// *
	TOK_SCOPE_ASTERISK,	// ::*
	TOK_AMPERSAND,		// &
	TOK_DOUBLE_AMPERSAND,	// &&
	TOK_SEMI,			// ;
	TOK_LAST
};

static std::string TokToName(TokenTypeEnum t)
{
	switch (t)
	{
	case TOK_BRACE_OPEN: return "{";
	case TOK_BRACE_CLOSE: return "}";
	case TOK_BRACES: return "{}";
	case TOK_PARENTH_OPEN: return "(";
	case TOK_PARENTH_CLOSE: return ")";
	case TOK_PARENTHESIS: return "()";
	case TOK_BRACKET_LEFT: return "[";
	case TOK_BRACKET_RIGHT: return "]";
	case TOK_BRACKETS: return "[]";
	case TOK_COMMA: return ",";
	case TOK_PERIOD: return ".";
	case TOK_ELLIPSES: return "...";
	case TOK_SCOPE: return "::";
	case TOK_ASTERISK: return "*";
	case TOK_SCOPE_ASTERISK: return "::*";
	case TOK_AMPERSAND: return "&";
	case TOK_DOUBLE_AMPERSAND: return "&&";
	case TOK_SEMI: return ";";
	default:
		break;
	}
	return "";
}

TokenTypeEnum NodeTypeToTok(NodeTypeEnum type)
{
	switch (type)
	{
	case NODE_PTR: return TOK_ASTERISK;
	case NODE_REF: return TOK_AMPERSAND;
	case NODE_RVAL_REF: return TOK_DOUBLE_AMPERSAND;
	case NODE_PTR_TO_MEMBER: return TOK_SCOPE_ASTERISK;
	default:
		assert(0);
	}
	return TOK_NULL;
}

NodeTypeEnum TokToNodeType(TokenTypeEnum tok)
{
	switch (tok)
	{
	case TOK_ASTERISK: return NODE_PTR;
	case TOK_AMPERSAND: return NODE_REF;
	case TOK_DOUBLE_AMPERSAND: return NODE_RVAL_REF;
	case TOK_SCOPE_ASTERISK: return NODE_PTR_TO_MEMBER;
	default:
		assert(0);
	}
	return NODE_NULL;
}

/////////////////////////////////////node_t

node_t *new_node(NodeTypeEnum n)
{
	return new node_t(n);
}

void delete_node(node_t *pn)
{
	if (pn)
	{
		delete_node(pn->left);
		delete_node(pn->right);
#ifdef _DEBUG
		pn->left = nullptr;
		pn->right = nullptr;
#endif
		delete pn;
	}
}


node_t::node_t(NodeTypeEnum t)
	: type(t),
	attr(NodeAttr_NULL),
	parent(nullptr),
	left(nullptr),
	right(nullptr)
{
	setName(type);
}

node_t::~node_t()
{
	assert(!left);
	assert(!right);
}

void node_t::setType(NodeTypeEnum t)
{
	type = t;
	setName(type);
}

/*static bool __iscsym(char c)
{
	if ('a' <= c <= 'z')
		return true;
	if ('A' <= c <= 'Z')
		return true;
	if ('0' <= c <= '9')
		return true;
	if (c == '_')
		return true;
	return false;
}*/

#define ISCSYM(c) (isalnum(c) || ((c) == '_'))

static bool remove_tag(std::string& s, const std::string &tag)
{
	size_t n(s.find(tag));
	if (n == std::string::npos || (n > 0 && ISCSYM(s[n - 1])))
		return false;
	s.erase(n, tag.length());
	return true;
}

static void fix_node_name(node_t *pn)//remove unwanted tags
{
	if (!pn)
		return;
	//fix_node_names(pn->left);
	//fix_node_names(pn->right);
	for (;;)
	{
		if (!remove_tag(pn->name, "struct "))
			if (!remove_tag(pn->name, "class "))
				if (!remove_tag(pn->name, "union "))
					if (!remove_tag(pn->name, "enum "))
						break;
	}
}

void node_t::setName(NodeTypeEnum t)
{
	//name.clear();
	switch (t)
	{
		case NODE_PTR: name = TokToName(TOK_ASTERISK); break;
		case NODE_REF: name = TokToName(TOK_AMPERSAND); break;
		case NODE_RVAL_REF: name = TokToName(TOK_DOUBLE_AMPERSAND); break;
		case NODE_PTR_TO_MEMBER: name = TokToName(TOK_SCOPE_ASTERISK); break;
		case NODE_FUNC: break;
		case NODE_ARRAY: name = TokToName(TOK_BRACKETS); break;
		case NODE_TYPE: break;
		case NODE_NUMBER: break;
		case NODE_COMMA: name = TokToName(TOK_COMMA); break;
		case NODE_SEMI: name = TokToName(TOK_SEMI); break;
		case NODE_VALIST: break;
		default: break;
	}
}

bool node_t::isScopeOpen() const
{
	return (strncmp(name.c_str(), "::", 2) == 0);
}

std::string node_t::toString() const
{
	std::string s;
	switch (attr & NodeAttr__SV_MASK)
	{
	default: break;
	case NodeAttr_STATIC: s.append("static "); break;
	case NodeAttr_VIRTUAL: s.append("virtual "); break;
	}

	switch (attr & NodeAttr__CV_MASK)
	{
	default: break;
	case NodeAttr_CONST: s.append("const "); break;
	case NodeAttr_VOLATILE: s.append("volatile "); break;
	}

	switch (attr & NodeAttr__CC_MASK)
	{
	default: break;
	case NodeAttr_CDECL: s.append("__cdecl "); break;
	case NodeAttr_STDCALL: s.append("__stdcall "); break;
	case NodeAttr_THISCALL: s.append("__thiscall "); break;
	case NodeAttr_FASTCALL: s.append("__fastcall "); break;
	}
	switch (attr & NodeAttr__TT_MASK)
	{
	default: break;
	case NodeAttr_SIGNED: s.append("signed "); break;
	case NodeAttr_UNSIGNED: s.append("unsigned "); break;
	case NodeAttr_ENUM: s.append("enum "); break;
	case NodeAttr_STRUCT: s.append("struct "); break;
	case NodeAttr_CLASS: s.append("class "); break;
	case NodeAttr_UNION: s.append("union "); break;
	}
	switch (attr & NodeAttr__X64_MASK)
	{
	default: break;
	case NodeAttr_PTR64: s.append("__ptr64 "); break;
	}
		
	if (hasName())
		s.append(name);
	else
		s.append("{node}");
	return s;
}

struct token_t
{
	std::string	m_name;
	TokenTypeEnum m_tok;
	node_t *node;
	token_t(const char *s, unsigned n, TokenTypeEnum tok)
		: m_name(s, n),
		m_tok(tok),
		node(nullptr)
	{
	}
	token_t(const std::string &s, TokenTypeEnum tok)
		: m_name(s),
		m_tok(tok),
		node(nullptr)
	{
	}
};

class parser_t : public std::list<token_t>
{
#ifdef _DEBUG
	std::string m_str;
#endif
public:
	parser_t(const char *str, bool bSkipTempl = false)
#ifdef _DEBUG
		: m_str(str)
#endif
	{
		tokenize(str, bSkipTempl);
	}
private:
	void tokenize(const char *str0, bool bSkipTempl)
	{
		const char *str(str0);
		while (next_token(&str, bSkipTempl))
		{
		}
	}
	void wrapSymbolBack(TokenTypeEnum tokStop, TokenTypeEnum tokSkip)
	{
		unsigned level(0);
		bool bSymbol(false);//to whitespace two symbols
		std::string s;
		for (;;)
		{
			TokenTypeEnum tok(back().m_tok);
			if (tok == tokSkip)
				level++;
			else if (tok == tokStop)
				level--;

			if (bSymbol)
				if (tok == TOK_SYMBOL || tok == TOK_COMMA)
					s.insert(0, " ");
			s.insert(0, back().m_name);
			bSymbol = tok == TOK_SYMBOL;
			pop_back();
			if (bSymbol)
				continue;//symbols do not affect the level
			if (level == 0)
				break;
		}
		if (!empty() && back().m_tok == TOK_SYMBOL)
		{
			s.insert(0, back().m_name);
			pop_back();
		}
		while (!empty() && back().m_tok == TOK_SYMBOL)
		{
			if (!ispunct(s[0]))
				s.insert(0, " ");
			s.insert(0, back().m_name);
			pop_back();
		}
		push_back(token_t(s, TOK_SYMBOL));
	}
	bool checkNestedFuncScope()
	{
		for (reverse_iterator i(rbegin()); i != rend(); i++)
		{
			const token_t &t(*i);
			if (t.m_tok == TOK_PARENTH_CLOSE)
			{
				wrapSymbolBack(TOK_PARENTH_OPEN, TOK_PARENTH_CLOSE);
				return true;
			}
			//skip possible func attributes
			if (t.m_tok != TOK_SYMBOL)
				break;
			if (t.m_name != "const")
				break;
		}
		return false;
	}

	bool next_token(const char **ppc, bool bSkipTempl)
	{
		EATWSPACE(*ppc);
		const char *beg = *ppc;
		TokenTypeEnum tok = TOK_NULL;
		int count(0);
		bool bStrip(false);
		for (; beg[count]; count++)
		{
			char c(beg[count]);
			if (!ISCSYM(c) && c != '$')
			{
				bool bBreak(true);
				if (ispunct(c) && tok == TOK_NULL)
				{
					if (!empty())//check for C++ operators
					{
						if (c == '.')//check for '...' (vararg list indicator)
						{
							reverse_iterator j(rbegin());
							if ((*j).m_tok == TOK_PERIOD)
								if (++j != rend())
									if ((*j).m_tok == TOK_PERIOD)
									{
										pop_back();
										back().m_name += c;
										back().m_name += c;
										back().m_tok = TOK_ELLIPSES;
										continue;
									}
						}
/*						else if (c != '(')//closing brace as a terminator
						{
							if (c == ')')//checking for operator()
							{
								reverse_iterator j(rbegin());
								if ((*j).m_tok == TOK_PARENTH_OPEN)
								{
									if (++j != rend())
										if (strncmp((*j).c_str(), "operator", sizeof("operator") - 1) == 0)//startsWith
										{
											std::string s(back());
											pop_back();
											s += c;
											back() += s;
											continue;
										}
								}
							}
							else
							{
								if (strncmp(back().c_str(), "operator", sizeof("operator") - 1) == 0)//startsWith
								{
									back() += c;
									*ppc += 1;//skip to next symbol
									//count -= 1;
									//continue;
									return true;
								}
							}
						}*/
					}

					switch (c)
					{
					case '{'://parse as a solid entity
						for (++count; beg[count]; count++)
						{
							c = beg[count];
							if (c == '}')
								break;
						}
						bStrip = true;
						tok = TOK_SYMBOL_Y;
						/*if (!empty() && back().m_tok == TOK_SYMBOL_X)
							//back().m_name.append(std::string(beg, ++count));
						else
							push_back(token_t(beg, ++count, TOK_SYMBOL_Y));
						*ppc += count;
						return true;*/
						break;
					//case '{': tok = TOK_BRACE_OPEN; break;
					//case '}': tok = TOK_BRACE_CLOSE; break;
					case '(':	//::(anonymous namespace)::
						if (!empty() && back().m_tok == TOK_SCOPE)
						{
							for (++count; beg[count]; count++)
							{
								c = beg[count];
								if (c == ')')
									break;
								assert(c != '(');
							}
							tok = TOK_SYMBOL;
							bBreak = true;
						}
						else
							tok = TOK_PARENTH_OPEN;
						break;
					case ')': tok = TOK_PARENTH_CLOSE; break;
					case '[':
						if (strncmp(*ppc, "[abi:", 5) == 0)
						{
							//back().m_name.append(*ppc, 5);
							count += 5;
							while (beg[count])
							{
								c = beg[count];
								//back().m_name += c;
								count++;
								if (c == ']')
								{
									if (beg[count] == '<' && !scan_for_gt(beg, count))//scan for a matching '>'
										return false;
									break;
								}
								assert(c != '[');
							}
							back().m_name.append(beg, count);
							*ppc += count;
							return true;
						}
						tok = TOK_BRACKET_LEFT;
						break;
/*					case '<':
						if (back().m_tok == TOK_SYMBOL && back().m_name.back() == ']')
						{
						}
						tok = TOK_SYMBOL;
						break;*/
					case ']': tok = TOK_BRACKET_RIGHT; break;
					case '*': tok = TOK_ASTERISK; break;
					case '&':
						tok = TOK_AMPERSAND;
						//check for double amperasand (move semantics)
						if (!empty())
						{
							if (back().m_tok == TOK_AMPERSAND)
							{
								back().m_tok = TOK_DOUBLE_AMPERSAND;
								back().m_name += c;
								*ppc += ++count;
								return true;
							}
							else if (back().m_tok == TOK_DOUBLE_AMPERSAND)
								return false;
						}
						break;
					case ';': return false;//end of statement	//tok = TOK_SEMI; break;
					case ',': tok = TOK_COMMA; break;
					case '.': tok = TOK_PERIOD; break;
					case ':'://check for scope combination
						if (beg[count + 1] != ':')
							break;// return false;
						count++;
						checkNestedFuncScope();
						tok = TOK_SCOPE;
						break;
					case '~'://check destructor
						if (back().m_tok == TOK_SCOPE)
							bBreak = false;
						break;
					case '`'://something like: `vftable'
						for (++count; beg[count]; count++)
						{
							c = beg[count];
							if (c == '\'')
								break;
						}
						tok = TOK_SYMBOL_X;
						bBreak = true;
						bStrip = true;
						break;
					case '\'':
/*?						if (back().m_name[0] != '`')
							return false;//error!
						back().m_name.erase(0, 1);
						*ppc += 1;//skip to next symbol
						return true;*/
						return false;
					case '#'://a chopped identifier
						back().m_name += c;
						*ppc += 1;
						return true;

					case '-'://like Object-in-Gtk
						if (back().m_tok == TOK_SYMBOL)
						{
							if (strncmp(*ppc, "-in-", 4) == 0)
							{
								back().m_name.append(*ppc, 4);
								for (count += 4;; count++)
								{
									c = beg[count];
									if (!c || ispunct(c))
										break;
									back().m_name += c;
								}
								*ppc += count;
								return true;
							}
						}
						//fall thru..
					default:
						//return false;
						tok = TOK_SYMBOL;
						break;
					}
					count++;
				}
				else if (!bSkipTempl)
				{
					if (tok == TOK_SYMBOL)
					{
						if (c == '<')//templated type
						{
							//it may be an 'operator<', not a template arg
							if (strncmp(beg, "operator", count) == 0)
								break;
							if (scan_for_gt(beg, count))//scan for a matching '>'
								bBreak = true;
						}
					}
				}
				if (bBreak)
					break;
			}
			if (tok == TOK_NULL)
			{
				if (isdigit(c))
					tok = TOK_NUMBER;
				else
					tok = TOK_SYMBOL;
			}
		}
		if (count == 0)
			return false;
		if (tok == TOK_NUMBER && back().m_name.back() == '#')//chopped name
			back().m_name.append(std::string(beg, count));
		else if (bStrip)
			push_back(token_t(beg + 1, count - 2, tok));
		else
			push_back(token_t(beg, count, tok));
		*ppc += count;
		return true;
	}

	bool scan_for_gt(const char *beg, int &count)
	{
		//scan for a matching '>'
		int level(0);
		for (; beg[count]; count++)
		{
			char c(beg[count]);
			if (c == '<')
				level++;
			else if (c == '>')
			{
				if (--level == 0)
				{
					++count;
					return true;
				}
			}
		}
		return false;
	}
public:
	TokenTypeEnum scopeOpen(TokenTypeEnum tok)
	{
		if (tok == TOK_PARENTH_OPEN)
			return TOK_PARENTHESIS;
		if (tok == TOK_BRACKET_LEFT)
			return TOK_BRACKETS;
		if (tok == TOK_BRACE_OPEN)
			return TOK_BRACES;
		return TOK_NULL;
	}

	TokenTypeEnum scopeClose(TokenTypeEnum tok)
	{
		if (tok == TOK_PARENTH_CLOSE)
			return TOK_PARENTHESIS;
		if (tok == TOK_BRACKET_RIGHT)
			return TOK_BRACKETS;
		if (tok == TOK_BRACE_CLOSE)
			return TOK_BRACES;
		return TOK_NULL;
	}

	node_t *evaluate()
	{
		bool bTypedef(false);
		if (!empty())
		{
			if (front().m_name == "typedef")
			{
				pop_front();
				bTypedef = true;
			}
			else if (front().m_name == "struct" && back().m_tok == TOK_SYMBOL_Y)
			{
				pop_front();
				node_t *root(new_node(NODE_STRUCT));
				root->setName(front().m_name);
				pop_front();
				std::list<std::string> l;
				std::string &s(front().m_name);
				for (size_t m(0);;)
				{
					size_t n0(s.find(';', m));
					if (n0 == std::string::npos)
						break;
					while (isspace(s[m])) m++;
					size_t n(n0);
					while (isspace(s[n - 1])) n--;
					l.push_back(s.substr(m, n - m));
					m = n0 + 1;
				}
				node_t *cur(root);
				for (auto i(l.begin()); i != l.end(); ++i)
				{
					const std::string &s(*i);
					parser_t parser2(s.c_str());
					assert(!cur->right);//the fields are rhs'es
					cur = parser2.set_node_right(cur, parser2.evaluate());
					STOP
				}
				return root;
			}
		}
		node_t *node(top_node(evaluate(begin(), nullptr)));
		if (node && bTypedef)
		{
			assert(node->type == NODE_NULL);
			node->type = NODE_TYPEDEF;
		}
		return node;
	}

	node_t *evaluate(iterator i0, node_t *cur)
	{
		//static struct AFX_MSGMAP const CImgTip::messageMap


		for (iterator i(i0); true; i++)
		{
			if (i != end() && scopeClose((*i).m_tok) == TOK_NULL)//while not scope closing
			{
				token_t &q(*i);
				TokenTypeEnum tz;
				//go right--->
				if ((tz = scopeOpen(q.m_tok)) != TOK_NULL)//opening scope
				{
					iterator i2(i--);
					iterator i3(i2++);
					cur = evaluate(i2, nullptr);// new_node());
//					cur = top_node(cur);
					iterator i4(i3++);
					erase(i4);
					if (i3 == end())
						throw(-9);
					token_t &o(*i3);

					node_t *top(top_node(cur));

					//replace '(' ')' with a single '()' and set a node ptr

					if (tz == TOK_BRACKETS)
					{
						o.m_tok = tz;
						o.m_name.assign(TokToName(tz));
						o.node = cur;

						token_t &t_prev(*i);
						if (t_prev.m_tok == TOK_PARENTHESIS && t_prev.node)
						{
							node_t *prev_top(top_node(t_prev.node));
							if (prev_top->type == NODE_NULL)//data?
								prev_top = prev_top->left;

							switch (prev_top->type)
							{
							case NODE_PTR:
							case NODE_REF:
							case NODE_RVAL_REF:
								t_prev.m_tok = NodeTypeToTok(prev_top->type);
								t_prev.m_name = TokToName(t_prev.m_tok);
								t_prev.node->setType(NODE_ARRAY);
								assert(o.node->type == NODE_NUMBER);
								set_node_right(t_prev.node, o.node);//set index
								t_prev.node = set_node_left(t_prev.node, new_node());//this is gonna be an array's type
								erase(i3);
								break;
							default:
								//assert(0);
								throw(-8);
							}
						}
						else
						{
							i++;
						}
						cur = nullptr;
					}
					else if (tz == TOK_PARENTHESIS)//func?
					{
						o.m_tok = tz;
						o.m_name.assign(TokToName(tz));
						o.node = cur;

						token_t &t_prev(*i);
						if (t_prev.m_tok == TOK_PARENTHESIS && t_prev.node)
						{
							node_t *prev_top(top_node_func(t_prev.node));
							if (prev_top->type == NODE_NULL)//data
								prev_top = prev_top->left;
							switch (prev_top->type)
							{
							case NODE_PTR:
							case NODE_REF:
							case NODE_RVAL_REF:
							case NODE_PTR_TO_MEMBER:
								t_prev.m_tok = NodeTypeToTok(prev_top->type);
								t_prev.m_name = TokToName(t_prev.m_tok);
								t_prev.node->setType(NODE_FUNC);
								set_node_right(t_prev.node, top_node(o.node));//set arguments
								t_prev.node = set_node_left(t_prev.node, new_node(NODE_TYPE));//ret value
								erase(i3);
								break;
							default:
								//assert(0);
								throw(-9);
							}
						}
						else
						{
							i++;
						}
						cur = nullptr;

						//i++;
						//cur = nullptr;

		/*				cur = new_node(NODE_FUNC);
						set_node_right(cur, top);
						erase(i3);
						//cur = set_node_left(cur, new_node());//this is gonna be an array's type
						*/
					}
				}
				/*else if (q.m_tok == TOK_SYMBOL_X)
				{
					cur = new_node(NODE_VFTABLE);
					//cur = add_node_left(cur);
					//cur->type = NODE_VFTABLE;
					//prepend_name(cur, t.m_name);
				}*/
/*				else if (q.m_tok == TOK_COMMA)
				{
					//token_t &t_prev(*prev(i));
					if (!cur)
//						cur = t_prev.node;
						cur  = new_node();
				}*/
			}
			else
			{
				// <---go left
				iterator j(i);
				j--;
				for (;;)
				{
					token_t &t(*j);
					if (!cur && t.m_tok != TOK_SYMBOL && t.m_tok != TOK_SYMBOL_X && t.m_tok != TOK_SYMBOL_Y)
					{
						if (!t.node || t.m_tok == TOK_PARENTHESIS)
							cur = new_node();
						else if (t.m_tok != TOK_BRACKETS)
							cur = t.node;
					}

					switch (t.m_tok)
					{
					case TOK_COMMA:
					{
						node_t *top = new_node(NODE_COMMA);
						//top->name = ",";
						set_node_right(top, top_node(cur));

						token_t &t_prev(*prev(j));
						if (!t_prev.node)
							cur = add_node_left(top, NODE_TYPE);
						else
						{
							set_node_left(top, top_node(t_prev.node));
							cur = t_prev.node;
							iterator j2(j--);
							erase(j2);
						}
						break;
					}
					case TOK_BRACES://{}
					{
						cur = t.node;
						break;
					}
					case TOK_BRACKETS://[]
					{
						if (!cur)
							cur = t.node;

						node_t *cur_top(top_node(cur));
						token_t &t_prev(*prev(j));
						node_t *prev_top(top_node(t_prev.node));
						if (!prev_top || t_prev.m_tok == TOK_BRACKETS)
						{
							node_t *arr(new_node(NODE_ARRAY));
							//arr->name.assign(TokToName(t.m_tok));
							set_node_right(arr, t.node);
							if (cur_top->type != NODE_ARRAY)
							{
								cur = set_node_left(arr, new_node());//this is gonna be an array's type
								cur->type = NODE_TYPE;
							}
							else//upper dimension of array
								set_node_left(arr, cur_top);//create a higher dimension
						}
						else
						{
							if (prev_top->type == NODE_NULL)//data
								prev_top = prev_top->left;
							switch (prev_top->type)
							{
							case NODE_PTR:
							case NODE_REF:
							case NODE_RVAL_REF:
								t_prev.node->type = NODE_ARRAY;
								t_prev.node->name = t.m_name;
								assert(t.node->type == NODE_NUMBER);
								set_node_right(t_prev.node, t.node);//set index
								t_prev.node = set_node_left(t_prev.node, new_node(NODE_NULL));
								t_prev.m_tok = NodeTypeToTok(prev_top->type);
								t_prev.m_name.assign(TokToName(t_prev.m_tok));
								cur = t_prev.node;
								break;
							default:
								assert(0);
							}
						}
					//	cur = set_node_left(cur, t.node);//set data type?
						break;
					}
					case TOK_PARENTHESIS://()
					{
						if (cur->parent)
						{
							if (cur->parent->type == NODE_REF)// || cur->parent->type == NODE_RVAL_REF)
							{
								cur = cur->parent;
								delete cur->left;
								cur->left = nullptr;
								cur->setAttr(NodeAttr_L_REF);
								cur->name.clear();
							}
							else if (cur->parent->type == NODE_RVAL_REF)// || cur->parent->type == NODE_RVAL_REF)
							{
								cur = cur->parent;
								delete cur->left;
								cur->left = nullptr;
								cur->setAttr(NodeAttr_R_REF);
								cur->name.clear();
							}
						}

						node_t *cur_top(top_node(t.node));

						cur->type = NODE_FUNC;
						set_node_right(cur, cur_top);
						check_operator(j, i0);
						break;
					}
					case TOK_ASTERISK:
					case TOK_AMPERSAND:
					case TOK_DOUBLE_AMPERSAND:
					case TOK_SCOPE_ASTERISK:
					{
						if (!t.node)
						{
							if (cur->type == NODE_NULL)
								cur->setType(NODE_TYPE);

							//assert(cur == top);
							if (cur->type == NODE_FUNC)
							{
								//assert(cur->right);//empty args list?
								if (!cur->left)
									cur = add_node_left(cur, TokToNodeType(t.m_tok));//return type
								else
									ASSERT0;
							}
							else if (cur->type == NODE_TYPE)
							{
								if (!cur->left && cur->hasName())
								{
									cur->type = NODE_NULL;//transform to data
									cur = add_node_left(cur, TokToNodeType(t.m_tok));//data
								}
								else
									cur->setType(TokToNodeType(t.m_tok));
							}
							cur = add_node_left(cur, NODE_TYPE);
						}
						else if (/*cur->type == NODE_NULL &&*/ !cur->left && cur->hasName())//data
						{
							XASSERT(cur->type == NODE_TYPE);
							cur->type = NODE_NULL;
							set_node_left(cur, top_node(t.node));
							cur = t.node;
						}
						break;
					}
/*?					case TOK_AMPERSAND:
					{
						if (!t.node)
						{
							if (cur->type == NODE_FUNC && !cur->left)
							{
								assert(cur->right);
								cur = add_node_left(cur);//return type
							}
							cur->setType(NODE_REF);
							cur = add_node_left(cur);
						}
						break;
					}*/
					case TOK_SCOPE:
					{
						if (!cur->hasName() && cur->attr == NodeAttr_NULL && cur->parent && cur->parent->type == NODE_PTR)
						{
							assert(cur->parent->left == cur);
							cur->parent->setType(NODE_PTR_TO_MEMBER);
						}
//?						if (!cur->has_name() && !cur->type_name.empty())//mast have been a type name, not a data name!
	//?						std::swap(cur->name, cur->type_name);
						cur->name.insert(0, t.m_name);//prepend
						break;
					}
					case TOK_ELLIPSES:
					{
						cur->type = NODE_VALIST;
						cur->name = t.m_name;//type_name
						break;
					}
					case TOK_SYMBOL:
					{
						if (!cur)
						{
							assert(!t.node);
							cur = new_node(NODE_TYPE);//think a type by default
						}
						/*else if (cur->type == NODE_FUNC && !cur->has_name())
						{
							iterator k(check_operator(i));
							if (k != end())
							{
								j = k;
								continue;
							}
						}*/

						if (check_call_type(t.m_name, cur))
						{
							STOP
						}
						else if (cur->name.substr(0, 2) != "::" && check_type(t.m_name, &cur))//not a scope (even if a basic type: int::`RTTI Type Descriptor')
						{
							assert(!cur->hasName());
							//top = new_node();
							//cur->type = NODE_TYPE;
							cur->name = t.m_name;//type_name
						}
						else if (check_type_attr(t.m_name, cur))
						{
							assert(cur->type == NODE_NULL || cur->type == NODE_TYPE);
							if (cur->type != NODE_TYPE)
								cur->type = NODE_TYPE;
//?							if (cur->has_name() && cur->type_name.empty())//mast have been a type name, not a data name!
	//?							std::swap(cur->name, cur->type_name);
						}
						else if (check_sv_attr(t.m_name, cur))
						{
							STOP
						}
						else if (check_cv_attr(t.m_name, &cur))
						{
						}
						else if (check_ptr64_attr(t.m_name, &cur))
						{
							STOP
						}
						else//an identifer
						{
							if ((cur->type == NODE_FUNC || cur->type == NODE_TYPE) && !cur->left && cur->hasName() && !cur->isScopeOpen())//func or data?
							{
								if (check_register_attr(t.m_name, cur))
									break;

								if (cur->type == NODE_TYPE)
									cur->type = NODE_NULL;//convert to data

//CHECK(cur->type == NODE_FUNC && !cur->right)
//STOP
								//assert(cur->type != NODE_FUNC || cur->right);
								cur = add_node_left(cur);//return type cannot be a data (a type only)
								cur->name = t.m_name;//type_name
								fix_node_name(cur);
								cur->type = NODE_TYPE;
								break;
							}
							
							//prepend_name(cur, t.m_name != "#" ? t.m_name : std::string(), "#");//take care of aliased identifiers (name#N)
							prepend_name(cur, t.m_name, "#");
							fix_node_name(cur);
						}
						break;
					}

					case TOK_SYMBOL_X:
					{//
						check_symbol_x(t.m_name, &cur);
						break;
					}

					case TOK_SYMBOL_Y:
					{
						if (!cur)
							cur = new_node(NODE_TYPE);
						prepend_name(cur, "{" + t.m_name + "}");
						break;
					}

					case TOK_NUMBER:
						cur->name = t.m_name;
						cur->type = NODE_NUMBER;
						break;

					default:
						if (cur)
							delete cur;
						//assert(0);
						return nullptr;
					}

					if (j == i0)
					{
						erase(j);
						break;
					}
					iterator j2(j--);
					erase(j2);
				}
				return cur;
			}
		}

		return nullptr;
	}

	void check_symbol_x(const std::string &t, node_t **pcur)
	{
		std::string s2;
		NodeTypeEnum g(symbolXToNode(t, s2));

		node_t *cur(*pcur);
		node_t *old(cur);

#if(NO_BS)
		switch (g)
		{
		case NODE_THUNK:
			cur = new_node(g);
			cur->setName("`" + s2 + "\'");
			break;

		case NODE_VTHUNK:
			cur = new_node(g);
			cur->setName("`" + s2 + "\'");
			break;

		case NODE_NVTHUNK:
			cur = new_node(g);
			cur->setName("`" + s2 + "\'");
			break;

		case NODE_STRING:
			assert(!cur);
			cur = new_node(g);//make it data
			//cur = new_node();//make it data
			//cur = add_node_left(cur, g);
			cur->setName("`" + t + "\'");
			break;
		default:
#endif
			if (!cur)
				cur = new_node(NODE_TYPE);
			prepend_name(cur, "`" + t + "\'");
#if(NO_BS)
			break;
		}
#endif
		*pcur = cur;
	}

	NodeTypeEnum symbolXToNode(const std::string &s, std::string &t)
	{
		t = s;
#if(NO_BS)
		//exact match
		if (s == "string")
			return NODE_STRING;
		//partial match
		static const char * const pfx0 = "thunk to ";
		if (s.rfind(pfx0, 0) == 0)//startsWith
		{
			//t = s.substr(strlen(pfx1));
			return NODE_THUNK;
		}
		static const char * const pfx1 = "virtual thunk to ";
		if (s.rfind(pfx1, 0) == 0)//startsWith
		{
			//t = s.substr(strlen(pfx1));
			return NODE_VTHUNK;
		}
		static const char * const pfx2 = "non-virtual thunk to ";
		if (s.rfind(pfx2, 0) == 0)//startsWith
		{
			//t = s.substr(strlen(pfx2));
			return NODE_NVTHUNK;
		}
#endif
		return NODE_NULL;
	}

	bool check_Y_pfx(const std::string &t, const std::string &pfx, std::string &s)
	{
		assert(pfx.back() == '`');
		if (t.rfind(pfx, 0) != 0)
			return false;
		if (t.back() != '\'')
			return false;
		s = t.substr(pfx.length());
		s.resize(s.length() - 1);
		return true;
	}

	void prepend_name(node_t *cur, const std::string s, const char *sep = nullptr)
	{
		if (cur->hasName())
		{
			if (!cur->isScopeOpen())
			{
				//if (cur->type_name.empty())
				// std::swap(cur->name, cur->type_name);
				if (sep)
					cur->name.insert(0, sep);
				cur->name.insert(0, s);
			}
			else
				cur->name.insert(0, s);
		}
		else
			cur->name = s;
	}

	iterator check_operator(iterator j, iterator i0)
	{
		//check for 'operator' keyword
		iterator k(j);
		while (k != i0)//begin())
		{
			token_t &g(*(--k));
			if (g.m_tok == TOK_SYMBOL)
			{
				if (g.m_name == "operator")
				{
					iterator l(k);
					while (++l != j)
					{
						char c1(g.m_name.back());
						char c2((*l).m_name.front());
						if (ISCSYM(c1) && ISCSYM(c2))
							g.m_name.append(" ");
						g.m_name.append((*l).m_name);
						erase(l);
						l = k;
					}
					//bSkip = true;
					//j = k;
					return k;
				}
				if (g.m_name == "decltype")
				{
					STOP
				}
			}
		}
		return end();
	}

	TokenTypeEnum next_tok(iterator i)
	{
		iterator j(i);
		if (++j == end())
			return TOK_NULL;
		return (*j).m_tok;
	}
	class type_sym_map : public std::set<std::string>
	{
	public:
		type_sym_map()
		{
			insert("void");
			insert("char");
			insert("short");
			insert("int");
			insert("__int8");
			insert("__int16");
			insert("__int32");
			insert("__int64");
			insert("__int128");
			insert("long");
			insert("float");
			insert("double");
			//insert("signed");
			//insert("unsigned");
			insert("bool");
			insert("char16_t");
			insert("char32_t");
			insert("wchar_t");
			//insert("__cdecl");
			//insert("__stdcall");
			//insert("__fastcall");
			//insert("__thiscall");
		}
	};
	bool check_type(const std::string &s, node_t **pcur)
	{
		static type_sym_map m;
		if (m.find(s) == m.end())
			return false;
		node_t *cur(*pcur);
		if (cur->type == NODE_FUNC && !cur->left)
		{
			//assert(cur->right);//empty args list?
			cur = add_node_left(cur, NODE_TYPE);//func (return) type
			*pcur = cur;
		}
		else if (cur->type == NODE_TYPE && cur->hasName())
		{
			//convert to data
			cur->type = NODE_NULL;//do not change the name
			cur = add_node_left(cur, NODE_TYPE);//data type
			*pcur = cur;
		}
		cur->setAttr(NodeAttr__BasicType);
		return true;
	}
	bool check_call_type(const std::string &t, node_t *cur)
	{
		if (t == "__cdecl")
			cur->setAttr(NodeAttr_CDECL);
		else if (t == "__stdcall")
			cur->setAttr(NodeAttr_STDCALL);
		else if (t == "__fastcall")
			cur->setAttr(NodeAttr_FASTCALL);
		else if (t == "__thiscall")
			cur->setAttr(NodeAttr_THISCALL);
		else
			return false;
		return true;
	}
	bool check_type_attr(const std::string &t, node_t *cur)
	{
		//assert(cur->type == NODE_TYPE);
		if (t == "signed")
			cur->setAttr(NodeAttr_SIGNED);
		else if (t == "unsigned")
			cur->setAttr(NodeAttr_UNSIGNED);
		else if (t == "enum")
			cur->setAttr(NodeAttr_ENUM);
		else if (t == "struct")
			cur->setAttr(NodeAttr_STRUCT);
		else if (t == "union")
			cur->setAttr(NodeAttr_UNION);
		else if (t == "class")
			cur->setAttr(NodeAttr_CLASS);
		else
			return false;
		return true;
	}

	bool check_ptr64_attr(const std::string &t, node_t **pcur)
	{
		node_t *cur(*pcur);
		if (t == "__ptr64")
		{
			cur->setAttr(NodeAttr_PTR64);
		}
		else
			return false;
		return true;
	}

	bool check_sv_attr(const std::string &t, node_t *cur)
	{
		if (t == "static")
			top_node(cur)->setAttr(NodeAttr_STATIC);
		else if (t == "virtual")
			top_node(cur)->setAttr(NodeAttr_VIRTUAL);//appicable only to the functions
		else
			return false;
		return true;
	}

	bool check_cv_attr(const std::string &t, node_t **pcur)
	{
		node_t *cur(*pcur);
		//assert(cur->type == NODE_TYPE);
		if (t == "const")
		{
			if (cur->type == NODE_FUNC && !cur->left)//retured value const attribute!
			{
				cur = add_node_left(cur, NODE_TYPE);
				*pcur = cur;
				cur->setAttr(NodeAttr_CONST);
			}
			else
			{
	//			node_t *node = new_node(NODE_CONST);
//				set_node_right(node, cur);

				//cur = add_node_left(cur, NODE_TYPE);
				cur->setAttr(NodeAttr_CONST);
			}
		}
		else if (t == "volatile")
			cur->setAttr(NodeAttr_VOLATILE);
		else
			return false;
		return true;
	}

	bool check_register_attr(const std::string &t, node_t *cur)
	{
		if (t == "register")
		{
			cur->setAttr(NodeAttr_REGISTER);
			return true;
		}
		return false;
	}

	node_t *set_node_right(node_t *cur, node_t *right)
	{
		assert(!cur->right);
		cur->right = right;
		if (right)
			right->parent = cur;
		return right;
	}

	node_t *set_node_left(node_t *cur, node_t *left)
	{
		assert(!cur->left);
		cur->left = left;
		left->parent = cur;
		return left;
	}

	node_t *top_node(node_t *node)
	{
		if (node)
			while (node->parent)
				node = node->parent;
		return node;
	}

	node_t *top_node_func(node_t *node)
	{
		if (node)
		while (node->parent && node->parent->type != NODE_FUNC)
			node = node->parent;
		return node;
	}

	node_t *add_node_right(node_t *cur, NodeTypeEnum attr = NODE_NULL)
	{
		assert(!cur->right);
		cur->right = new_node(attr);
		cur->right->parent = cur;
		return cur->right;
	}
	node_t *add_node_left(node_t *cur, NodeTypeEnum attr = NODE_NULL)
	{
		assert(!cur->left);
		cur->left = new_node(attr);
		cur->left->parent = cur;
		return cur->left;
	}
	iterator prev(iterator i)
	{
		iterator j(i);
		return --j;
	}
};

static unsigned node_depth(node_t *node)
{
	unsigned u(0);
	for (; node->parent; u++)
		node = node->parent;
	return u;
}

static void print_indent(unsigned u, std::ostream &os)
{
	for (size_t t(0); t < u; t++)
		os << "\t";
}

node_t *SymParse(const char *str)
{
	parser_t parser(str);
	try
	{
		return parser.evaluate();
	}
	catch(int)
	{
		//memory leaks?
	}
	catch(...)
	{
	}
	fprintf(STDERR, "Error: Failed to parse symbol: %s\n", str);
	return nullptr;
}

void SymParsePrint(node_t *node, std::ostream &os)
{
	node_t nul(NODE_NULL);

	std::list<node_t *> l;
	l.push_back(node);
	//iterator i(begin());
	while (!l.empty())
	{
		node_t *cur(l.back());
		l.pop_back();
		print_indent(node_depth(cur), os);
		std::string s(cur->toString());
		if (cur->type == NODE_FUNC)
			s.append("()");
		else if (cur->type == NODE_TYPEDEF)
			s.append(" {typedef}");
		else if (cur->type == NODE_NULL)
			s.append(" {data}");
		os << s << std::endl;
		if (cur->right)
			l.push_back(cur->right);
		else
			if (cur->type == NODE_FUNC)
			{
				l.push_back(&nul);
				nul.parent = cur;
			}
		if (cur->left)
			l.push_back(cur->left);
		else
			if (cur->type == NODE_FUNC)
			{
				l.push_back(&nul);
				nul.parent = cur;
			}
	}
}

void SymParsePrintFlat(node_t *node, std::ostream &os)
{
	if (node->left)
	{
		SymParsePrintFlat(node->left, os);
		if (node->type == NODE_FUNC)
			os << " ";
	}
	os << node->toString();
	if (node->right)
	{
		if (node->type == NODE_FUNC)
			os << "(";
		else
			os << " ";
		SymParsePrintFlat(node->right, os);
		if (node->type == NODE_FUNC)
			os << ")";
	}
}

static void append_tabs(std::string &o, int n)
{
	for (int j(0); j < n; j++)
		o.append("    ");//4 spaces
		//o.append("\t");
}

std::string FormatTemplatedType(const std::string &s, int level0, const std::string &scope)
{
	std::string o;

	int level(level0 * 2);

	parser_t parser(s.c_str(), true);

	for (parser_t::iterator i(parser.begin()); i != parser.end(); i++)
	{
		token_t &t(*i);
		if (t.m_name == "<")
		{
			o.append(t.m_name + "\n");
			append_tabs(o, ++level);
			continue;
		}
		if (t.m_name == ">")
		{
			parser_t::iterator j(i);
			j++;
			o.append(t.m_name);
			if (j != parser.end())
			{
				if ((*j).m_tok == TOK_COMMA)//leave comma on the same line
				{
					o.append(",");
					i++;
				}
				else if ((*j).m_name == ">")//repeated template clousure on the same line
				{
					if (--level < level0)
					{
						o.append("...");
						break;
					}
					continue;
				}
			}
			o.append("\n");
			if (--level < level0)
			{
				o.append("...");
				break;
			}
			append_tabs(o, level);
			continue;
		}
		if (t.m_tok == TOK_COMMA)
		{
			o.append(t.m_name + "\n");
			append_tabs(o, level);
			continue;
		}
		if (i != parser.begin() && !scope.empty() && t.m_name.rfind(scope, 0) == 0)//startsWith, but not the object's name itself
			o.append(t.m_name.substr(scope.length()));//strip a scope prefix
		else
			o.append(t.m_name);
	}

	return o;
}

#include "dump_proto.h"

void SymParsePrintProto(node_t *node, std::ostream &os)
{
	TProtoDumper<ProtoImpl4Sym_t> proto(os);
	if (node->type == NODE_TYPEDEF)
		proto.dumpTypedefDeclaration(node, nullptr);
	else if (node->type == NODE_FUNC)
		proto.dumpFunctionDeclaration(node, nullptr);
	else
		proto.dumpFieldDeclaration(node, node->left, nullptr);
}

#ifdef _DEBUG

/*struct test_s
{
	static int const (*)[12][8] a;
	static int const (*b)[12][8];
};

void test_f()
{
	test_s a;
}*

/*void testtt(float const (*const p)[2])
{
	float a = (*p)[0];
	float b = *p[1];
	float c = *p[2];
}

int(__cdecl*__cdecl test3(void))(void)
{
	return 0;
}

int(*data3)[8] = nullptr;
//(int(*))[8] data4 = nullptr;
typedef int(*type3)[8]	;

void test5(type3 zz)
//.void test5(int(*)[8] zz)
{

}*/

//////////////////////////////////////////////////////////////

//_nc::gui::InspectorItem* nc::gui::(anonymous namespace)::findDescendant<nc::gui::InspectorView::highlightNodes(std::vector<nc::core::likec::TreeNode const*, std::allocator<nc::core::likec::TreeNode const*> > const&)::{lambda(nc::gui::InspectorItem*)#1}>(nc::gui::InspectorItem*, int, nc::gui::InspectorView::highlightNodes(std::vector<nc::core::likec::TreeNode const*, std::allocator<nc::core::likec::TreeNode const*> > const&)::{lambda(nc::gui::InspectorItem*)#1})
//T1* nc::gui::(anonymous namespace)::findDescendant<nc::gui::InspectorView::highlightNodes(std::vector<T2*, std::allocator<T2*> > const&)::{lambda(nc::gui::InspectorItem*)#1}>(nc::gui::InspectorItem*, int, nc::gui::InspectorView::highlightNodes(std::vector<T2*, std::allocator<T2*> > const&)::{lambda(nc::gui::InspectorItem*)#1})
//T1* nc::gui::(anonymous namespace)::findDescendant<nc::gui::InspectorView::highlightNodes(T3 const&)::{lambda(nc::gui::InspectorItem*)#1}>(nc::gui::InspectorItem*, int, nc::gui::InspectorView::highlightNodes(T3 const&)::{lambda(nc::gui::InspectorItem*)#1})
//T1* S0::findDescendant<S1::highlightNodes(T3 const&)::{lambda(T4*)#1}>(T4*, int, S1::highlightNodes(T3 const&)::{lambda(T4*)#1})
//T1* S0::findDescendant<F0::{L1}}>(T4*, int, F0::{L1})
//T1* S0::findDescendant<F1>(T4*, int, F1)
//T1* F2(T4*, int, F1)


//_nc::gui::(anonymous namespace)::printTree(nc::core::likec::Tree const&, nc::gui::RangeTree&)::Callback::onEndPrinting(nc::core::likec::TreeNode const*)
//S0::printTree(T1::Tree const&, T2&)::Callback::onEndPrinting(T1::TreeNode const*)

//char const * const `private: int __cdecl std::num_get<wchar_t,class std::istreambuf_iterator<wchar_t,struct std::char_traits<wchar_t> > >::_Getffld(char * __ptr64,class std::istreambuf_iterator<wchar_t,struct std::char_traits<wchar_t> > & __ptr64,class std::istreambuf_iterator<wchar_t,struct std::char_traits<wchar_t> > & __ptr64,class std::ios_base & __ptr64,int * __ptr64)const __ptr64'::`4'::_Src
//char const * const `private: int __cdecl std::num_get<wchar_t,class std::istreambuf_iterator<wchar_t,T1> >::_Getffld(char * __ptr64,class std::istreambuf_iterator<wchar_t,T1> & __ptr64,class std::istreambuf_iterator<wchar_t,T1> & __ptr64,class std::ios_base & __ptr64,int * __ptr64)const __ptr64'::`4'::_Src
//char const * const `private: int __cdecl std::num_get<wchar_t,T2>::_Getffld(char * __ptr64,T2 & __ptr64,T2 & __ptr64,class std::ios_base & __ptr64,int * __ptr64)const __ptr64'::`4'::_Src
//char const * const `private: int __cdecl std::S1::_Getffld(A1,A2,A3,A4,A5)const __ptr64'::`4'::_Src
//char const * const S1::S2::_Src

const char *test_str[] = {
	"int::`RTTI Type Descriptor'",

	"bool std::operator==<std::__detail::_NFA<std::regex_traits<char> > >(std::shared_ptr<std::__detail::_NFA<std::regex_traits<char> > > const&, decltype(nullptr))",
	"std::_Tuple_impl<0ul, std::string&&>::_Tuple_impl<std::string<>, void>(std::string<>&&, (void&&)...)",

	"class listIterator<class MapeItem> __thiscall CMissedDoc::GetMissionIterator(void) const",
	"__thiscall CEmbeddedButActsLikePtr<class CMapPtrToPtr>::CEmbeddedButActsLikePtr<class CMapPtrToPtr>(void)",
	"`non-virtual thunk to Gtk::AccelLabel::~AccelLabel()'",
	"`virtual thunk to Gtk::UIManager::~UIManager()'",
	"virtual void * __thiscall ADCGuiThread::`vector deleting dtor'`adjustor{8}'(unsigned int)",
	"struct POINT { LONG x; LONG y; }",
	"struct _cheats* __thiscall Mission::GetCheats(void)",
	"double _CIfmod(register double, register double)",
	"long _ftol(register double)",
	"__cdecl QTextStreamManipulator::QTextStreamManipulator(void (__cdecl QTextStream::*)(int) __ptr64,int) __ptr64",
	"__thiscall QScopedArrayPointer<int,struct QScopedPointerArrayDeleter<int> >::H$00::H$00(int *)",
	"BOOL __stdcall IsTextUnicode(const VOID *lpv, int iSize, INT *lpiResult)",
	"const int var",
	"void F0()",
	"`string'",
	"int(__stdcall* FONTENUMPROCW)(const struct LOGFONTW *, const VOID *, DWORD, LPARAM)",
	"typedef int(__stdcall* FONTENUMPROCW)(const struct LOGFONTW *, const VOID *, DWORD, LPARAM)",
	"typedef char CHAR",
	"char CHAR",
	"std::_Bind_simple_helper<void (std::thread::*)(), std::reference_wrapper<std::thread> >::__type std::__bind_simple<void (std::thread::*)(), std::reference_wrapper<std::thread> >(void (std::thread::*&&)(), std::reference_wrapper<std::thread>&&)",

	"int _M_extract_int[abi:cxx11]<int>(int) const",
	"std::istreambuf_iterator<char, std::char_traits<char> > std::num_get<char, std::istreambuf_iterator<char, std::char_traits<char> > >::_M_extract_int[abi:cxx11]<unsigned int>(int) const",
	"std::istreambuf_iterator<char, std::char_traits<char> > std::num_get<char, std::istreambuf_iterator<char, std::char_traits<char> > >::_M_extract_int[abi:cxx11]<unsigned int>(std::istreambuf_iterator<char, std::char_traits<char> >, std::istreambuf_iterator<char, std::char_traits<char> >, std::ios_base&, std::_Ios_Iostate&, unsigned int&) const",

	//"static int const (*)[12][8] icu_58::Calendar::kDatePrecedence",
	"static int const (* icu_58::Calendar::kDatePrecedence)[12][8]",
	"void (__cdecl*)(char const * __ptr64) `RTTI Type Descriptor'",
	"char const * __ptr64 `RTTI Type Descriptor'",
	"class std::error_category `RTTI Type Descriptor'",
	"int `RTTI Type Descriptor'",
	"char * `RTTI Type Descriptor'",
	"struct Stub_t * `RTTI Type Descriptor'",
	"class QAbstractItemView `RTTI Type Descriptor'",

	"const CFSStr::`vftable'",
	"static int const __stdcall Countries::GetItemCount(void)",

	"static char16_t const * const xercesc_3_2::PSVIUni::fgSchemaAnnotations",
	//"static UNKNOWN const * const xercesc_3_2::PSVIUni::fgSchemaAnnotations",

	"__cdecl std::locale::id::operator unsigned __int64(void) __ptr64",

	"const osgUtil::BaseOptimizerVisitor::`vbtable'",
	"`virtual thunk to std::istrstream::~istrstream()'",

	"Gdk::AtomStringTraits::to_cpp_type[abi:cxx11](_GdkAtom* a8h)",
	"Gtk::SelectionData::get_targets[abi:cxx11]()",

	"Gtk::Object-in-Gtk::AccelLabel::`construction vftable'",
	
	"const CFileNameEdit::`vftable'{for `CImgCtrl'}",
	"void __thiscall ifstream::`vbase destructor'(void)",

	"char const * const `private: int __cdecl std::num_get<wchar_t,class std::istreambuf_iterator<wchar_t,struct std::char_traits<wchar_t> > >::_Getffld(char * __ptr64,class std::istreambuf_iterator<wchar_t,struct std::char_traits<wchar_t> > & __ptr64,class std::istreambuf_iterator<wchar_t,struct std::char_traits<wchar_t> > & __ptr64,class std::ios_base & __ptr64,int * __ptr64)const __ptr64'::`4'::_Src",

	"(anonymous namespace)::ARMFastISel::ARMSimplifyAddress((anonymous namespace)::Address&, llvm::MVT, bool)",
	"(anonymous namespace)::DisasmMemoryObject::~DisasmMemoryObject()",

	"F1(std::_Any_data const&, nc::core::ir::BasicBlock const*)",

	"F1(T1&, T2 const&) const::{L1}::operator()(char const*) const",

	"F1(void (std::thread::*&&)())::zz()",

	"__thiscall QTextStreamManipulator::QTextStreamManipulator(void (__thiscall QTextStream::*)(int),int)",

	"void (std::thread::*)()",
	"void (std::thread::*&&)()",
	"std::reference_wrapper<std::thread>&&",

	"std::call_once<void (std::thread::*)(), std::reference_wrapper<std::thread> >(std::once_flag&, void (std::thread::*&&)(), std::reference_wrapper<std::thread>&&)::{lambda()#2}::operator()() const",

	"transaction clone for std::bad_exception::what() const",

	"std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*)",

	"__thiscall QAuthenticator::QAuthenticator(void)",
	
	///
	"static struct XYZF64 (*)[8] AirFormationDB::m_FormTables",
	"static char const * const Mission::_pszDebriefFileBaseName",
	"DWORD zzz",
	"DWORD",
	"static struct ImgItemChkHelper CImgItemChk::m_helper",
	"class QDebug __cdecl operator<<(class QDebug, enum QAbstractSocket::SocketError)",
	"operator *() const",
	"T& operator *() &",
	"T&& operator *() &&",
	"T const& operator *() const&",

	"nc::arch::x86::X86MasterAnalyzer::detectCallingConvention(nc::core::Context&, nc::core::ir::calling::CalleeId const&) const::{lambda(char const*)#1}::operator()(char const*) const",


	"void S0::parseSegmentCommand<segment_command, section>()",
	"void nc::input::mach_o::(anonymous namespace)::MachOParserImpl::parseSegmentCommand<segment_command, section>()",

	"S0::printTree(T1::Tree const&, T2&)::Callback::onEndPrinting(T1::TreeNode const*)",
	"_nc::gui::(anonymous namespace)::printTree(nc::core::likec::Tree const&, nc::gui::RangeTree&)::Callback::onEndPrinting(nc::core::likec::TreeNode const*)",//nested funcs?

	"T1* nc::gui::(anonymous namespace)::findDescendant<F0::{L1}>(T4*, int, F0::{L1})",
	"T1* S0::findDescendant<F0::{L1}>(T4*, int, F0::{L1})",
	"_nc::gui::InspectorItem* nc::gui::(anonymous namespace)::findDescendant<nc::gui::InspectorView::highlightNodes(std::vector<nc::core::likec::TreeNode const*, std::allocator<nc::core::likec::TreeNode const*> > const&)::{lambda(nc::gui::InspectorItem*)#1}>(nc::gui::InspectorItem*, int, nc::gui::InspectorView::highlightNodes(std::vector<nc::core::likec::TreeNode const*, std::allocator<nc::core::likec::TreeNode const*> > const&)::{lambda(nc::gui::InspectorItem*)#1})",
	"qInstallMsgHandler(void (*)(QtMsgType, char const*))",
	"std::logic_error::what() const",

	"virtual __cdecl std::basic_iostream<char,struct std::char_traits<char> >::~basic_iostream<char,struct std::char_traits<char> >(void) __ptr64",

	"void __cdecl QNetworkSession::migrate(void) __ptr64",
	"void test1(void) const",

	"class QDnsDomainNameRecord & __thiscall QDnsDomainNameRecord::operator=(class QDnsDomainNameRecord &&)",
	"static void (__cdecl* QAbstractDeclarativeData::destroyed)(class QAbstractDeclarativeData *,class QObject *)",

	"float & __thiscall QMatrix4x4::operator()(int,int)",

	"int (__cdecl*__cdecl test3(void))(void)",
	"int (__cdecl*__cdecl _set_new_handler(int (__cdecl*)(unsigned int)))(unsigned int)",
	"void __cdecl ImgRegisterClass(char const *,class CWnd * (__stdcall*)(struct HWND__ *),char const *)",
	"void __thiscall CImgDialog::DrawControlsToImage(class CImage *,class CImgCtrl *,class CRect &,bool (__cdecl*)(class CImgCtrl *,class CImgCtrl *))",
	"int __thiscall Countries::PopulateCombo(class CImgCB &,enum Countries::INFOTYPE,bool (__stdcall*)(int))const",

	"void test2(float const (*const)[2])",
	"class QChar const * __cdecl QIPAddressUtils::parseIp6(unsigned char(&)[16], class QChar const *, class QChar const *)",
	"__thiscall LLA::operator struct LATLONALT const(void)const",
	
	//"void test1(float *)",
	"void __thiscall QOpenGLShaderProgram::setUniformValue(int,float const (* const)[2])",
	"__thiscall QTabletEvent::QTabletEvent(enum QEvent::Type, class QPointF const &,class QPointF const &, int,int, double, int, int, double,double, int, class QFlags<enum Qt::KeyboardModifier>, __int64, enum Qt::MouseButton, class QFlags<enum Qt::MouseButton>)",

	
	"bool __thiscall MissionDate::operator==(class MissionDate const &)const",
	"bool CIsSpace::operator()(char c) const",
	"static void __cdecl CAutoCursorArr::operator delete[](void *)",
	"void __cdecl operator delete(void *)",
	"bool __thiscall MissionDate::operator>(class MissionDate const &)const",
	
	"int __stdcall FileUtils::FileExists(char const *)",
	"class AIVec<class CElement *,100> & __thiscall CDynamicMission::GetElements(void)",
	"static class Countries * Countries::m_pCountries",
	"static int CCombatFSDlg::m_Country",
	"static struct AFX_MSGMAP const CImgTip::messageMap",
	"class CFSStr const & __cdecl CFSStr::VPrintf(int,...)",

	"bool __thiscall CLLAEdit::Lon(union ANGL48 &,char const *)",
	"virtual int __thiscall CImgMap::OnToolHitTest(class CPoint,struct tagTOOLINFOA *)const",
	"class LLA const & __thiscall CImgMap::GetDefaultViewPoint(void)const",
	"enum MapMode __thiscall CImgMap::SetMode(enum MapMode,enum CImgMap::MapSubMode)",
	"virtual __thiscall ofstream::~ofstream(void)",
	"void open(char const *,int,int)",
	"void __cdecl terminate(void)",
	"void AddMissionEvent(struct LATLONALT const &,char const *,enum _evlog_type,unsigned int,int)",
	"void __thiscall ofstream::open(char const *,int,int)",
	"class ostream & __thiscall ostream::write(char const *,int)",
	"static int const filebuf::openprot",
	nullptr
};

void SymParseTest()
{
	for (const char **p = test_str; *p; p++)
	{
		const char *p0 = *p;
		parser_t parser(*p);
		node_t *node = parser.evaluate();
		if (node)
		{
			SymParsePrintProto(node, std::cout);
			std::cout << std::endl;
			SymParsePrint(node, std::cout);
			//SymParsePrintFlat(node, std::cout);
			std::cout << std::endl;
		}
	}
}

#endif

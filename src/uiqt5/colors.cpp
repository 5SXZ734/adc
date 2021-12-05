#include <QtGui/QPainter>
#include "interface/IADCGui.h"
#include "colors.h"

using namespace adcui;

#ifdef _DEBUG
static const bool NO_COLORS  = 0;
#endif

ADCColorPair MapColorPair(uint index)//adjust FG color for special BG change
{
	switch (index)
	{
	case COLOR_DASM_BGND:
		return ADCColorPair(ColorFromId(index), Qt::black);
	case COLOR_DASM_BGND_EX:
		return ADCColorPair(ColorFromId(index), Qt::black);
	case COLOR_TASK_TOP:
		return ADCColorPair(ColorFromId(index), Qt::black);
	case COLOR_CUR_STUB:
		return ADCColorPair(ColorFromId(index), Qt::black);
	case adcui::COLOR_MARGIN_ERROR:
		return ADCColorPair(ColorFromId(index), Qt::white);
	default:
		break;
	}
	return ADCColorPair(ColorFromId(COLOR_DASM_BGND), Qt::black);//black on white
}

/*static Qt::GlobalColor fromColorTerm(Color_t iColor, Qt::GlobalColor &)//fore)
{
	switch (iColor)
	{
	case COLOR_TERM_DGREY: return Qt::darkGray;
	case COLOR_TERM_GREY: return Qt::gray;
	case COLOR_TERM_LGREY:
		//fore = Qt::black;
		return Qt::lightGray;
	case COLOR_TAG_ERROR: return Qt::red;
	case COLOR_TERM_GREEN: return Qt::green;
	case COLOR_TERM_BLUE: return Qt::blue;
	case COLOR_TERM_CYAN: return Qt::cyan;
	case COLOR_TERM_MAGENTA: return Qt::magenta;
	case COLOR_TERM_YELLOW: return Qt::yellow;
	case COLOR_TERM_DRED: return Qt::darkRed;
	case COLOR_TERM_DGREEN: return Qt::darkGreen;
	case COLOR_TERM_DBLUE: return Qt::darkBlue;
	case COLOR_TERM_DCYAN: return Qt::darkCyan;
	case COLOR_TERM_DMAGENTA: return Qt::darkMagenta;
	case COLOR_TERM_DYELLOW: return Qt::darkYellow;
	default:
		break;
	}
	return Qt::color0;
};*/

QColor ColorFromId(uint index)
{
	assert(index < COLOR__TOTAL);
	switch (index)
	{
	case COLOR_DASM_BGND:
		return QColor(255, 255, 255);
	case COLOR_DASM_BGND_EX:
		return QColor(224, 224, 224);
	case COLOR_DASM_ADDR_CODE:
		return QColor(128, 128, 0);
	case COLOR_DASM_ADDR_DATA:
		return QColor(128, 0, 0);
	case COLOR_DASM_ADDR_FUNC:
		return QColor(0, 0, 0);
	case COLOR_DASM_ADDR_UNK:
		return QColor(192, 192, 192);
	case COLOR_ARRAY:
		return QColor(Qt::darkBlue);
	case COLOR_ERROR:
		return QColor(Qt::red);
	case COLOR_DASM_ENDP2:
	case COLOR_DASM_ENDP:
	case COLOR_DASM_CODE:
		return QColor(0, 0, 128);
	case COLOR_DASM_NUMBER:
		return QColor(0, 128, 0);
//	case COLOR_DASM_OFFS:
//	case COLOR_DASM_RELOC:
//		return QColor(128, 128, 0);
//	case COLOR_DASM_RVA:
//		return QColor(0, 128, 192);
	case COLOR_WSTRING:
		//return QColor(255, 0, 0);
	case COLOR_DASM_ASCII:
		return QColor(128, 0, 0);
	case COLOR_DASM_DIRECTIVE:
		return QColor(0, 0, 255);
	case COLOR_DASM_UNK:
		return QColor(192, 192, 192);
//	case COLOR_DASM_ADDRESS:
//		return QColor(255, 128, 0);
	case COLOR_DASM_DATA_NAME:
		return QColor(0, 0, 128);
	case COLOR_DASM_CODE_NAME:
		return QColor(0, 0, 128);
	case COLOR_DASM_UNK_NAME:
		return QColor(128, 96, 128);
	case COLOR_DASM_COMMENT:
		return QColor(128, 128, 128);
	case COLOR_DASM_TYPE:
		return QColor(128, 0, 128);
	case COLOR_TREE:
		return QColor(Qt::gray);
	case COLOR_TREE_FUNC_DECOMPILED:
		return QColor(224, 176, 128);
	case COLOR_TREE_FUNC_BEING_DECOMPILED:
		return QColor(Qt::red);
	case COLOR_TREE_SHARED_TYPE:
		return QColor(160, 192, 248);
	case COLOR_TREE_STRUCVAR_TYPE:
		//return QColor(80, 240, 180);
		//return QColor(210, 240, 120);
		return QColor(176, 234, 166);
	case COLOR_DASM_BYTES:
		return QColor(0, 0, 255);
	case COLOR_UNKNOWN:
		return QColor(128, 128, 128);
	case COLOR_DASM_SEL:
		return QColor(255, 0, 0);
	case COLOR_DASM_SEL_AUX:
		return QColor(255, 128, 128);
	case COLOR_TASK_TOP:
		return QColor(255, 255, 215);//yellowish
	case COLOR_CUR_STUB:
		return QColor(242, 242, 242);
	case COLOR_MARGIN_ERROR:
		return QColor(Qt::red);
	case COLOR_KEYWORD:
		return QColor(Qt::blue);
	case COLOR_KEYWORD_EX:
		return QColor(128, 0, 255);
	case COLOR_PREPROCESSOR:
		return QColor(0, 0, 128);
	case COLOR_USER_FUNCTION:
		return QColor(128, 0, 128);
	case COLOR_IMPORT_REF:
		return QColor(128, 0, 0);
	case COLOR_EXPORT_REF:
		return QColor(255, 128, 255);
//		return QColor(0, 128, 128);
	case COLOR_EXPORTED:
		return QColor(255, 0, 255);
	case COLOR_COMMENT:
		return QColor(Qt::darkGreen);
	case COLOR_CALL_BREAK:
		return QColor(Qt::blue);
	case COLOR_FLOW_BREAK:
		return QColor(Qt::lightGray);
	case COLOR_FLOW_SPLIT:
		return QColor(Qt::lightGray);
	case COLOR_UNEXPLORED:
		return QColor(Qt::lightGray);
	case COLOR_UNNAMED:
		return QColor(Qt::darkGray);
	case COLOR_UNANALIZED:
		return QColor(Qt::darkGray);
	case COLOR_STRING:
		//return QColor(Qt::magenta);
		return QColor(163, 21, 21);
	case COLOR_DUP_SUFFIX:
//		return QColor(164, 164, 0);
//?		return QColor(0, 128, 192);
	case COLOR_XOUTS://bg
		return QColor(100, 181, 247);//blue
	case COLOR_XINS://bg
		return QColor(128, 187, 62);//green
	case COLOR_XDEPS://bg
		return QColor(228, 86, 86);//red
	case COLOR_SEL:
		return QColor(Qt::red);
	case COLOR_CUR://bg
//		return QColor(Qt::red);
//		return QColor(223, 249, 231);//greenish
		return QColor(246, 185, 77);//orange-like
//		return QColor(190, 240, 205);
	case COLOR_CUR_EXPR://bg
//		return QColor(Qt::yellow);
//		return QColor(255, 255, 224);//light yellow
		return (QColor(252, 234, 201));
//		return QColor(190, 240, 205);
		break;
	case COLOR_CUR_EDIT:
		return QColor(Qt::white);
	case COLOR_DASM_PROBE://in bin view - bg
		//return QColor(Qt::white);
		//return QColor(240, 240, 240);//Qt::lightGray);
		//return QColor(Qt::yellow);
		//return QColor(223, 249, 231);//greenish
		//return QColor(190, 240, 205);
		return QColor(246, 185, 77);//the same as for COLOR_CUR
//		return QColor(Qt::blue);
	case COLOR_SELAUX:
		return QColor(255,128,128);
	case COLOR_TAG_PROCESSING://bg
		return QColor(255, 128, 64);//orange
	case COLOR_TAG_INTRINSIC://bg
		return QColor(240, 220, 240);//pale magenta
//		return QColor(Qt::gray);
	case COLOR_TAG_NOPROTO:
	case COLOR_TAG_STUB://bg
		return QColor(250, 250, 250);//pale gray
	case COLOR_TAG_ERROR://bg
		return Qt::red;
	case COLOR_DASM_ADDRESS:
		return QColor(0, 128, 192);
	default:
		if (COLOR_ATTRIB_BEGIN <= index && index < COLOR_ATTRIB_END)
			return QColor(0, 128, 192);
		break;
	}
	return QColor(Qt::black);
}

void ColorFromId(QPainter *p, uint index, bool bNoBgnd, bool bHasSel)
{
#ifdef _DEBUG
	if (NO_COLORS)
		return;
#endif
	//if (index == 0)
		//return;
	/*if (p->font().underline())
	{
		QFont f(p->font());
		f.setUnderline(false);
		p->setFont(f);
	}*/
	//if (!(index < COLOR__TOTAL)) return;//?
	Q_ASSERT(index < COLOR__TOTAL);
	switch (index)
	{
	case COLOR_DASM_ENDP2:
	{
		QFont f(p->font());
		f.setItalic(true);
		p->setFont(f);
		break;
	}

	case COLOR_TREE:
		p->setPen(QPen(ColorFromId(index), 1, Qt::DotLine));
		return;

	case COLOR_CALL_BREAK:
	{
		p->setPen(QPen(ColorFromId(index), 1, Qt::DashLine));
		return;
	}
	case COLOR_FLOW_BREAK:
	{
		p->setPen(QPen(ColorFromId(index), 1, Qt::SolidLine));
		return;
	}
	case COLOR_FLOW_SPLIT:
	{
		p->setPen(QPen(ColorFromId(index), 1, Qt::DashLine));
		return;
	}

	case COLOR_DUP_SUFFIX:
		//p->setPen(ColorFromId(index)));
	{
		QFont f(p->font());
		f.setUnderline(true);
		p->setFont(f);
//?		p->setPen(ColorFromId(index)));
		return;
	}
	case COLOR_XOUTS:
	case COLOR_XINS:
	case COLOR_XDEPS:
		p->setPen(Qt::white);
		p->setBackgroundMode(Qt::OpaqueMode);
		p->setBackground(QBrush(ColorFromId(index)));
		return;

	case COLOR_CUR:
	case COLOR_CUR_EXPR:
	case COLOR_DASM_PROBE://in bin view
		if (!bHasSel)
		{
			p->setBackgroundMode(Qt::OpaqueMode);
			p->setBackground(QBrush(ColorFromId(index)));
		}
		return;//no foreground change

	case COLOR_CUR_EDIT:
		p->setBackgroundMode(Qt::OpaqueMode);
		p->setBackground(QBrush(Qt::darkGray));
		break;
	
	case COLOR_FONT_BOLD:
	{
		QFont f(p->font());
		f.setBold(true);
		p->setFont(f);
		return;
	}
	case COLOR_FONT_ITALIC:
	{
		QFont f(p->font());
		f.setItalic(true);
		p->setFont(f);
		return;
	}
	case COLOR_FONT_UNDERLINE:
	{
		QFont f(p->font());
		f.setUnderline(true);
		p->setFont(f);
		return;
	}

	case COLOR_TAG_PROCESSING:
	{
		if (!bNoBgnd)
		{
			p->setBackgroundMode(Qt::OpaqueMode);
			p->setBackground(QBrush(ColorFromId(index)));
			p->setPen(QColor(Qt::white));
		}
		return;
	}

	case COLOR_TAG_INTRINSIC:
	case COLOR_TAG_NOPROTO:
	case COLOR_TAG_STUB:
		if (!bNoBgnd)
		{
			p->setBackgroundMode(Qt::OpaqueMode);
			p->setBackground(QBrush(ColorFromId(index)));
			p->setPen(QColor(Qt::gray));
			return;
		}
		return;

	case COLOR_TAG_ERROR:
		//if (!bNoBgnd)
		{
			p->setBackgroundMode(Qt::OpaqueMode);
			p->setBackground(QBrush(ColorFromId(index)));
			p->setPen(QColor(Qt::white));
			return;
		}
		return;

	default:
/*		if (COLOR__TERM__BEGIN <= index && index < COLOR__TERM__END)
		{
			Qt::GlobalColor eFgnd(Qt::white);
			Qt::GlobalColor eBgnd(fromColorTerm((Color_t)index, eFgnd));
#if(1)
			p->setBackgroundMode(Qt::OpaqueMode);
			p->setPen(eFgnd);
			p->setBackground(QBrush(eBgnd));
#else
			QFont f(p->font());
			f.setUnderline(true);
			p->setFont(f);
			p->setPen(eBgnd);
#endif
			return;
		}
		else*/ if (COLOR_ATTRIB_BEGIN <= index && index < COLOR_ATTRIB_END)
		{
	case COLOR_DASM_ADDRESS:
			QFont f(p->font());
			f.setUnderline(true);
			p->setFont(f);
			break;
		}
		/*p->setBackgroundMode(Qt::TransparentMode);
		p->setBackground(QBrush(Qt::NoBrush));*/
	}
	p->setPen(ColorFromId(index));
}

/*QBrush MapBrush(int colorID)
{
	switch (colorID)
	{
	case COLOR_CUR_EDIT:
		return QBrush(Qt::darkGray);
	default:
		break;
	}
	return QBrush(Qt::NoBrush);
}*/

void MapFont::map(uint fontID)
{
	enum Font_t
	{
		FONT_DEFAULT,//pop previous font back
		FONT_ITALIC,
		FONT_BOLD,
		FONT_BOLD_ITALIC,
		//..
		FONT__TOTAL
	};

	QFont f(mpPainter->font());
	if (fontID == FONT_DEFAULT)
	{
		if (!empty())
			pop_back();
		if (!empty())
			fontID = back();
	}
	else
	{
		if (f.italic())
			fontID |= FONT_ITALIC;
		if (f.bold())
			fontID |= FONT_BOLD;
		push_back(fontID);
	}
	
	switch (fontID)
	{
	case FONT_ITALIC:
		f.setBold(false);
		f.setItalic(true);
		break;
	case FONT_BOLD:
		f.setBold(true);
		f.setItalic(false);
		break;
	case FONT_BOLD_ITALIC:
		f.setBold(true);
		f.setItalic(true);
		break;
	default:
	case FONT_DEFAULT:
		f.setBold(false);
		f.setItalic(false);
		break;
	}
	mpPainter->setFont(f);
}



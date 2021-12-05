#ifndef __COLORS_H__
#define __COLORS_H__

#include <QtCore/QList>
#include <QtGui/QColor>

class MapFont : public QList<uint>
{
	QPainter *mpPainter;
public:
	MapFont(QPainter *p)
		: mpPainter(p)
	{
	}
	void map(uint);
};

QColor ColorFromId(uint index);
void ColorFromId(QPainter *, uint, bool bNoBgnd, bool bHasSel);
typedef QPair<QColor,QColor>	ADCColorPair;//bgnd,fgnd
ADCColorPair MapColorPair(uint);

#endif//__COLORS_H__

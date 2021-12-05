
#include <math.h>

#include <QtCore/QEvent>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QAction>
#include <QDesktopWidget>
#include <QApplication>
#include <QMainWindow>
#include <QSizePolicy>
#include <QStyle>
#include <QToolButton>
#include <QToolTip>
#include <QWhatsThis>
#include <QDockWidget>


#include "toolpalette.h"

static const int DEFAULT_ROWS = 3;
static const int DEFAULT_COLS = 2;
static const int ALLOW_RESIZING = false;


/*!
\class QSilToolPalette
\brief Provides a movable, dockable two-dimentsional toolbar that can be resized.

A QSilToolPalette can only contain QToolButtons added with the addAction() method.
Resizing the QSilToolPalette will allow the user to change the dimensions of the widget and will snap
to a button-sized grid.
This kind of widget is useful to implement a "toolbox" as seen in programs such as Adobe Photoshop
Gimp and MS Word.
*/

class PaletteLayout : public QGridLayout
{
public :
	PaletteLayout(QSilToolPalette* parent, QLayout* parentLayout, int rows, int cols) :
	   QGridLayout(parentLayout->widget())//, rows, cols)
	   {
		   QSize unit_size = parent->unitSize();
		   mSizeHint =  QSize(cols * unit_size.height(), rows * unit_size.width());
	   }
	   virtual ~PaletteLayout(){}
protected :
	virtual QSize sizeHint() const { return mSizeHint; }
private:
	QSize mSizeHint;
};

/*!
\brief Creates an empty QSilToolPalette

\param parent QWidget parent.
\param name widget name.
*/
QSilToolPalette::QSilToolPalette(QMainWindow* parent, const char* name) :
QToolBar(name, parent),
	mpMainWindow(parent)
{
	init(DEFAULT_ROWS, DEFAULT_COLS);
}

/*!
\brief Creates an empty QSilToolPalette

\param label passed straight to QMainWindow::addDockWindow().
\param mainWindow QMainWindow that manages this tool palette.
\param parent QWidget parent.
\param newLine passed straight to QMainWindow::addDockWindow().
\param name widget name.
\param f widget flags.
*/
QSilToolPalette::QSilToolPalette(const QString& label,
	QMainWindow* mainWindow,
	QWidget* parent,
	bool,// newLine,
	const char*)// name,
	//Qt::WFlags)
	: QToolBar(label, parent),//mainWindow, parent, newLine, name, f),
	mpMainWindow(mainWindow)
{
	init(DEFAULT_ROWS, DEFAULT_COLS);
}


/*!
Destructor
*/
QSilToolPalette::~QSilToolPalette()
{
}

void QSilToolPalette::init(const int rows, const int cols)
{
	/*?    if (boxLayout())
	{
	boxLayout()->setAutoAdd(false);
	}*/

	mNumRows = rows;
	mNumCols = cols;
	mIsResizing = false;
#if 0
	mpLayout = new QGridLayout(boxLayout(), 
		rows , 
		cols , 
		-1,// space
		"special grid layout");
#endif
	/*?    mpLayout = new PaletteLayout(this,
	boxLayout(), 
	rows, 
	cols);


	mpLayout->setResizeMode(QLayout::Fixed);

	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, false));

	resize(unitSize().width() * cols, unitSize().height() * rows);

	// Need to monitor mouse movement
	setMouseTracking(true);*/
}


/*!
Deletes all the tool palette's child widgets.
*/
void QSilToolPalette::clear()
{
	QToolBar::clear();  // To delete all buttons
	mButtons.clear();
}

QSize QSilToolPalette::unitSize() const
{
	const QSize ICON_SIZE = mainWindow()->iconSize();//usesBigPixmaps() ? QSize(32, 32) : QSize(16, 16);
	//?	QIconSet::iconSize(QIconSet::Large) :
	//?	QIconSet::iconSize(QIconSet::Small);


	int spacing = 0;//style().pixelMetric(QStyle::PM_ToolBarItemSpacing) << 1;
	return ICON_SIZE + QSize(6 + spacing, 5 + spacing);
}

void QSilToolPalette::reorganize(const QSize& newSize, const QSize& oldSize)
{
	if ( (oldSize.width() < newSize.width()) ||
		(oldSize.height() > newSize.height()) )
	{
		mNumCols = qMax(1, newSize.width() / unitSize().width());
		mNumCols = qMin(mNumCols, mButtons.count());
		mNumRows = int( ceil(float(mButtons.count()) / mNumCols) );

	}
	else
	{
		mNumRows = qMax(1, newSize.height() / unitSize().height());
		mNumRows = qMin(mNumRows, mButtons.count());
		mNumCols = int( ceil(float(mButtons.count()) / mNumRows) );
	}

	//qDebug("Reorganise %d, %d", mNumRows, mNumCols);

	// Re-add all widgets to the layout
	QWidgetList::Iterator itr(mButtons.begin());
	//int index = 0;
	//QWidget* w = 0;

	delete mpLayout;
	layout()->invalidate();
#if 0
	mpLayout = new QGridLayout(boxLayout(), 
		mNumRows /* rows */, 
		mNumCols /* cols */, 
		0 /* space */,
		"special grid layout");
#endif
	/*?    mpLayout = new PaletteLayout(this,
	boxLayout(), 
	mNumRows, 
	mNumCols);

	mpLayout->setResizeMode(QLayout::Fixed);

	while (w = *itr)
	{
	int col = index % mNumCols;
	int row = index / mNumCols;
	mpLayout->addWidget(w, row, col);

	//qDebug("Toolbutton size hint : %d, %d", w->sizeHint().width(), w->sizeHint().height());
	//qDebug("Toolbutton size : %d, %d", w->size().width(), w->size().height());
	++itr;
	index++;
	}
	layout()->activate();*/
}

/* DON'T DOCUMENT
Intercept and override events before QToolBar starts its shite!
*/
bool QSilToolPalette::event(QEvent* e)
{
	if (e->type() == QEvent::ChildAdded)
	{
		QChildEvent* child_event = dynamic_cast<QChildEvent*>(e);

		QToolButton* tool_button = dynamic_cast<QToolButton*>(child_event->child());
		if (tool_button)
		{
			tool_button->setMouseTracking(true);

			// Monitor mouse entering button
			tool_button->installEventFilter(this);

			mButtons.append(tool_button);
			//tool_button->show();

			// Layout the buttons
			if (isVisible())
			{
				reorganize(size(), size());
			}

			return true;
		}
	}
	else if (e->type() == QEvent::ChildRemoved)
	{
		QChildEvent* child_event = dynamic_cast<QChildEvent*>(e);

		QToolButton* tool_button = dynamic_cast<QToolButton*>(child_event->child());
		if (tool_button)
		{
			(void) mButtons.removeOne(tool_button);

			// Layout the remaining buttons
			if (isVisible())
			{
				reorganize(size(), size());
			}
		}

		return true;
	}
	return QToolBar::event(e);
}


QSilToolPalette::DragZone QSilToolPalette::dragZone(const QPoint mousePos)
{
	static const int DRAG_MARGIN = qApp->startDragDistance();

	// Change if mouse is over side/corner
	if ( (mousePos.x() > (width() - DRAG_MARGIN)) && 
		(mousePos.y() > (height() - DRAG_MARGIN)) )
	{
		return BottomRight;
	}
	else if ( mousePos.x() > (width() - DRAG_MARGIN) )
	{
		return Right;
	}
	else if ( mousePos.y() > (height() - DRAG_MARGIN) )
	{
		return Bottom;
	}
	else
	{
		return None;
	}
}

void QSilToolPalette::mouseReleaseEvent(QMouseEvent* e)
{
	if (Qt::LeftButton == e->button())
	{
		if (mIsResizing)
		{
			// erase last rubberband
			if (!mOldSize.isNull())
			{
				QRect rect;
				// Set top left to window position
				rect.setTopLeft(pos() + QPoint(1, 1));
				// Draw rubber band
				QPainter p(qApp->desktop()->screen());//? , true);
				/*?		p.setRasterOp(Qt::NotROP);
				QPen pen;
				pen.setWidth(3);
				p.setPen(pen);
				rect.setSize(mOldSize);
				rect.rRight() += (margin() << 1);
				rect.rBottom() += QStyle::PM_DockWindowFrameWidth + (margin() << 1);
				p.drawRect(rect);*/
			}

			QSize new_size = mOldSize;
			QSize old_size = size();

			reorganize(new_size, old_size);
			resize(sizeHint());
		}
	}

	mIsResizing = false;
	mOldSize = QSize();

	// Call inherited
	QToolBar::mouseReleaseEvent(e);
}

void QSilToolPalette::mouseMoveEvent(QMouseEvent* e)
{
	static const int DRAG_MARGIN = qApp->startDragDistance();

/*	if (ALLOW_RESIZING)
	{
		if (!mIsResizing && QDockWidget::OutsideDock == place())
		{
			DragZone zone = dragZone(e->pos());

			// Change cursor if mouse is over side/corner
			if (BottomRight == zone)
			{
				setCursor(Qt::SizeFDiagCursor);
			}
			else if (Right == zone)
			{
				setCursor(Qt::SizeHorCursor);
			}
			else if (Bottom == zone)
			{
				setCursor(Qt::SizeVerCursor);
			}
			else
			{
				unsetCursor();
			}
		} 
		if (!mIsResizing && (Qt::LeftButton & e->state()))
		{
			mIsResizing = (None != dragZone(e->pos()));
			mOldSize = QSize();
		}
		if (mIsResizing)
		{
			// Draw rubber band
			QPainter p(qApp->desktop()->screen());//, true);
			//?	    p.setRasterOp(Qt::NotROP);
			QPen pen;
			pen.setWidth(3);
			p.setPen(pen);

			QRect rect;
			// Set top left to window position
			rect.setTopLeft(pos() + QPoint(1, 1));


			QSize new_size(e->x(), e->y());

			if ( (mOldSize.width() < new_size.width()) ||
				(mOldSize.height() > new_size.height()) )
			{
				int num_cols = QMAX(1, new_size.width() / unitSize().width());
				num_cols = QMIN(num_cols, mButtons.count());
				new_size.rwidth() = num_cols * unitSize().width();

				int num_rows = int( ceil(float(mButtons.count()) / num_cols) );
				new_size.rheight() = num_rows * unitSize().height();

			}
			else
			{
				int num_rows = QMAX(1, new_size.height() / unitSize().height());
				num_rows = QMIN(num_rows, mButtons.count());
				new_size.rheight() = num_rows * unitSize().height();

				int num_cols = int( ceil(float(mButtons.count()) / num_rows) );
				new_size.rwidth() = num_cols * unitSize().width();
			}
			//new_size.rheight() += QStyle::PM_DockWindowFrameWidth; 

			if (mOldSize != new_size)
			{
				// erase last rubberband
				if (!mOldSize.isNull())
				{
					rect.setSize(mOldSize);
					rect.rRight() += (margin() << 1);
					rect.rBottom() += QStyle::PM_DockWindowFrameWidth + (margin() << 1);
					p.drawRect(rect);
				}
				// draw new rubberband
				rect.setSize(new_size);
				rect.rRight() += (margin() << 1);
				rect.rBottom() += QStyle::PM_DockWindowFrameWidth + (margin() << 1);
				p.drawRect(rect);
			}
			// Store for next call
			mOldSize = new_size;
		}
	}*/

	// Call inherited
	QToolBar::mouseMoveEvent(e);
}

bool QSilToolPalette::eventFilter(QObject* o, QEvent* e)
{
/*?	if (Q3DockWindow::OutsideDock == place())
	{
		// Unset cursor in case it is still set to a resize cursor
		if (QEvent::Enter == e->type() && dynamic_cast<QToolButton*>(o))
		{
			if (None == dragZone(mapFromGlobal(QCursor::pos())))
				unsetCursor();
		}
	}*/
	// Call inherited
	return QToolBar::eventFilter(o, e);
}


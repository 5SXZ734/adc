#ifndef QSIL_TOOLBAR_H
#define QSIL_TOOLBAR_H

#include <QToolBar>
#include <QLayout>
#include <QtGui/QWidgetList>

//#include <qsil/qsilglobal.h>

class Q3Action;
class QChildEvent;
class QMainWindow;
class QResizeEvent;
class QToolButton;
class QTimerEvent;

class QSilToolPalette : public QToolBar
{
Q_OBJECT
public:
    QSilToolPalette(QMainWindow* parent = 0, const char* name = 0);
	QSilToolPalette(const QString& label,
		QMainWindow* mainWindow,
		QWidget* parent,
		bool newLine = false,
		const char* name = 0);
		//Qt::WFlags f = 0);
    virtual ~QSilToolPalette();

    QMainWindow* mainWindow() const { return mpMainWindow; }
    virtual void clear();

    // FOR INTERNAL USE ONLY
    QSize unitSize() const;
    QWidgetList& buttons() { return mButtons; }
    // END FOR INTERNAL USE ONLY

protected:
    virtual bool event(QEvent*);

    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);

    virtual bool eventFilter(QObject*, QEvent*);
private:
    friend class ToolPaletteEditorAdapter;
    
    enum DragZone {None, Bottom, Right, BottomRight};

    void init(const int rows, const int cols);
    
    QGridLayout* gridLayout() const { return mpLayout; }
    void reorganize(const QSize& newSize, const QSize& oldSize);
    DragZone dragZone(const QPoint mousePos);

    QGridLayout* mpLayout;
    int mNumRows;
    int mNumCols;  
    bool mIsResizing;  
    QMainWindow* mpMainWindow;
    QWidgetList mButtons;
    QSize mOldSize;
  
    // Disabled copy constructor and operator=
    QSilToolPalette(const QSilToolPalette&);
    QSilToolPalette& operator=(const QSilToolPalette&);
};

#endif

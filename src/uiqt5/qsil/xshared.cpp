#include <time.h>

#include <assert.h>

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QPointer>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#if QT_VERSION_MAJOR < 6
#include <QtCore/QRegExp>
#else
#include <QRegularExpression>
#endif
#include <QtCore/QObjectList>
#include <QtGui/QWidgetList>
#include <QtGui/QKeySequence>
#include <QLayout>
#include <QApplication>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

#include "xshared.h"
#include "toolpalette.h"

// Accels in keystring are delimeted by a comma.  This regexp filters out
// the special cases
//const QRegExp QSil::ACCEL_DELIMETER = QRegExp("(\\+{2}|.[^\\+])\\,");
const QString QSil::ACCEL_DELIMETER = QString(", ");

// Enumeration of all possible dock areas for a dock window.
/*const Qt::Dock QSil::DOCK_PLACES[PLACE_MAX] = {
Qt::DockTop,
Qt::DockBottom,
Qt::DockLeft,
Qt::DockRight,
Qt::DockMinimized,
Qt::DockTornOff
};*/

static const char ACTION_BUTTON_SUFFIX[] = "_action_button";


/*class MyHackSignal : public Q3Signal
{
public :
QConnectionList *receivers( const char* signal ) const {
return QObject::receivers(signal);
}
};*/


/*?QAction* QSil::locateActionByMenuData(
	QMainWindow*,
	const QMenu* menu,
	const int id)
{
	// Establish if action is connected to this menu item
	// This uses Qt internal stuff and is not future-proof
	// BE WARNED!
	QMenuItem* item = menu->findItem(id);

	if (!item) 
		return 0;
	    QSignal* signal = item->signal();
	if (!signal)
	return 0;

	MyHackSignal* hack_signal= static_cast<MyHackSignal*>(signal);

	QConnectionList* recvrs = hack_signal->receivers(SIGNAL(signal( const QVariant& )));

	if (!recvrs)
	return 0;

	QAction* result = 0;

	for (QConnection* con = recvrs->first(); con != 0; con = recvrs->next())
	{
	result = dynamic_cast<QAction*>(con->object());
	if (result)
	break;
	}

	return result;
}*/

/*DON'T DOCUMENT 
\brief Find a specific QAction by its name - better than locateActionByLabel(), since names are unique.

\param mainWindow the QMainWindow parent to search.
\actionName textual name of action.

\returns the first QAction found with a matching name.
*/
/*QAction* QSil::locateActionByName(
	QMainWindow* mainWindow,
	const QString& actionName)
{
	// Maps action names to respective actions.  Assumes names are unique, since actions with the same
	// names will not be entered into the map
	typedef QMap<QString, QPointer<QAction> > ActionDict;

	static QMap<QMainWindow*, ActionDict> sActionMap;

	ActionDict& action_dict = sActionMap[mainWindow];
	QPointer<QAction> result = action_dict[actionName];
	if (!result)
	{
		result = dynamic_cast<QAction*>(mainWindow->child(actionName.local8Bit(), "QAction"));

		// Store action
		if (result)
			action_dict.insert(actionName, result);
	}
	return result;
}*/

/*QWidget* QSil::locateWidgetByName(
	QMainWindow* mainWindow,
	const QString& widgetName)
{
	// Maps widget names to respective widgets.  Assumes names are unique, since widgets with the same
	// names will not be entered into the map
	typedef QMap<QString, QPointer<QWidget> > WidgetDict;

	static QMap<QMainWindow*, WidgetDict> sWidgetMap;

	WidgetDict& widget_dict = sWidgetMap[mainWindow];
	QPointer<QWidget> result = widget_dict[widgetName];
	if (!result)
	{
		result = dynamic_cast<QWidget*>(mainWindow->child(widgetName.local8Bit(), "QWidget"));

		// Store widget
		if (result)
			widget_dict.insert(widgetName, result);
	}
	return result;
}*/


//typedef QToolBar QSilToolPalette;
QStringList QSil::getToolbarContents(QToolBar* toolBar)
{
	QStringList toolbar_contents;

	// See if we have a tool palette or a toolbar
	QSilToolPalette* tool_palette = qobject_cast<QSilToolPalette*>(toolBar);

	if (!tool_palette)
	{
		/*?		QList<QWidget *> children = toolBar->findChildren<QWidget *>(0);//? // name ,
		//?						   true  match regexp , 
		//?						   false  recurse );
		//Fill list with nulls
		for (int n = 0; n < children.count(); n++)
		toolbar_contents.append(QString());

		int i = 0;

		for (QList<QWidget*>::iterator it = children.begin(); it != children.end(); it++)
		{
		QWidget *child(*it);
		int index = toolBar->boxLayout()->findWidget(child);

		// Before window is shown for first time, index is always -1,
		// so change it to be the current iteration no.
		if (-1 == index)
		index = i;
		i++;
		bool is_internal_widget = QString(child->name()).startsWith("qt_");

		if (child->isA("QToolButton"))
		{
		QToolButton* tool_button = dynamic_cast<QToolButton*>(child);

		if (!is_internal_widget)
		{
		QString action_name(tool_button->name());
		// We know that toolbutton has same name as action, once this suffix is removed
		action_name.remove(ACTION_BUTTON_SUFFIX);
		toolbar_contents[index] = action_name;
		}
		}
		else if (child->isA("QToolBarSeparator"))
		{
		toolbar_contents[index] = QString("|");
		}
		else
		{
		if (!is_internal_widget
		|| QString(child->name()).contains("actiongroup"))
		{
		// A combobox, or some other crap that user has put in the toolbar
		toolbar_contents[index] = child->name();
		}
		}
		}
		//Remove any remaining nulls
		toolbar_contents.remove(QString());*/

		// Delete list	
		//?delete children;
	}
	else
	{
		//assert(0);
		/*	QWidgetList buttons = tool_palette->buttons();
		for (QWidget* w = buttons.first(); w; w = buttons.next())
		{
		toolbar_contents.append(QString(w->name()).remove(ACTION_BUTTON_SUFFIX));
		}*/
	}

	return toolbar_contents;
}

void QSil::setToolbarContents(QToolBar* /*toolBar*/,
	QMainWindow* /*window*/,
	const QStringList& /*contents*/)
{
	// Remove any toolbuttons from the toolbar, store any other widgets for 'later'
/*?	static Q3Dict<QWidget> tool_widgets;

	QList<QWidget *> children = toolBar->findChildren<QWidget *>(0);//? name ,
	//? true // match regexp , 
	//? false // recurse );
	for (QList<QWidget*>::iterator it(children.begin()); it != children.end(); it++) 
	{
		QWidget *widget(*it);
		QToolButton* tool_button = dynamic_cast<QToolButton*>(widget);

		bool is_separator = widget->isA("QToolBarSeparator");
		bool is_internal_widget = QString(widget->name()).startsWith("qt_");

		if ( (!tool_button && !is_separator && !is_internal_widget)
			|| QString(widget->name()).contains("actiongroup") )
		{
			// hide
			//widget->hide();
			// Remove from layout
			//box_layout->remove(widget);
			// Store in dict
			tool_widgets.insert(widget->name(), widget);
			widget->reparent(window, Qt::WType_TopLevel, QPoint(), false);// Takes it out of the toolbar properly
		}
	}
	// Deallocate list
	//?    delete children;
	// Delete tool buttons and separators, they will be re-added if required
	toolBar->clear();


	int insert_pos = 0;
	//Repopulate based on loaded values
	for (QStringList::const_iterator it = contents.begin();
		it != contents.end();
		it++)
	{
		if ("|" == *it)
		{
			toolBar->addSeparator();
		}
		else
		{
			QAction* action = QSil::locateActionByName(window, *it);
			if (action)
			{
				action->addTo(toolBar);
			}
			else
			{
				//QWidget* widget = tool_widgets[*it];
				QWidget* widget = QSil::locateWidgetByName(window, *it);
				if (widget)
				{
					widget->reparent(toolBar, 0, QPoint(), true);// Puts it back in the toolbar
					//box_layout->insertWidget(insert_pos, widget); // 'insert' not required
					//widget->show();
				}
			}
		}
		insert_pos++;
	}
	toolBar->updateGeometry();*/
}


QString QSil::shortenFilePath(const QString& filePath)
{
	QString result = filePath;

	QChar sep = QDir::separator();

	if (result.count(sep) < 5)// nothing to do
		return result;

	//strip_pattern = QDir::toNativeSeparators(strip_pattern);
	int start_pos = result.indexOf(sep);
	if (start_pos > -1)
		start_pos = result.indexOf(sep, start_pos + 1);
	// handle Win32 UNC path names
#ifdef WIN32
	if (1 == start_pos)
		start_pos = result.indexOf(sep, start_pos + 1);
#endif

	int end_pos = result.lastIndexOf(sep);
	if (end_pos > -1)
		end_pos = result.lastIndexOf(sep, end_pos - 1);

	if ((start_pos > -1) && (end_pos > -1))
	{
		// Remove matched string (leave separators)
		(void)result.replace(start_pos + 1, end_pos - start_pos - 1, "...");
	}

	return result;
}



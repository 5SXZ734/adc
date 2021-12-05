
#include "ADCUtils.h"
#include <QSpinBox>

const char *toIconStr(uint u)
{
	switch (u)
	{
	case adcui::FOLDERTYPE_FILE_CPP:
		return ":c++.png";// ":page_white_cplusplus.png";
	case adcui::FOLDERTYPE_FILE_C:
		return ":c.png";
	case adcui::FOLDERTYPE_FILE_H:
		return ":h.png";// ":page_white_h.png";
	case adcui::FOLDERTYPE_FILE_RC:
		return ":resources.png";
	case adcui::FOLDERTYPE_FILE_T:
		return ":document_types.png";
	case adcui::FOLDERTYPE_FILE_N:
		return ":document_names.png";
	case adcui::FOLDERTYPE_FILE_E:
		return ":doc_export_16.png";
	case adcui::FOLDERTYPE_FILE_I:
		return ":doc_import_16.png";
	case adcui::FOLDERTYPE_FILE_TT:
		return ":type_template.png";//return ":document_templates.png";
	case adcui::FOLDERTYPE_FILE_STUB:
		return ":document_stubs.png";
	case adcui::FOLDERTYPE_BINARY_EXE:
		return ":application_green.png";
	case adcui::FOLDERTYPE_BINARY_DLL:
		return ":application_link.png";
	case adcui::FOLDERTYPE_BINARY_PHANTOM:
		return ":application_phantom.png";
	case adcui::FOLDERTYPE_FOLDER:
		return ":std_folder_closed_16.png";
	default:
		break;
	}
	return ":dasm.16.png";// ":binary.png";
}

const char *toFolderIconStr(bool bOpened)
{
	if (bOpened)
		return ":std_folder_opened_16.png";
	return ":std_folder_closed_16.png";
}

const char *toIconStr(QString s)
{
	s = s.toLower();
	if (s.endsWith(SOURCE_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_CPP);
	if (s.endsWith(SOURCE_C_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_C);
	if (s.endsWith(HEADER_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_H);
	if (s.endsWith(RESOURCE_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_RC);
	if (s.endsWith(TYPES_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_T);
	if (s.endsWith(NAMES_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_N);
	if (s.endsWith(EXPORTS_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_E);
	if (s.endsWith(IMPORTS_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_I);
	if (s.endsWith(TEMPLATES_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_TT);
	if (s.endsWith(STUBS_EXT))
		return toIconStr(adcui::FOLDERTYPE_FILE_STUB);
	const QByteArray cs(MODULE_SEP);
	if (s.endsWith(cs))
		s.truncate(s.length() - cs.length());
	if (s.endsWith(".exe"))
		return toIconStr(adcui::FOLDERTYPE_BINARY_EXE);
	if (s.endsWith(".dll"))
		return toIconStr(adcui::FOLDERTYPE_BINARY_DLL);
	return toIconStr(adcui::FOLDERTYPE_UNK);
}


////////////////////////////////////////////////////////////
// ADCWinToolbar


#include <QToolButton>
#include <QComboBox>
#include <QLayout>

ADCWinToolbar::ADCWinToolbar(QWidget *parent)
	: QFrame(parent)
{
	//setFrameStyle(QFrame::Panel | QFrame::Raised);

	m_hbox.setContentsMargins(3, 3, 3, 3);
	m_hbox.setSpacing(0);
	setLayout(&m_hbox);
}

QToolButton *ADCWinToolbar::addButton(QAction *pAction)
{
	QToolButton *pTbtn = new QToolButton(this);
	pTbtn->setAutoRaise(true);
	if (pAction)
		pTbtn->setDefaultAction(pAction);
	m_hbox.addWidget(pTbtn);
	return pTbtn;
}

QToolButton *ADCWinToolbar::addButton(const QIcon &icon, const QString &text)
{
	QToolButton *pTbtn = new QToolButton(this);
	pTbtn->setAutoRaise(true);
	pTbtn->setText(text);
	pTbtn->setToolTip(pTbtn->text());
	pTbtn->setIcon(icon);
	m_hbox.addWidget(pTbtn);
	return pTbtn;
}

void ADCWinToolbar::addStretch()
{
	m_hbox.addStretch();
}

void ADCWinToolbar::addSeparator()
{
	QFrame *p(new QFrame(this));
	p->setFrameStyle(QFrame::VLine | QFrame::Sunken);
	m_hbox.addWidget(p);
}

QComboBox *ADCWinToolbar::addComboBox()
{
	QComboBox *p(new QComboBox(this));
	m_hbox.addWidget(p);
	return p;
}

QSpinBox* ADCWinToolbar::addSpinBox()
{
	QSpinBox* p(new QSpinBox(this));
	p->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	p->resize(QSize(30, p->height()));
	p->setRange(1, INT_MAX);
	p->setValue(1);
	m_hbox.addWidget(p);
	return p;
}

static void clearLayout(QLayout* layout, bool deleteWidgets = true)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void ADCWinToolbar::cleanup()
{
	clearLayout(&m_hbox);
}



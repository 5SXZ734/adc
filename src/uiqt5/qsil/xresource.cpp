#include <QtCore/QSettings>
#include <QtCore/QObject>

#include "xresource.h"

void QSilResourceBase::reset(QSettings* settings)
{
	// Remove the entry from the file/registry
	settings->remove(key());   
}


QString QSilResourceBase::typeName()
{
	switch (type())
	{
	case BOOL:
		return "Boolean";
	case INT:
		return "Number (integer)";
	case INT64:
		return "Number (64-bit integer)";
	case DOUBLE:
		return "Number (floating point)";
	case STRING:
		return "Text";
	case STRINGLIST:
		return "Text list";
	case COLOR:
		return "Color";
	case FONT: 
		return "Font";
	default:
		return "Custom";
	}
}

// QSilBoolResource
void QSilBoolResource::read(QSettings* settings)
{
	setValue(settings->value(key(), value()).toBool());
}

void QSilBoolResource::write(QSettings* settings) const
{
	settings->setValue(key(), value());
}

QString QSilBoolResource::toString() const
{
	return value() ? "true" : "false";
}


// QSilIntResource
void QSilIntResource::read(QSettings* settings)
{
	setValue(settings->value(key(), value()).toInt());
}

void QSilIntResource::write(QSettings* settings) const
{
	settings->setValue(key(), value());
}

QString QSilIntResource::toString() const
{
	return QString::number(value());
}

// QSilDoubleResource
void QSilDoubleResource::read(QSettings* settings)
{
	setValue(settings->value(key(), value()).toDouble());
}

void QSilDoubleResource::write(QSettings* settings) const
{
	settings->setValue(key(), value());
}

QString QSilDoubleResource::toString() const
{
	return QString::number(value());
}

// QSilStringResource
QString QSilStringResource::toString() const
{
	return value();
}

void QSilStringResource::fromString(const QString& rString)
{
	setValue(rString);
}

// QSilStringListResource
void QSilStringListResource::read(QSettings* settings)
{
	bool ok(settings->contains(key()));
	QStringList val = settings->value(key()).toStringList();
	if (ok)
		setValue(val);
}

void QSilStringListResource::write(QSettings* settings) const
{
	settings->setValue(key(), value());
}

QString QSilStringListResource::toString() const
{
	return value().join(",");
}

// QSilColorResource
QString QSilColorResource::toString() const
{
	QColor color = value();
	return QString("%1,%2,%3").
		arg(color.red()).
		arg(color.green()).
		arg(color.blue());
}

void QSilColorResource::fromString(const QString& rString)
{
	QStringList vals(rString.split(',', QString::SkipEmptyParts));

	QColor color;
	if (vals.count() > 2)
	{
		int r = vals[0].toInt();
		int g = vals[1].toInt();
		int b = vals[2].toInt();
		color.setRgb(r, g, b);
	}

	if (color.isValid())
		setValue(color);
}

// QSilFontResource
QString QSilFontResource::toString() const
{
	return value().toString();
}

void QSilFontResource::fromString(const QString& rString)
{
	QFont font;
	if (font.fromString(rString))
		setValue(font);
}





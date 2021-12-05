#ifndef _QSIL_RESOURCE_H_
#define _QSIL_RESOURCE_H_


#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "xshared.h"
#include "xresmgr.h"


class QSettings;

// QSilResourceBase - Base of all resources

class QSilResourceBase : public QObject
{
    Q_OBJECT
public:
    enum ResourceType
    {
	UNDEFINED,
    	BOOL,
    	INT,
    	INT64,
    	DOUBLE,
    	STRING,
    	STRINGLIST,
    	COLOR,
    	FONT,  
    	CUSTOM  
    };
    virtual ~QSilResourceBase(){}
    QString key() const { return mKey; }
    
    virtual void read(QSettings*) = 0;
    virtual void write(QSettings*) const = 0;
    virtual QString toString() const = 0;
    virtual bool isDirty() const = 0;
    virtual void reset(QSettings*);
    virtual int type(){ return UNDEFINED; }
    virtual QString typeName();
    virtual void resetDirty() = 0;
    bool isVersioned() const { return mVersioned; }
    
    void setDisplayName(const QString& rDisplayName){ mDisplayName = rDisplayName; }
    QString displayName() const { return mDisplayName; }
protected:
    QSilResourceBase(const QString& keyName, const bool versioned = false) :
	mKey(keyName),
        mDisplayName(keyName),
	mVersioned(versioned)
    {
    }
    virtual void fromString(const QString&) = 0;
private:
    QSIL_DISABLE_COPY(QSilResourceBase)
            
    QString mKey;
    QString mDisplayName;
    const bool mVersioned;
};

// QSilResource - Template class to allow type-specialisation
	
template <class TType>
class QSilResource : public QSilResourceBase
{
public:
    virtual ~QSilResource(){}

    void setValue(const TType& val);
    TType value() const { return mValue; }

    virtual void valueChanged(const TType&) = 0;
protected:
    QSilResource(const QString& keyName,
		 const TType& defaultVal,
		 const bool versioned = false); 

    virtual void read(QSettings*);
    virtual void write(QSettings*) const;
private:
    QSIL_DISABLE_COPY(QSilResource)
        
    virtual bool isDirty() const { return mIsDirty; }
    void resetDirty() {  mIsDirty = false; }
    void reset(QSettings*);// Make private

    TType mDefault;
    TType mValue;
    bool mIsDirty;
};

template <class TType>
inline void QSilResource<TType>::setValue(const TType& val)
{ 
    if (mValue != val)
    {
	 mValue = val;
	 mIsDirty = true;// Mark as "dirty"
	 // emit signal 
	 emit valueChanged(value());
    }
}

template <class TType>
inline void QSilResource<TType>::reset(QSettings* settings)
{ 
    setValue(mDefault);
    mIsDirty = false;
    // Call inherited
    QSilResourceBase::reset(settings);
}
	
template <class TType>
inline QSilResource<TType>::QSilResource(const QString& keyName, 
				  const TType& defaultVal,
				  const bool versioned) :
	QSilResourceBase(keyName, versioned),
	mDefault(defaultVal),
	mValue(defaultVal), 
	mIsDirty(false)
{
}

template <class TType>
inline void QSilResource<TType>::read(QSettings* settings)
{
    // Default implementation, just uses the string value
    fromString(settings->value(key(), toString()).toString());
}

template <class TType>
inline void QSilResource<TType>::write(QSettings* settings) const
{
    // Default implementation, just uses the string value
    (void) settings->setValue(key(), toString());
}


//  All the remaining 'stock' resource types - specialisations of QSilResource


class QSilBoolResource : public QSilResource<bool>  
{
    Q_OBJECT
public:
    QSilBoolResource(const QString& keyName, 
		     const bool defaultVal = false,
		     const bool versioned = false) :
	QSilResource<bool>(keyName, defaultVal, versioned){}
    virtual int type(){ return BOOL; }
signals:
    void valueChanged(const bool&);
protected:
    virtual void read(QSettings*);
    virtual void write(QSettings*) const;
    virtual QString toString() const;
    virtual void fromString(const QString& rString){ Q_UNUSED(rString) }
private:
    QSIL_DISABLE_COPY(QSilBoolResource)
};

class QSilIntResource : public QSilResource<int> 
{
    Q_OBJECT
public:
    QSilIntResource(const QString& keyName, 
		    const int defaultVal = 0,
		    const bool versioned = false) :
	QSilResource<int>(keyName, defaultVal, versioned){}
    virtual int type(){ return INT; }
signals:
    void valueChanged(const int&);
protected:
    virtual void read(QSettings*);
    virtual void write(QSettings*) const;
    virtual QString toString() const;
    virtual void fromString(const QString& rString){ Q_UNUSED(rString) }
private:
    QSIL_DISABLE_COPY(QSilIntResource)
};

class QSilDoubleResource : public QSilResource<double> 
{
    Q_OBJECT
public:
    QSilDoubleResource(const QString& keyName, 
		       const double defaultVal = 0.0,
		       const bool versioned = false) :
	QSilResource<double>(keyName, defaultVal, versioned){}
    virtual int type(){ return DOUBLE; }
signals:
    void valueChanged(const double&);
protected:
    virtual void read(QSettings*);
    virtual void write(QSettings*) const;
    virtual QString toString() const;
    virtual void fromString(const QString& rString){ Q_UNUSED(rString) }
private:
    QSIL_DISABLE_COPY(QSilDoubleResource)
};

class QSilStringResource : public QSilResource<QString>
{
    Q_OBJECT
public:
    QSilStringResource(const QString& keyName, 
		       const QString& defaultVal = QString(),
		       const bool versioned = false) :
	QSilResource<QString>(keyName, defaultVal, versioned){}
    virtual int type(){ return STRING; }
signals:
    void valueChanged(const QString&);
protected:
    virtual QString toString() const;
    virtual void fromString(const QString&);
private:
    QSIL_DISABLE_COPY(QSilStringResource)
};

class QSilStringListResource : public QSilResource<QStringList>
{
    Q_OBJECT
public:
    QSilStringListResource(const QString& keyName, 
			   const QStringList& defaultVal = QStringList(),
			   const bool versioned = false) :
	QSilResource<QStringList>(keyName, defaultVal, versioned){}
    virtual int type(){ return STRINGLIST; }
signals:
    void valueChanged(const QStringList&);
protected:
    virtual void read(QSettings*);
    virtual void write(QSettings*) const;
    virtual QString toString() const;
    virtual void fromString(const QString& rString){ Q_UNUSED(rString) }
private:
    QSIL_DISABLE_COPY(QSilStringListResource)
};

class QSilColorResource : public QSilResource<QColor>
{
    Q_OBJECT
public:
    QSilColorResource(const QString& keyName, 
		      const QColor& defaultVal = qRgba(0, 0, 0, 255),
		      const bool versioned = false) :
	QSilResource<QColor>(keyName, defaultVal, versioned){}
    virtual int type(){ return COLOR; }
signals:
    void valueChanged(const QColor&);
protected:
    virtual QString toString() const;
    virtual void fromString(const QString&);
private:
    QSIL_DISABLE_COPY(QSilColorResource)
};

class QSilFontResource : public QSilResource<QFont>  
{
    Q_OBJECT
public:
    QSilFontResource(const QString& keyName, 
		     const QFont& defaultVal = QFont(),
		     const bool versioned = false) :
	QSilResource<QFont>(keyName, defaultVal, versioned){}
    virtual int type(){ return FONT; }
signals:
    void valueChanged(const QFont&);
protected:
    virtual QString toString() const;
    virtual void fromString(const QString&);
private:
    QSIL_DISABLE_COPY(QSilFontResource)
};



#endif /* _QSIL_RESOURCE_H_ */

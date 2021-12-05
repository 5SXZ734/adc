/* -*- C++ -*- -------------------------------------------------------------
 * Description: SignalMultiplexer - Taken From Qt Quarterley 8.
 * Signal Multiplexer class will allow us to easily connect and disconnect
 * senders from receivers (in this case, documents from main window actions).  
 * We need this to make the code cleaner and allow only the selected document
 * to respond to user events.
 * -------------------------------------------------------------------------*/

#ifndef __SIGNALMULTIPLEXER_H__
#define __SIGNALMULTIPLEXER_H__

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QList>

class QWidget;

class SignalMultiplexer : public QObject
{
    Q_OBJECT

public:
    
    SignalMultiplexer( QObject* parent = 0, const char* name = 0 );
    virtual ~SignalMultiplexer() {}

    void connect( QObject* sender, const char* signal, const char* slot );
    bool disconnect( QObject* sender, const char* signal, const char* slot );
    void connect( const char* signal, QObject* receiver, const char* slot );
    bool disconnect( const char* signal, QObject* receiver, const char* slot );
    
    QObject *currentObject() const { return mpObject; }

public slots:

    int setCurrentObject( QObject* newObject );

protected:
private:

    class Connection
    {
    public:
        Connection() : mSignal( "" ), mSlot( "" ) {}
        QPointer< QObject > mSender;
		QPointer< QObject > mReceiver;
        QString mSignal;
        QString mSlot;

        inline int operator == ( Connection & that ) const
            {
                return ((this->mSender == that.mSender) &&
                        (this->mReceiver == that.mReceiver) &&
                        (this->mSignal == that.mSignal) &&
                        (this->mSlot == that.mSlot) );
            }

    };

    void connect( const Connection& conn );
    void disconnect( const Connection& conn );

	QPointer< QObject >   mpObject;
    QList< Connection > mConnections;

};

#endif//__SIGNALMULTIPLEXER_H__

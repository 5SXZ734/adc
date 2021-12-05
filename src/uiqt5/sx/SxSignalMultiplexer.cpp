/* -*- C++ -*- -------------------------------------------------------------
 * Description: SignalMultiplexer - Taken From Qt Quarterley 8.
 * Signal Multiplexer class will allow us to easily connect and disconnect
 * senders from receivers (in this case, documents from main window actions).  
 * We need this to make the code cleaner and allow only the selected document
 * to respond to user events.
 *
 * -------------------------------------------------------------------------
*/
#include <QtCore/QObject>
#include "SxSignalMultiplexer.h"

//////////////////////////////////////////////////////////////

SignalMultiplexer::SignalMultiplexer(QObject* parent, const char* /*name*/)
	: QObject(parent),//, name),
    mpObject(0)
{

}

void SignalMultiplexer::connect(QObject* sender, const char* signal, const char* slot)
{
    Connection conn;
    conn.mSender = sender;
    conn.mSignal = signal;
    conn.mSlot = slot;
    
    mConnections.append(conn);
    connect(conn);
}

bool SignalMultiplexer::disconnect(QObject* sender, const char* signal, const char* slot)
{
    QList<Connection>::Iterator itr;
    for (itr = mConnections.begin(); itr != mConnections.end(); itr++)
    {
        Connection& conn = *itr;
	    if ( (static_cast<QObject*>(conn.mSender) == sender) &&
	        (conn.mSignal == signal) &&
	        (conn.mSlot == slot) )
	    {
	        disconnect(conn);
	        mConnections.erase(itr);
	        return true;
	    }
    }
    return false;
}

void SignalMultiplexer::connect(const char* signal, QObject* receiver, const char* slot)
{
    Connection conn;
    conn.mReceiver = receiver;
    conn.mSignal = signal;
    conn.mSlot = slot;

    mConnections.append(conn);
    connect(conn);
}
 
bool SignalMultiplexer::disconnect(const char* signal, QObject* receiver, const char* slot)
{
    QList<Connection>::Iterator itr;
    for (itr = mConnections.begin(); itr != mConnections.end(); itr++)
    {
        Connection& conn = *itr;

	    if ( (static_cast<QObject*>(conn.mReceiver) == receiver) &&
	        (conn.mSignal == signal) &&
	        (conn.mSlot == slot) )
	    {
	        disconnect(conn);
	        mConnections.erase(itr);
	        return true;
	    }
    }
    return false;
}

int SignalMultiplexer::setCurrentObject( QObject* newObject )
{
    if (newObject == mpObject) 
		return -1;//no change

    // Remove connections to old object
    QList<Connection>::Iterator itr;
    for (itr = mConnections.begin(); itr != mConnections.end(); itr++)
        disconnect(*itr);

    // Make connections to new object
    mpObject = newObject;
    for (itr = mConnections.begin(); itr != mConnections.end(); itr++)
        connect(*itr);

	return 1;
}

void SignalMultiplexer::connect(const Connection& conn)
{
    if (!mpObject) return;
    if (!conn.mSender && !conn.mReceiver) return;

    if (conn.mSender)
		QObject::connect(conn.mSender, conn.mSignal.toLatin1(), mpObject, conn.mSlot.toLatin1());
    else
		QObject::connect(mpObject, conn.mSignal.toLatin1(), conn.mReceiver, conn.mSlot.toLatin1());
}

void SignalMultiplexer::disconnect(const Connection& conn)
{
    if (!mpObject) return;
    if (!conn.mSender && !conn.mReceiver) return;

    if (conn.mSender)
		QObject::disconnect(conn.mSender, conn.mSignal.toLatin1(), mpObject, conn.mSlot.toLatin1());
    else
		QObject::disconnect(mpObject, conn.mSignal.toLatin1(), conn.mReceiver, conn.mSlot.toLatin1());
}


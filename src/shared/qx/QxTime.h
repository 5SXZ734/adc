#pragma once

class QxTime
{
public:
	enum TimeSpec {
		LocalTime,
		UTC
	};

    QxTime() { ds=0; }				// set null time
    QxTime( int h, int m, int s=0, int ms=0 );	// set time

    bool   isNull()	 const { return ds == 0; }
    bool   isValid()	 const;			// valid time

    int	   hour()	 const;			// 0..23
    int	   minute()	 const;			// 0..59
    int	   second()	 const;			// 0..59
    int	   msec()	 const;			// 0..999

    bool   setHMS( int h, int m, int s, int ms=0 );

    QxTime  addSecs( int secs )		const;
    int	   secsTo( const QxTime & )	const;
    QxTime  addMSecs( int ms )		const;
    int	   msecsTo( const QxTime & )	const;

    bool   operator==( const QxTime &d ) const { return ds == d.ds; }
    bool   operator!=( const QxTime &d ) const { return ds != d.ds; }
    bool   operator<( const QxTime &d )	const { return ds < d.ds; }
    bool   operator<=( const QxTime &d ) const { return ds <= d.ds; }
    bool   operator>( const QxTime &d )	const { return ds > d.ds; }
    bool   operator>=( const QxTime &d ) const { return ds >= d.ds; }

    static QxTime currentTime();
    static QxTime currentTime( TimeSpec );

    static bool	 isValid( int h, int m, int s, int ms=0 );

    void   start();
    int	   restart();
    int	   elapsed() const;

private:
    static bool currentTime( QxTime * );
    static bool currentTime( QxTime *, TimeSpec );

    unsigned   ds;
};




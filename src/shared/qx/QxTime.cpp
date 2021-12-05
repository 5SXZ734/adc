#include "QxTime.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <stdio.h>
#include <time.h>

static const unsigned FIRST_DAY = 2361222;	// Julian day for 1752-09-14
static const int  FIRST_YEAR = 1752;		// ### wrong for many countries
static const unsigned SECS_PER_DAY = 86400;
static const unsigned MSECS_PER_DAY = 86400000;
static const unsigned SECS_PER_HOUR = 3600;
static const unsigned MSECS_PER_HOUR = 3600000;
static const unsigned SECS_PER_MIN = 60;
static const unsigned MSECS_PER_MIN = 60000;

QxTime::QxTime(int h, int m, int s, int ms)
{
	setHMS(h, m, s, ms);
}

bool QxTime::isValid() const
{
	return ds < MSECS_PER_DAY;
}

int QxTime::hour() const
{
	return ds / MSECS_PER_HOUR;
}

int QxTime::minute() const
{
	return (ds % MSECS_PER_HOUR) / MSECS_PER_MIN;
}

int QxTime::second() const
{
	return (ds / 1000) % SECS_PER_MIN;
}

int QxTime::msec() const
{
	return ds % 1000;
}

bool QxTime::setHMS(int h, int m, int s, int ms)
{
	if (!isValid(h, m, s, ms)) {
		ds = MSECS_PER_DAY;		// make this invalid
		return false;
	}
	ds = (h*SECS_PER_HOUR + m*SECS_PER_MIN + s) * 1000 + ms;
	return true;
}

QxTime QxTime::addSecs(int nsecs) const
{
	return addMSecs(nsecs * 1000);
}

int QxTime::secsTo(const QxTime &t) const
{
	return ((int)t.ds - (int)ds) / 1000;
}

QxTime QxTime::addMSecs(int ms) const
{
	QxTime t;
	if (ms < 0) {
		// % not well-defined for -ve, but / is.
		int negdays = (MSECS_PER_DAY - ms) / MSECS_PER_DAY;
		t.ds = ((int)ds + ms + negdays*MSECS_PER_DAY)
			% MSECS_PER_DAY;
	}
	else {
		t.ds = ((int)ds + ms) % MSECS_PER_DAY;
	}
	return t;
}

int QxTime::msecsTo(const QxTime &t) const
{
	return (int)t.ds - (int)ds;
}

QxTime QxTime::currentTime()
{
	return currentTime(LocalTime);
}

QxTime QxTime::currentTime(TimeSpec ts)
{
	QxTime t;
	currentTime(&t, ts);
	return t;
}

bool QxTime::currentTime(QxTime *ct)
{
	return currentTime(ct, LocalTime);
}

#define _POSIX_THREAD_SAFE_FUNCTIONS	1

bool QxTime::currentTime(QxTime *ct, TimeSpec ts)
{
	if (!ct) {
		return false;
	}

#if defined(WIN32)
	SYSTEMTIME t;
	if (ts == LocalTime) {
		GetLocalTime(&t);
	}
	else {
		GetSystemTime(&t);
	}
	ct->ds = (unsigned)(MSECS_PER_HOUR*t.wHour + MSECS_PER_MIN*t.wMinute +
		1000 * t.wSecond + t.wMilliseconds);
#else//?if defined(UNIX)
	// posix compliant system
	struct timeval tv;
	gettimeofday(&tv, 0);
	time_t ltime = tv.tv_sec;
	tm *t;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
	// use the reentrant versions of localtime() and gmtime() where available
	tm res;
	if (ts == LocalTime)
		t = localtime_r(&ltime, &res);
	else
		t = gmtime_r(&ltime, &res);
#else
	if (ts == LocalTime)
		t = localtime(&ltime);
	else
		t = gmtime(&ltime);
#endif // _POSIX_THREAD_SAFE_FUNCTIONS

	ct->ds = (unsigned)(MSECS_PER_HOUR * t->tm_hour + MSECS_PER_MIN * t->tm_min +
		1000 * t->tm_sec + tv.tv_usec / 1000);
	/*?#else
	time_t ltime; // no millisecond resolution
	::time( &ltime );
	tm *t;
	if ( ts == LocalTime )
	localtime( &ltime );
	else
	gmtime( &ltime );
	ct->ds = (unsigned) ( MSECS_PER_HOUR * t->tm_hour + MSECS_PER_MIN * t->tm_min +
	1000 * t->tm_sec );*/
#endif
	// 00:00.00 to 00:00.59.999 is considered as "midnight or right after"
	return ct->ds < (unsigned)MSECS_PER_MIN;
}

bool QxTime::isValid(int h, int m, int s, int ms)
{
	return (unsigned)h < 24 && (unsigned)m < 60 && (unsigned)s < 60 && (unsigned)ms < 1000;
}

void QxTime::start()
{
	*this = currentTime();
}

int QxTime::restart()
{
	QxTime t = currentTime();
	int n = msecsTo(t);
	if (n < 0)				// passed midnight
		n += 86400 * 1000;
	*this = t;
	return n;
}

int QxTime::elapsed() const
{
	int n = msecsTo(currentTime());
	if (n < 0)				// passed midnight
		n += 86400 * 1000;
	return n;
}


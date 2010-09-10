
#include "ZKit_DateTime.h"

BEGIN_ZKIT

void DateTime::Init(time_t seconds)
{
	this->seconds = seconds;
	this->date = *localtime(&this->seconds);
}

//struct tm 
//{
//	int tm_sec;     /* seconds after the minute - [0,59] */
//	int tm_min;     /* minutes after the hour - [0,59] */
//	int tm_hour;    /* hours since midnight - [0,23] */
//	int tm_mday;    /* day of the month - [1,31] */
//	int tm_mon;     /* months since January - [0,11] */
//	int tm_year;    /* years since 1900 */
//	int tm_wday;    /* days since Sunday - [0,6] */
//	int tm_yday;    /* days since January 1 - [0,365] */
//	int tm_isdst;   /* daylight savings time flag */
//};

void DateTime::Init( int year,int month,int day,int hour,int minute,int second )
{
	if (year < 1900)
	{
		assert(0);
		year = 1970;
	}
	else if (month < 1 || month > 12)
	{
		assert(0);
		month = 0;
	}
	else if (day < 1 || day > 31)
	{
		assert(0);
		day = 1;
	}
	else if (hour < 0 || hour > 23)
	{
		assert(0);
		hour = 0;
	}
	else if (minute < 0 || minute > 59)
	{
		assert(0);
		minute = 0;
	}
	else if (second < 0 || second > 59)
	{
		assert(0);
		second = 0;
	}

	tm t;
	t.tm_year = year - 1900;
	t.tm_mon  = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min  = minute;
	t.tm_sec  = second;

	Init(mktime(&t));
}

void DateTime::Init(std::string dateTimeStr,std::string formaterStr)
{
	int year,month,day,hour,minutes,seconds;
	sscanf(dateTimeStr.c_str(),formaterStr.c_str(),&year,&month,&day,&hour,&minutes,&seconds); 
	Init(year, month, day, hour, minutes, seconds);
}

DateTime::DateTime(const time_t seconds)
{
	Init(seconds);
}

DateTime::DateTime(int year,int month,int day)
{
	Init(year, month, day, 0, 0, 0);
}

DateTime::DateTime(int year,int month,int day,int hour,int minute,int second)
{
	Init(year, month, day, hour, minute, second);
}

DateTime::DateTime(std::string strDateTime)
{
	Init(strDateTime, "%d/%d/%d %d:%d:%d");
}

DateTime::DateTime(std::string strDateTime,std::string format)
{
	Init(strDateTime, format);
}

DateTime DateTime::Parse(std::string strDateTime)
{
	return DateTime(strDateTime);
}

DateTime DateTime::Parse(const std::string strDateTime,const std::string format)
{
	return DateTime(strDateTime,format);
}

int DateTime::DayOfWeek() const
{
	return this->date.tm_wday;
}

int DateTime::DayOfYear() const
{
	return this->date.tm_yday;
}

int DateTime::DaysInMonth(const int year,const int month)
{
	static const int day_of_month[13] = {31,31,28,31,30,31,30,31,31,30,31,30,31};

	int m = (month < 1 || month > 12)? 0 : month;

	if (m == 2 && ( (year%4 == 0) && (year%100 != 0) ) || (year%400 == 0) )
	{
		return 29;
	}
	else
	{
		return day_of_month[m];
	}
}

bool DateTime::Equals(const DateTime *dateTime) const
{
	return this->seconds == dateTime->seconds;
}

bool DateTime::Equals(const DateTime *value1,const DateTime *value2)
{
	return value1->seconds == value2->seconds;
}

int DateTime::GetDay() const
{    
	return this->date.tm_mday;;
}

int DateTime::GetHour() const
{
	return this->date.tm_hour;
}
int DateTime::GetMinute() const
{
	return this->date.tm_min;
}
int DateTime::GetMonth() const
{
	return this->date.tm_mon;
}

DateTime DateTime::GetNow()
{
	return DateTime(time(0));
}

int DateTime::GetSecond() const
{
	return this->date.tm_sec;
}

int DateTime::GetYear() const
{
	return this->date.tm_year+1900;
}

void DateTime::AddYears( const int years )
{
	this->date.tm_year = this->date.tm_year + (int)years;
	this->seconds = mktime(&this->date);
}
void DateTime::AddMonths(const int months)
{
	int a  =(int)((this->date.tm_mon+months)/12);

	this->date.tm_year = this->date.tm_year+a;
	this->date.tm_mon = this->date.tm_mon+(int)((this->date.tm_mon+months)%12)-1;
	this->seconds = mktime(&this->date);
}

void DateTime::AddDays(const int days)
{    
	this->AddHours(days*24);
}
void DateTime::AddHours(const int hours)
{
	this->AddMinutes(hours*60);
}
void DateTime::AddMinutes(const int minutes)
{
	this->AddSeconds(minutes *60);
}

void DateTime::AddSeconds(const int seconds)
{
	this->seconds = this->seconds+seconds;
	this->date = *localtime(&this->seconds);
}
void DateTime::AddWeeks(const int weeks)
{
	this->AddDays(weeks*7);
}

int DateTime::CompareTo(const DateTime *value) const
{
	return (int)(this->seconds - value->seconds);
}

int DateTime::Compare(const DateTime *t1,const DateTime *t2)
{
	return t1->CompareTo(t2);
}

std::string DateTime::ToString(const std::string formaterStr) const
{
	char s[256];
	strftime(s, sizeof(s), formaterStr.c_str(), &this->date);
	return std::string(s);
}

std::string DateTime::ToString() const
{
	return this->ToString("%Y-%m-%d %H:%M:%S");
}

std::string DateTime::ToShortDateString() const
{
	return this->ToString("%Y-%m-%d");
}

bool DateTime::operator ==( DateTime &datetime)
{
	return this->seconds == datetime.seconds;
}

bool DateTime::operator != (DateTime &datetime)
{
	return this->seconds != datetime.seconds;
}
bool DateTime::operator > (DateTime &dateTime)
{
	return this->seconds > dateTime.seconds;
}
bool DateTime::operator < (DateTime &datetime)
{
	return this->seconds <datetime.seconds;
}
bool DateTime::operator >=(DateTime &datetime)
{
	return this->seconds >= datetime.seconds;
}

bool DateTime::operator <=(DateTime &datetime)
{
	return this->seconds <= datetime.seconds;
}
END_ZKIT
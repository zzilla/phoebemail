#ifndef _ZKit_DateTime_h_
#define _ZKit_DateTime_h_

#include "ZKit_Config.h"

BEGIN_ZKIT

class DateTime
{
public:    
	DateTime(time_t seconds);
	DateTime(int year,int month,int day);
	DateTime(int year,int month,int day,int hour,int minute,int second);

	DateTime(std::string strDateTime);
	DateTime(std::string strDateTime, std::string format);

	static DateTime Parse(std::string strDateTime);//yyyy/MM/dd HH:mm:ss

	static DateTime Parse(std::string strDateTime,std::string format);

	static DateTime GetNow(); //返回当前日期是间

public:
	void AddSeconds(const int seconds); //将指定的秒数加到此实例的值上。
	void AddMinutes(const int minutes);//将指定的分钟数加到此实例的值上。 
	void AddHours(const int hours);//将指定的小时数加到此实例的值上。 
	void AddDays(const int days); //将指定的天数加到此实例的值上。 
	void AddWeeks(const int weeks);//将指定的周数加到些实上的值上。
	void AddMonths(const int Months);//将指定的月份数加到此实例的值上。  
	void AddYears(const int years); //将指定的年份数加到此实例的值上。  
	
public:
	static int Compare(const DateTime *value1,const DateTime *value2);
	int CompareTo(const DateTime *value) const;    

	bool Equals(const DateTime* dateTime) const;
	static bool Equals(const DateTime *value1,const DateTime *value2);
 
	std::string ToString() const;
	std::string ToString(const std::string format) const;//format = "%Y-%m-%d %H:%M:%S" %Y=年 %m=月 %d=日 %H=时 %M=分 %S=秒
	std::string ToShortDateString() const;//将当前 DateTime 对象的值转换为其等效的短日期字符串表示形式。 

public:        
	int GetYear() const;//获取此实例所表示日期的年份部分。
	int GetMonth() const;//获取此实例所表示日期的年份部分。
	int GetDay() const;// 获取此实例所表示的日期为该月中的第几天。
	int GetHour() const;//获取此实例所表示日期的小时部分。 
	int GetMinute() const;//获取此实例所表示日期的分钟部分
	int GetSecond() const;//获取此实例所表示日期的秒部分。 
	int DayOfWeek() const; //获取此实例所表示的日期是星期几。
	int DayOfYear() const;//记录今天是一年里面的第几天,从1月1日起,0-365

	static int DaysInMonth(const int year,const int months);//返回指定年和月中的天数。
public:
	bool operator == (DateTime &dateTime);
	bool operator > (DateTime &dateTime);
	bool operator < (DateTime &DateTime);
	bool operator >= (DateTime &DateTime);
	bool operator <= (DateTime &DateTime);
	bool operator != (DateTime &DateTime);

private:
	void Init(time_t seconds);

	void Init(int year,int month,int day,int hour,int minute,int second);

	void Init(std::string strDateTime, std::string format);

	time_t seconds;//自1970起的秒数
	tm date;
};
END_ZKIT

#endif // _ZKit_DateTime_h_


/*
 * SccsId:	@(#)csstime.h	43.1	9/9/91
 */

struct date_time{
	double epoch;
	long date;
	int year;
	int month;
	char mname[4];
	int day;
	int doy;
	int hour;
	int minute;
	float second;
};

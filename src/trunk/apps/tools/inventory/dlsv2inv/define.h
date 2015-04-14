#ifndef DEFINE_H
#define DEFINE_H
#include <iostream>
#include <map>

// separators to split each request line
#define LINE_SEPARATOR  ' '
#define TIME_SEPARATOR  ','
#define ATTR_SEPARATOR  '='
#define CSV_SEPARATOR	'|'
#define SEED_SEPARATOR	'~'

// waiting time for responses coming back
#define LRECL		4096
#define	LINE		512

// abbreviations for analogue or digital transformation
#define ACCEL1		 "M/S**2"
#define ACCEL2		 "M/S2"
#define VELOCITY	 "M/S"
#define DISPLACE	 "M"
#define VOLTAGE		 "V"
#define DIGITAL		 "COUNTS"
#define TEMPERATURE  "C"
#define TEMPERATURE2 "CELSIUS"
#define PRESSURE     "PA"

// initializing values for gettting or saving data
#define ARCHIVE     "ARCH"

typedef std::map<std::string, std::string> INIT_MAP;
typedef std::map<std::string, std::string> STATION_INFO;

#endif //DEFINE_H

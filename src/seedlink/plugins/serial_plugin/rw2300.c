/*  open2300  - rw2300.c library functions
 *  This is a library of functions common to Linux and Windows
 *  
 *  Version 1.10
 *  
 *  Control WS2300 weather station
 *  
 *  Copyright 2003-2005, Kenneth Lavrsen
 *  This program is published under the GNU General Public license
 */

#include "rw2300.h"

/********************************************************************/
/* temperature_indoor
 * Read indoor temperature, current temperature only
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Returns: Temperature (deg C if temperature_conv is 0)
 *                      (deg F if temperature_conv is 1)
 *
 ********************************************************************/
double temperature_indoor(WEATHERSTATION ws2300, int temperature_conv)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x346;
	int bytes=2;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	if (temperature_conv)
		return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
		          (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) -
		          30.0) * 9 / 5 + 32);
	else
		return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
		          (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0));
}


/********************************************************************/
/* temperature_indoor_minmax
 * Read indoor min/max temperatures with timestamps
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Output: Temperatures temp_min and temp_max
 *                (deg C if temperature_conv is 0)
 *                (deg F if temperature_conv is 1)
 *         Timestamps for temp_min and temp_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 ********************************************************************/
void temperature_indoor_minmax(WEATHERSTATION ws2300,
                               int temperature_conv,
                               double *temp_min,
                               double *temp_max,
                               struct timestamp *time_min,
                               struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x34B;
	int bytes=15;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
	
	*temp_min = ((data[1]>>4)*10 + (data[1]&0xF) + (data[0]>>4)/10.0 +
	             (data[0]&0xF)/100.0) - 30.0;

	*temp_max = ((data[4]&0xF)*10 + (data[3]>>4) + (data[3]&0xF)/10.0 +
	             (data[2]>>4)/100.0) - 30.0;

	if (temperature_conv)
	{
		*temp_min = *temp_min * 9/5 + 32;
		*temp_max = *temp_max * 9/5 + 32;
	}

	time_min->minute = ((data[5] & 0xF) * 10) + (data[4] >> 4);
	time_min->hour = ((data[6] & 0xF) * 10) + (data[5] >> 4);
	time_min->day = ((data[7] & 0xF) * 10) + (data[6] >> 4);
	time_min->month = ((data[8] & 0xF) * 10) + (data[7] >> 4);
	time_min->year = 2000 + ((data[9] & 0xF) * 10) + (data[8] >> 4);
		
	time_max->minute = ((data[10] & 0xF) * 10) + (data[9] >> 4);
	time_max->hour = ((data[11] & 0xF) * 10) + (data[10] >> 4);
	time_max->day = ((data[12] & 0xF) * 10) + (data[11] >> 4);
	time_max->month = ((data[13] & 0xF) * 10) + (data[12] >> 4);
	time_max->year = 2000 + ((data[14] & 0xF) * 10) + (data[13] >> 4);
	
	return;
}

/********************************************************************/
/* temperature_indoor_reset
 * Reset indoor min/max temperatures with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int temperature_indoor_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current temperature into data_value
	address=0x346;
	number=2;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x34B;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x354;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x350;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x35E;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************/
/* temperature_outdoor
 * Read indoor temperature, current temperature only
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Returns: Temperature (deg C if temperature_conv is 0)
 *                      (deg F if temperature_conv is 1)
 *
 ********************************************************************/
double temperature_outdoor(WEATHERSTATION ws2300, int temperature_conv)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x373;
	int bytes=2;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	if (temperature_conv)
		return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
		          (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) -
		          30.0) * 9 / 5 + 32);
	else
		return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
		          (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0));
}


/********************************************************************
 * temperature_outdoor_minmax
 * Read outdoor min/max temperatures with timestamps
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Output: Temperatures temp_min and temp_max
 *                (deg C if temperature_conv is 0)
 *                (deg F if temperature_conv is 1)
 *         Timestamps for temp_min and temp_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 ********************************************************************/
void temperature_outdoor_minmax(WEATHERSTATION ws2300,
                                int temperature_conv,
                                double *temp_min,
                                double *temp_max,
                                struct timestamp *time_min,
                                struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x378;
	int bytes=15;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
	
	*temp_min = ((data[1]>>4)*10 + (data[1]&0xF) + (data[0]>>4)/10.0 +
	             (data[0]&0xF)/100.0) - 30.0;

	*temp_max = ((data[4]&0xF)*10 + (data[3]>>4) + (data[3]&0xF)/10.0 +
	             (data[2]>>4)/100.0) - 30.0;

	if (temperature_conv)
	{
		*temp_min = *temp_min * 9/5 + 32;
		*temp_max = *temp_max * 9/5 + 32;
	}

	time_min->minute = ((data[5] & 0xF) * 10) + (data[4] >> 4);
	time_min->hour = ((data[6] & 0xF) * 10) + (data[5] >> 4);
	time_min->day = ((data[7] & 0xF) * 10) + (data[6] >> 4);
	time_min->month = ((data[8] & 0xF) * 10) + (data[7] >> 4);
	time_min->year = 2000 + ((data[9] & 0xF) * 10) + (data[8] >> 4);
	
	time_max->minute = ((data[10] & 0xF) * 10) + (data[9] >> 4);
	time_max->hour = ((data[11] & 0xF) * 10) + (data[10] >> 4);
	time_max->day = ((data[12] & 0xF) * 10) + (data[11] >> 4);
	time_max->month = ((data[13] & 0xF) * 10) + (data[12] >> 4);
	time_max->year = 2000 + ((data[14] & 0xF) * 10) + (data[13] >> 4);
	
	return;
}


/********************************************************************/
/* temperature_outdoor_reset
 * Reset outdoor min/max temperatures with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int temperature_outdoor_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current temperature into data_value
	address=0x373;
	number=2;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x378;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x381;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x37D;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x38B;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************
 * dewpoint
 * Read dewpoint, current value only
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Returns: Dewpoint    (deg C if temperature_conv is 0)
 *                      (deg F if temperature_conv is 1)
 *
 ********************************************************************/
double dewpoint(WEATHERSTATION ws2300, int temperature_conv)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x3CE;
	int bytes=2;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	if (temperature_conv)
		return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
		          (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) -
		          30.0) * 9 / 5 + 32);
	else
		return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
		          (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0));
}


/********************************************************************
 * dewpoint_minmax
 * Read outdoor min/max dewpoint with timestamps
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Output: Dewpoints dp_min and dp_max
 *                (deg C if temperature_conv is 0),
 *                (deg F if temperature_conv is 1)
 *         Timestamps for dp_min and dp_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 ********************************************************************/
void dewpoint_minmax(WEATHERSTATION ws2300,
                     int temperature_conv,
                     double *dp_min,
                     double *dp_max,
                     struct timestamp *time_min,
                     struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x3D3;
	int bytes=15;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
	
	*dp_min = ((data[1]>>4)*10 + (data[1]&0xF) + (data[0]>>4)/10.0 +
	           (data[0]&0xF)/100.0) - 30.0;

	*dp_max = ((data[4]&0xF)*10 + (data[3]>>4) + (data[3]&0xF)/10.0 +
	           (data[2]>>4)/100.0) - 30.0;

	if (temperature_conv)
	{
		*dp_min = *dp_min * 9/5 + 32;
		*dp_max = *dp_max * 9/5 + 32;
	}

	time_min->minute = ((data[5] & 0xF) * 10) + (data[4] >> 4);
	time_min->hour = ((data[6] & 0xF) * 10) + (data[5] >> 4);
	time_min->day = ((data[7] & 0xF) * 10) + (data[6] >> 4);
	time_min->month = ((data[8] & 0xF) * 10) + (data[7] >> 4);
	time_min->year = 2000 + ((data[9] & 0xF) * 10) + (data[8] >> 4);
	
	time_max->minute = ((data[10] & 0xF) * 10) + (data[9] >> 4);
	time_max->hour = ((data[11] & 0xF) * 10) + (data[10] >> 4);
	time_max->day = ((data[12] & 0xF) * 10) + (data[11] >> 4);
	time_max->month = ((data[13] & 0xF) * 10) + (data[12] >> 4);
	time_max->year = 2000 + ((data[14] & 0xF) * 10) + (data[13] >> 4);
	
	return;
}


/********************************************************************/
/* dewpoint_reset
 * Reset min/max dewpoint with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int dewpoint_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current dewpoint into data_value
	address=0x3CE;
	number=2;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x3D3;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x3DC;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x3D8;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x3E6;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************
 * humidity_indoor
 * Read indoor relative humidity, current value only
 * 
 * Input: Handle to weatherstation
 * Returns: relative humidity in percent (integer)
 * 
 ********************************************************************/
int humidity_indoor(WEATHERSTATION ws2300)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x3FB;
	int bytes=1;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	return ((data[0] >> 4) * 10 + (data[0] & 0xF));
}


/********************************************************************
 * humidity_indoor_all
 * Read both current indoor humidity and min/max values with timestamps
 * 
 * Input: Handle to weatherstation
 * Output: Relative humidity in % hum_min and hum_max (integers)
 *         Timestamps for hum_min and hum_max in pointers to
 *                timestamp structures for time_min and time_max
 * Returns: releative humidity current value in % (integer)
 *
 ********************************************************************/
int humidity_indoor_all(WEATHERSTATION ws2300,
                        int *hum_min,
                        int *hum_max,
                        struct timestamp *time_min,
                        struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x3FB;
	int bytes=13;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	*hum_min = (data[1] >> 4) * 10 + (data[1] & 0xF);
	*hum_max = (data[2] >> 4) * 10 + (data[2] & 0xF);

	time_min->minute = ((data[3] >> 4) * 10) + (data[3] & 0xF);
	time_min->hour = ((data[4] >> 4) * 10) + (data[4] & 0xF);
	time_min->day = ((data[5] >> 4) * 10) + (data[5] & 0xF);
	time_min->month = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_min->year = 2000 + ((data[7] >> 4) * 10) + (data[7] & 0xF);
	
	time_max->minute = ((data[8] >> 4) * 10) + (data[8] & 0xF);
	time_max->hour = ((data[9] >> 4) * 10) + (data[9] & 0xF);
	time_max->day = ((data[10] >> 4) * 10) + (data[10] & 0xF);
	time_max->month = ((data[11] >> 4) * 10) + (data[11] & 0xF);
	time_max->year = 2000 + ((data[12] >> 4) * 10) + (data[12] & 0xF);
	
	return ((data[0] >> 4) * 10 + (data[0] & 0xF));
}


/********************************************************************/
/* humidity_indoor_reset
 * Reset min/max indoor humidity with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int humidity_indoor_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current humidity into data_value
	address=0x3FB;
	number=1;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x3FD;
		number=2;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x401;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x3FF;
		number=2;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x40B;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();		
	}

	return 1;
}


/********************************************************************
 * humidity_outdoor
 * Read relative humidity, current value only
 * 
 * Input: Handle to weatherstation
 * Returns: relative humidity in percent (integer)
 *
 ********************************************************************/
int humidity_outdoor(WEATHERSTATION ws2300)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x419;
	int bytes=1;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	return ((data[0] >> 4) * 10 + (data[0] & 0xF));
}


/********************************************************************
 * humidity_outdoor_all
 * Read both current outdoor humidity and min/max values with timestamps
 * 
 * Input: Handle to weatherstation
 * Output: Relative humidity in % hum_min and hum_max (integers)
 *         Timestamps for hum_min and hum_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: releative humidity current value in % (integer)
 *
 ********************************************************************/
int humidity_outdoor_all(WEATHERSTATION ws2300,
                         int *hum_min,
                         int *hum_max,
                         struct timestamp *time_min,
                         struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x419;
	int bytes=13;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	*hum_min = (data[1] >> 4) * 10 + (data[1] & 0xF);
	*hum_max = (data[2] >> 4) * 10 + (data[2] & 0xF);

	time_min->minute = ((data[3] >> 4) * 10) + (data[3] & 0xF);
	time_min->hour = ((data[4] >> 4) * 10) + (data[4] & 0xF);
	time_min->day = ((data[5] >> 4) * 10) + (data[5] & 0xF);
	time_min->month = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_min->year = 2000 + ((data[7] >> 4) * 10) + (data[7] & 0xF);
	
	time_max->minute = ((data[8] >> 4) * 10) + (data[8] & 0xF);
	time_max->hour = ((data[9] >> 4) * 10) + (data[9] & 0xF);
	time_max->day = ((data[10] >> 4) * 10) + (data[10] & 0xF);
	time_max->month = ((data[11] >> 4) * 10) + (data[11] & 0xF);
	time_max->year = 2000 + ((data[12] >> 4) * 10) + (data[12] & 0xF);
	
	return ((data[0] >> 4) * 10 + (data[0] & 0xF));
}


/********************************************************************/
/* humidity_outdoor_reset
 * Reset min/max outdoor humidity with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int humidity_outdoor_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current humidity into data_value
	address=0x419;
	number=1;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x41B;
		number=2;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x41F;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x41D;
		number=2;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x429;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************
 * wind_current
 * Read wind speed, wind direction and last 5 wind directions
 *
 * Input: Handle to weatherstation
 *        wind_speed_conv_factor controlling convertion to other
 *             units than m/s
 *
 * Output: winddir - pointer to double in degrees
 *
 * Returns: Wind speed (double) in the unit given in the loaded config
 *
 ********************************************************************/
double wind_current(WEATHERSTATION ws2300,
                    double wind_speed_conv_factor,
                    double *winddir)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int i;
	int address=0x527; //Windspeed and direction
	int bytes=3;
	
	for (i=0; i<MAXWINDRETRIES; i++)
	{
		if (read_safe(ws2300, address, bytes, data, command)!=bytes) //Wind
			read_error_exit();
		
		if ( (data[0]!=0x00) ||                            //Invalid wind data
		    ((data[1]==0xFF) && (((data[2]&0xF)==0)||((data[2]&0xF)==1))) )
		{
			sleep_long(10); //wait 10 seconds for new wind measurement
			continue;
		}
		else
		{
			break;
		}
	}
	
	//Calculate wind directions

	*winddir = (data[2]>>4)*22.5;

	//Calculate raw wind speed 	- convert from m/s to whatever
	return( (((data[2]&0xF)<<8)+(data[1])) / 10.0 * wind_speed_conv_factor );
}


/********************************************************************
 * wind_all
 * Read wind speed, wind direction and last 5 wind directions
 *
 * Input: Handle to weatherstation
 *        wind_speed_conv_factor controlling convertion to other
 *             units than m/s
 *
 * Output: winddir_index
 *              Current wind direction expressed as ticks from North
 *              where North=0. Used to convert to direction string
 *         winddir
 *              Array of doubles containing current winddirection
 *              in winddir[0] and the last 5 in the following
 *              positions all given in degrees
 *
 * Returns: Wind speed (double) in the unit given in the loaded config
 *
 ********************************************************************/
double wind_all(WEATHERSTATION ws2300,
                double wind_speed_conv_factor,
                int *winddir_index,
                double *winddir)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int i;
	int address=0x527; //Windspeed and direction
	int bytes=6;
	
	for (i=0; i<MAXWINDRETRIES; i++)
	{
		if (read_safe(ws2300, address, bytes, data, command)!=bytes) //Wind
			read_error_exit();
		     
		if ( (data[0]!=0x00) ||                             //Invalid wind data
		   ((data[1]==0xFF) && (((data[2]&0xF)==0)||( (data[2]&0xF)==1))) )
		{
			sleep_long(10); //wait 10 seconds for new wind measurement
			continue;
		}
		else
		{
			break;
		}
	}
	
	//Calculate wind directions

	*winddir_index = (data[2]>>4);
	winddir[0] = (data[2]>>4)*22.5;
	winddir[1] = (data[3]&0xF)*22.5;
	winddir[2] = (data[3]>>4)*22.5;
	winddir[3] = (data[4]&0xF)*22.5;
	winddir[4] = (data[4]>>4)*22.5;
	winddir[5] = (data[5]&0xF)*22.5;

	//Calculate raw wind speed - convert from m/s to whatever
	return ( (((data[2]&0xF)<<8)+(data[1]) ) / 10.0 * wind_speed_conv_factor);
}


/********************************************************************
 * wind_minmax
 * Read min/max wind speeds with timestamps
 * 
 * Input: Handle to weatherstation
 *        wind_speed_conv_factor controlling convertion to other
 *             units than m/s
 *
 * Output: Wind wind_min and wind_max (double)
 *                unit defined by config conversion factor
 *         Timestamps for wind_min and wind_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: wind max (double)
 *
 * Note: The function is made so that if a pointer to
 *       wind_min/max and time_min/max is a NULL pointer the function
 *       ignores this parameter. Example: if you only need wind_max
 *       use the function like this..
 *       windmax = wind_minmax(ws2300,METERS_PER_SECOND,NULL,NULL,NULL,NULL);
 *
 ********************************************************************/
double wind_minmax(WEATHERSTATION ws2300,
                   double wind_speed_conv_factor,
                   double *wind_min,
                   double *wind_max,
                   struct timestamp *time_min,
                   struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x4EE;
	int bytes=15;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
			read_error_exit();
	
	if (wind_min != NULL)
		*wind_min = (data[1]*256 + data[0])/360.0 * wind_speed_conv_factor;
	if (wind_max != NULL)
		*wind_max = (data[4]*256 + data[3])/360.0 * wind_speed_conv_factor;

	if (time_min != NULL)
	{	
		time_min->minute = ((data[5] >> 4) * 10) + (data[5] & 0xF);
		time_min->hour = ((data[6] >> 4) * 10) + (data[6] & 0xF);
		time_min->day = ((data[7] >> 4) * 10) + (data[7] & 0xF);
		time_min->month = ((data[8] >> 4) * 10) + (data[8] & 0xF);
		time_min->year = 2000 + ((data[9] >> 4) * 10) + (data[9] & 0xF);
	}
	
	if (time_max != NULL)
	{
		time_max->minute = ((data[10] >> 4) * 10) + (data[10] & 0xF);
		time_max->hour = ((data[11] >> 4) * 10) + (data[11] & 0xF);
		time_max->day = ((data[12] >> 4) * 10) + (data[12] & 0xF);
		time_max->month = ((data[13] >> 4) * 10) + (data[13] & 0xF);
		time_max->year = 2000 + ((data[14] >> 4) * 10) + (data[14] & 0xF);
	}
	
	return ((data[4]*256 + data[3])/360.0 * wind_speed_conv_factor);
}


/********************************************************************/
/* wind_reset
 * Reset min/max wind with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int wind_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;
	int i;
	int current_wind;
	
	address=0x527; //Windspeed
	number=3;
	
	for (i=0; i<MAXWINDRETRIES; i++)
	{
		if (read_safe(ws2300, address, number, data_read, command)!=number)
			read_error_exit();
		     
		if ((data_read[0]!=0x00) ||                            //Invalid wind data
		    ((data_read[1]==0xFF)&&(((data_read[2]&0xF)==0)||((data_read[2]&0xF)==1))))
		{
			sleep_long(10); //wait 10 seconds for new wind measurement
			continue;
		}
		else
		{
			break;
		}
	}
	
	current_wind = ( ((data_read[2]&0xF)<<8) + (data_read[1]) ) * 36;

	data_value[0] = current_wind&0xF;
	data_value[1] = (current_wind>>4)&0xF;
	data_value[2] = (current_wind>>8)&0xF;
	data_value[3] = (current_wind>>12)&0xF;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x4EE;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x4F8;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x4F4;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x502;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();		
	}

	return 1;
}


/********************************************************************
 * windchill
 * Read wind chill, current value only
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Returns: wind chill  (deg C if config.temperature_conv is not set)
 *                      (deg F if config.temperature_conv is set)
 *
 * It is recommended to run this right after a wind speed reading
 * to enhance the likelyhood that the wind speed is valid
 *
 ********************************************************************/
double windchill(WEATHERSTATION ws2300, int temperature_conv)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x3A0;
	int bytes=2;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	if (temperature_conv)
		return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
		          (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) -
		          30.0) * 9 / 5 + 32);
	else
		return ((((data[1] >> 4) * 10 + (data[1] & 0xF) +
		          (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) - 30.0));
}


/********************************************************************
 * windchill_minmax
 * Read wind chill min/max with timestamps
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Output: Windchill wc_min and wc_max
 *                (deg C if config.temperature_conv is not set)
 *                (deg F if config.temperature_conv is set)
 *         Timestamps for wc_min and wc_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: Nothing
 *
 ********************************************************************/
void windchill_minmax(WEATHERSTATION ws2300,
                      int temperature_conv,
                      double *wc_min,
                      double *wc_max,
                      struct timestamp *time_min,
                      struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x3A5;
	int bytes=15;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
	
	*wc_min = ( (data[1]>>4)*10 + (data[1]&0xF) + (data[0]>>4)/10.0 +
	            (data[0]&0xF)/100.0 ) - 30.0;
	
	*wc_max = ( (data[4]&0xF)*10 + (data[3]>>4) + (data[3]&0xF)/10.0 +
	            (data[2]>>4)/100.0 ) - 30.0;

	if (temperature_conv)
	{
		*wc_min = *wc_min * 9/5 + 32;
		*wc_max = *wc_max * 9/5 + 32;
	}

	time_min->minute = ((data[5] & 0xF) * 10) + (data[4] >> 4);
	time_min->hour = ((data[6] & 0xF) * 10) + (data[5] >> 4);
	time_min->day = ((data[7] & 0xF) * 10) + (data[6] >> 4);
	time_min->month = ((data[8] & 0xF) * 10) + (data[7] >> 4);
	time_min->year = 2000 + ((data[9] & 0xF) * 10) + (data[8] >> 4);
	
	time_max->minute = ((data[10] & 0xF) * 10) + (data[9] >> 4);
	time_max->hour = ((data[11] & 0xF) * 10) + (data[10] >> 4);
	time_max->day = ((data[12] & 0xF) * 10) + (data[11] >> 4);
	time_max->month = ((data[13] & 0xF) * 10) + (data[12] >> 4);
	time_max->year = 2000 + ((data[14] & 0xF) * 10) + (data[13] >> 4);
	
	return;
}


/********************************************************************/
/* windchill_reset
 * Reset min/max windchill with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int windchill_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current windchill into data_value
	address=0x3A0;
	number=2;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x3A5;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x3AE;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x3AA;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x3B8;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************
 * rain_1h
 * Read rain last 1 hour, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_1h(WEATHERSTATION ws2300, double rain_conv_factor)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x4B4;
	int bytes=3;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	return ( ((data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 +
	          (data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 +
	          (data[0] & 0xF) / 100.0 ) / rain_conv_factor);
}

/********************************************************************
 * rain_1h_all
 * Read rain last 1 hourand maximum with timestamp
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Output: Rain maximum in rain_max (double)
 *                unit defined by config conversion factor
 *         Timestamps for rain_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_1h_all(WEATHERSTATION ws2300,
                   double rain_conv_factor,
                   double *rain_max,
                   struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x4B4;
	int bytes=11;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
		
	*rain_max = ((data[5] >> 4) * 1000 + (data[5] & 0xF) * 100 +
	             (data[4] >> 4) * 10 + (data[4] & 0xF) + (data[3]>>4)/10.0 +
	             (data[3] & 0xF) / 100.0) / rain_conv_factor;
	             
	time_max->minute = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_max->hour = ((data[7] >> 4) * 10) + (data[7] & 0xF);
	time_max->day = ((data[8] >> 4) * 10) + (data[8] & 0xF);
	time_max->month = ((data[9] >> 4) * 10) + (data[9] & 0xF);
	time_max->year = 2000 + ((data[10] >> 4) * 10) + (data[10] & 0xF);

	return (((data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 +
	         (data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 +
	         (data[0] & 0xF) / 100.0) / rain_conv_factor);
}


/********************************************************************/
/* rain_1h_max_reset
 * Reset max rain 1h with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_1h_max_reset(WEATHERSTATION ws2300)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current rain 1h into data_value
	address=0x4B4;
	number=3;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	data_value[4] = data_read[2]&0xF;
	data_value[5] = data_read[2]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	// Set max value to current value
	address=0x4BA;
	number=6;
	
	if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
		write_error_exit();

	// Set max value timestamp to current time
	address=0x4C0;
	number=10;

	if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
		write_error_exit();

	return 1;
}

/********************************************************************/
/* rain_1h_reset
 * Reset current rain 1h
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_1h_reset(WEATHERSTATION ws2300)
{
	unsigned char data[50];
	unsigned char command[60];	//room for write data also
	int address;
	int number;

	// First overwrite the 1h rain history with zeros
	address=0x479;
	number=30;
	memset(&data, 0, sizeof(data));
	
	if (write_safe(ws2300, address, number, WRITENIB, data, command) != number)
		write_error_exit();
	
	// Set value to zero
	address=0x4B4;
	number=6;
	
	if (write_safe(ws2300, address, number, WRITENIB, data, command) != number)
		write_error_exit();

	return 1;
}


/********************************************************************
 * rain_24h
 * Read rain last 24 hours, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_24h(WEATHERSTATION ws2300, double rain_conv_factor)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x497;
	int bytes=3;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	return (((data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 +
	         (data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 +
	         (data[0] & 0xF) / 100.0) / rain_conv_factor);
}


/********************************************************************
 * rain_24h_all
 * Read rain last 24 hours and maximum with timestamp
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Output: Rain maximum in rain_max (double)
 *                unit defined by config conversion factor
 *         Timestamp for rain_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_24h_all(WEATHERSTATION ws2300,
                   double rain_conv_factor,
                   double *rain_max,
                   struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x497;
	int bytes=11;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
	
	*rain_max = ((data[5] >> 4) * 1000 + (data[5] & 0xF) * 100 +
	             (data[4] >> 4) * 10 + (data[4] & 0xF) + (data[3]>>4)/10.0 +
	             (data[3] & 0xF) / 100.0) / rain_conv_factor;
	
	time_max->minute = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_max->hour = ((data[7] >> 4) * 10) + (data[7] & 0xF);
	time_max->day = ((data[8] >> 4) * 10) + (data[8] & 0xF);
	time_max->month = ((data[9] >> 4) * 10) + (data[9] & 0xF);
	time_max->year = 2000 + ((data[10] >> 4) * 10) + (data[10] & 0xF);

	return (((data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 +
	         (data[1] >> 4) * 10 + (data[1] & 0xF) + (data[0] >> 4) / 10.0 +
	         (data[0] & 0xF) / 100.0) / rain_conv_factor);
}


/********************************************************************/
/* rain_24h_max_reset
 * Reset max rain 24h with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_24h_max_reset(WEATHERSTATION ws2300)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current rain 24h into data_value
	address=0x497;
	number=3;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	data_value[4] = data_read[2]&0xF;
	data_value[5] = data_read[2]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	// Set max value to current value
	address=0x49D;
	number=6;
	
	if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
		write_error_exit();

	// Set max value timestamp to current time
	address=0x4A3;
	number=10;

	if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
		write_error_exit();

	return 1;
}


/********************************************************************/
/* rain_24h_reset
 * Reset current rain 24h
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_24h_reset(WEATHERSTATION ws2300)
{
	unsigned char data[50];
	unsigned char command[60];	//room for write data also
	int address;
	int number;

	// First overwrite the 24h rain history with zeros
	address=0x446;
	number=48;
	memset(&data, 0, sizeof(data));
	
	if (write_safe(ws2300, address, number, WRITENIB, data, command) != number)
		write_error_exit();
	
	// Set value to zero
	address=0x497;
	number=6;
	
	if (write_safe(ws2300, address, number, WRITENIB, data, command) != number)
		write_error_exit();

	return 1;
}


/********************************************************************
 * rain_total
 * Read rain accumulated total, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_total(WEATHERSTATION ws2300, double rain_conv_factor)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x4D2;
	int bytes=3;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	return (((data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 +
	         (data[1] >> 4) * 10 + (data[1] & 0xF) +
	         (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) /
	         rain_conv_factor);
}


/********************************************************************
 * rain_total_all
 * Read rain total accumulated with timestamp
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Output: Timestamp for rain total in pointers to
 *                timestamp structures for time_since
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_total_all(WEATHERSTATION ws2300,
                   double rain_conv_factor,
                   struct timestamp *time_since)
{
	unsigned char data[20];
	unsigned char command[25];	//room for write data also
	int address=0x4D2;
	int bytes=8;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();

	time_since->minute = ((data[3] >> 4) * 10) + (data[3] & 0xF);
	time_since->hour = ((data[4] >> 4) * 10) + (data[4] & 0xF);
	time_since->day = ((data[5] >> 4) * 10) + (data[5] & 0xF);
	time_since->month = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_since->year = 2000 + ((data[7] >> 4) * 10) + (data[7] & 0xF);

	return (((data[2] >> 4) * 1000 + (data[2] & 0xF) * 100 +
	         (data[1] >> 4) * 10 + (data[1] & 0xF) +
	         (data[0] >> 4) / 10.0 + (data[0] & 0xF) / 100.0) /
	         rain_conv_factor);
}


/********************************************************************/
/* rain_total_reset
 * Reset current total rain
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_total_reset(WEATHERSTATION ws2300)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	// Set value to zero
	address=0x4D1;
	number=7;
	memset(&data_value, 0, sizeof(data_value));
	
	if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
		write_error_exit();

	// Set max value timestamp to current time
	address=0x4D8;
	number=10;

	if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
		write_error_exit();

	return 1;
}


/********************************************************************
 * rel_pressure
 * Read relaive air pressure, current value only
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Returns: pressure (double) converted to unit given in config
 *
 ********************************************************************/
double rel_pressure(WEATHERSTATION ws2300, double pressure_conv_factor)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x5E2;
	int bytes=3;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();


	return (((data[2] & 0xF) * 1000 + (data[1] >> 4) * 100 +
	         (data[1] & 0xF) * 10 + (data[0] >> 4) +
	         (data[0] & 0xF) / 10.0) / pressure_conv_factor);
}


/********************************************************************
 * rel_pressure_minmax
 * Read relative pressure min/max with timestamps
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Output: Pressure pres_min and pres_max (double)
 *                unit defined by config conversion factor
 *         Timestamps for pres_min and pres_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: nothing
 *
 ********************************************************************/
void rel_pressure_minmax(WEATHERSTATION ws2300,
                         double pressure_conv_factor,
                         double *pres_min,
                         double *pres_max,
                         struct timestamp *time_min,
                         struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x600;
	int bytes=13;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
	
	*pres_min = ((data[2]&0xF)*1000 + (data[1]>>4)*100 +
	            (data[1]&0xF)*10 + (data[0]>>4) +
	            (data[0]&0xF)/10.0) / pressure_conv_factor;
	
	*pres_max = ((data[12]&0xF)*1000 + (data[11]>>4)*100 +
	            (data[11]&0xF)*10 + (data[10]>>4) +
	            (data[10]&0xF)/10.0) / pressure_conv_factor;
		
	address=0x61E; //Relative pressure time and date for min/max
	bytes=10;
	
	if (read_safe(ws2300, address, bytes, data, command)!=bytes)	
		read_error_exit();

	time_min->minute = ((data[0] >> 4) * 10) + (data[0] & 0xF);
	time_min->hour = ((data[1] >> 4) * 10) + (data[1] & 0xF);
	time_min->day = ((data[2] >> 4) * 10) + (data[2] & 0xF);
	time_min->month = ((data[3] >> 4) * 10) + (data[3] & 0xF);
	time_min->year = 2000 + ((data[4] >> 4) * 10) + (data[4] & 0xF);
	
	time_max->minute = ((data[5] >> 4) * 10) + (data[5] & 0xF);
	time_max->hour = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_max->day = ((data[7] >> 4) * 10) + (data[7] & 0xF);
	time_max->month = ((data[8] >> 4) * 10) + (data[8] & 0xF);
	time_max->year = 2000 + ((data[9] >> 4) * 10) + (data[9] & 0xF);
	
	return;
}


/********************************************************************
 * abs_pressure
 * Read absolute air pressure, current value only
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Returns: pressure (double) converted to unit given in config
 *
 ********************************************************************/
double abs_pressure(WEATHERSTATION ws2300, double pressure_conv_factor)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x5D8;
	int bytes=3;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();


	return (((data[2] & 0xF) * 1000 + (data[1] >> 4) * 100 +
	         (data[1] & 0xF) * 10 + (data[0] >> 4) +
	         (data[0] & 0xF) / 10.0) / pressure_conv_factor);
}


/********************************************************************
 * abs_pressure_minmax
 * Read absolute pressure min/max with timestamps
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Output: Pressure pres_min and pres_max (double)
 *                unit defined by config conversion factor
 *         Timestamps for pres_min and pres_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: nothing
 *
 ********************************************************************/
void abs_pressure_minmax(WEATHERSTATION ws2300,
                         double pressure_conv_factor,
                         double *pres_min,
                         double *pres_max,
                         struct timestamp *time_min,
                         struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x5F6;
	int bytes=13;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
	
	*pres_min = ((data[2]&0xF)*1000 + (data[1]>>4)*100 +
	            (data[1]&0xF)*10 + (data[0]>>4) +
	            (data[0]&0xF)/10.0) / pressure_conv_factor;
	
	*pres_max = ((data[12]&0xF)*1000 + (data[11]>>4)*100 +
	            (data[11]&0xF)*10 + (data[10]>>4) +
	            (data[10]&0xF)/10.0) / pressure_conv_factor;
		
	address=0x61E; //Relative pressure time and date for min/max
	bytes=10;
	
	if (read_safe(ws2300, address, bytes, data, command)!=bytes)	
		read_error_exit();

	time_min->minute = ((data[0] >> 4) * 10) + (data[0] & 0xF);
	time_min->hour = ((data[1] >> 4) * 10) + (data[1] & 0xF);
	time_min->day = ((data[2] >> 4) * 10) + (data[2] & 0xF);
	time_min->month = ((data[3] >> 4) * 10) + (data[3] & 0xF);
	time_min->year = 2000 + ((data[4] >> 4) * 10) + (data[4] & 0xF);
	
	time_max->minute = ((data[5] >> 4) * 10) + (data[5] & 0xF);
	time_max->hour = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_max->day = ((data[7] >> 4) * 10) + (data[7] & 0xF);
	time_max->month = ((data[8] >> 4) * 10) + (data[8] & 0xF);
	time_max->year = 2000 + ((data[9] >> 4) * 10) + (data[9] & 0xF);
	
	return;
}



/********************************************************************/
/* pressure_reset
 * Reset min/max pressure (relative and absolute) with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int pressure_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value_abs[20];
	unsigned char data_value_rel[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current abs/rel pressure into data_value_abs/rel
	address=0x5D8;
	number=8;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_value_abs[0] = data_read[0]&0xF;
	data_value_abs[1] = data_read[0]>>4;
	data_value_abs[2] = data_read[1]&0xF;
	data_value_abs[3] = data_read[1]>>4;
	data_value_abs[4] = data_read[2]&0xF;
	
	data_value_rel[0] = data_read[5]&0xF;
	data_value_rel[1] = data_read[5]>>4;
	data_value_rel[2] = data_read[6]&0xF;
	data_value_rel[3] = data_read[6]>>4;
	data_value_rel[4] = data_read[7]&0xF;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min abs value to current abs value
		address=0x5F6;
		number=5;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value_abs, command) != number)
			write_error_exit();
			
		// Set min rel value to current rel value
		address=0x600;
		number=5;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value_rel, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x61E;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max abs value to current abs value
		address=0x60A;
		number=5;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value_abs, command) != number)
			write_error_exit();
			
		// Set max rel value to current rel value
		address=0x614;
		number=5;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value_rel, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x628;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();		
	}

	return 1;
}


/********************************************************************
 * pressure_correction
 * Read the correction from absolute to relaive air pressure
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Returns: pressure (double) converted to unit given in conv factor
 *
 ********************************************************************/
double pressure_correction(WEATHERSTATION ws2300, double pressure_conv_factor)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x5EC;
	int bytes=3;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();


	return ((data[2] & 0xF) * 1000 +
	        (data[1] >> 4) * 100 +
	        (data[1] & 0xF) * 10 +
	        (data[0] >> 4) +
	        (data[0] & 0xF) / 10.0 - 
	        1000
	       ) / pressure_conv_factor;
}


/********************************************************************
 * tendency_forecast
 * Read Tendency and Forecast
 * 
 * Input: Handle to weatherstation
 *
 * Output: tendency - string Steady, Rising or Falling
 *         forecast - string Rainy, Cloudy or Sunny
 *
 * Returns: nothing
 *
 ********************************************************************/
void tendency_forecast(WEATHERSTATION ws2300, char *tendency, char *forecast)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x26B;
	int bytes=1;
	const char *tendency_values[] = { "Steady", "Rising", "Falling" };
	const char *forecast_values[] = { "Rainy", "Cloudy", "Sunny" };

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
	    read_error_exit();

	strcpy(tendency, tendency_values[data[0] >> 4]);
	strcpy(forecast, forecast_values[data[0] & 0xF]);

	return;
}


/********************************************************************
 * read_history_info
 * Read the history information like interval, countdown, time
 * of last record, pointer to last record.
 * 
 * Input:  Handle to weatherstation
 *        
 * Output: interval - Current interval in minutes (integer)
 *         countdown - Countdown to next measurement (integer)
 *         timelast - Time/Date for last measurement (timestamp struct)
 *         no_records - number of valid records (integer)
 *
 * Returns: interger pointing to last written record. [0x00-0xAE]
 *
 ********************************************************************/
int read_history_info(WEATHERSTATION ws2300, int *interval, int *countdown,
                 struct timestamp *time_last, int *no_records)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x6B2;
	int bytes=10;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
	    read_error_exit();
	
	*interval = (data[1] & 0xF)*256 + data[0] + 1;
	*countdown = data[2]*16 + (data[1] >> 4) + 1;
	time_last->minute = ((data[3] >> 4) * 10) + (data[3] & 0xF);
	time_last->hour = ((data[4] >> 4) * 10) + (data[4] & 0xF);
	time_last->day = ((data[5] >> 4) * 10) + (data[5] & 0xF);
	time_last->month = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_last->year = 2000 + ((data[7] >> 4) * 10) + (data[7] & 0xF);
	*no_records = data[9];

	return data[8];

}


/********************************************************************
 * read_history_record
 * Read the history information like interval, countdown, time
 * of last record, pointer to last record.
 * 
 * Input:  Handle to weatherstation
 *         config structure with conversion factors
 *         record - record index number to be read [0x00-0xAE]
 *        
 * Output: temperature_indoor (double)
 *         temperature_indoor (double)
 *         pressure (double)
 *         humidity_indoor (integer)
 *         humidity_outdoor (integer)
 *         raincount (double)
 *         windspeed (double)
 *         windir_degrees (double)
 *         dewpoint (double) - calculated
 *         windchill (double) - calculated, new post 2001 formula
 *
 * Returns: interger index number pointing to next record 
 *
 ********************************************************************/
int read_history_record(WEATHERSTATION ws2300,
                        int record,
                        struct config_type *config,
                        double *temperature_indoor,
                        double *temperature_outdoor,
                        double *pressure,
                        int *humidity_indoor,
                        int *humidity_outdoor,
                        double *raincount,
                        double *windspeed,
                        double *winddir_degrees,
                        double *dewpoint,
                        double *windchill)
{
	unsigned char data[20];
	unsigned char command[25];
	int address;
	int bytes=10;
	long int tempint;
	double A, B, C; // Intermediate values used for dewpoint calculation
	double wind_kmph;

	address = 0x6C6 + record*19;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
	    read_error_exit();
	
	tempint = (data[4]<<12) + (data[3]<<4) + (data[2] >> 4);
	
	*pressure = 1000 + (tempint % 10000)/10.0;
	
	if (*pressure >= 1502.2)
		*pressure = *pressure - 1000;
		
	*pressure = *pressure / config->pressure_conv_factor;
	
	*humidity_indoor = (tempint - (tempint % 10000)) / 10000.0;

	*humidity_outdoor = (data[5]>>4)*10 + (data[5]&0xF);
	
	*raincount = ((data[7]&0xF)*256 + data[6]) * 0.518 / config->rain_conv_factor;
	
	*windspeed = (data[8]*16 + (data[7]>>4))/ 10.0; //Need metric for WC
	
	*winddir_degrees = (data[9]&0xF)*22.5;
	
	// Temperatures	in Celcius. Cannot convert until WC is calculated
	tempint = ((data[2] & 0xF)<<16) + (data[1]<<8) + data[0];
	*temperature_indoor = (tempint % 1000)/10.0 - 30.0;
	*temperature_outdoor = (tempint - (tempint % 1000))/10000.0 - 30.0;
	
	// Calculate windchill using new post 2001 USA/Canadian formula
	// Twc = 13.112 + 0.6215*Ta -11.37*V^0.16 + 0.3965*Ta*V^0.16 [Celcius and km/h] 
	
	wind_kmph = 3.6 * *windspeed;
	if (wind_kmph > 4.8)
	{
		*windchill = 13.12 + 0.6215 * *temperature_outdoor -
		             11.37 * pow(wind_kmph, 0.16) +
		             0.3965 * *temperature_outdoor * pow(wind_kmph, 0.16);
	}
	else
	{
		*windchill = *temperature_outdoor;
	}
	
	// Calculate dewpoint
	// REF http://www.faqs.org/faqs/meteorology/temp-dewpoint/             
	A = 17.2694;
	B = (*temperature_outdoor > 0) ? 237.3 : 265.5;
	C = (A * *temperature_outdoor)/(B + *temperature_outdoor) + log((double)*humidity_outdoor/100);
	*dewpoint = B * C / (A - C);

	// Now that WC/DP is calculated we can convert all temperatures and winds
	if (config->temperature_conv)
	{
		*temperature_indoor = *temperature_indoor * 9/5 + 32;
		*temperature_outdoor = *temperature_outdoor * 9/5 + 32;
		*windchill = *windchill * 9/5 + 32;
		*dewpoint = *dewpoint * 9/5 + 32;
	}
	
	*windspeed *= config->wind_speed_conv_factor;
	
	return (++record)%0xAF;
}


/********************************************************************
 * light
 * Turns display light on and off
 *
 * Input: control - integer -   0 = off, Anything else = on
 *
 * Returns: Nothing
 *
 ********************************************************************/
void light(WEATHERSTATION ws2300, int control)
{
	unsigned char data;
	unsigned char command[25];  //Data returned is just ignored
	int address=0x016;
	int number=1;
	unsigned char encode_constant;
	
	data = 0;
	encode_constant = UNSETBIT;
	if (control != 0)
		encode_constant = SETBIT;
		
	if (write_safe(ws2300, address, number, encode_constant, &data, command)!=number)
		write_error_exit();
	
	return;	
}


/********************************************************************
 * read_error_exit
 * exit location for all calls to read_safe for error exit.
 * includes error reporting.
 *
 ********************************************************************/
void read_error_exit(void)
{
	perror("read_safe() error");
	exit(0);
}

/********************************************************************
 * write_error_exit
 * exit location for all calls to write_safe for error exit.
 * includes error reporting.
 *
 ********************************************************************/
void write_error_exit(void)
{
	perror("write_safe() error");
	exit(0);
}


/********************************************************************
 * get_configuration()
 *
 * read setup parameters from ws2300.conf
 * It searches in this sequence:
 * 1. Path to config file including filename given as parameter
 * 2. ./open2300.conf
 * 3. /usr/local/etc/open2300.conf
 * 4. /etc/open2300.conf
 *
 * See file open2300.conf-dist for the format and option names/values
 *
 * input:    config file name with full path - pointer to string
 *
 * output:   struct config populated with valid settings either
 *           from config file or defaults
 *
 * returns:  0 = OK
 *          -1 = no config file or file open error
 *
 ********************************************************************/
int get_configuration(struct config_type *config, char *path)
{
	FILE *fptr;
	char inputline[1000] = "";
	char token[100] = "";
	char val[100] = "";
	char val2[100] = "";
	
	// First we set everything to defaults - faster than many if statements
	strcpy(config->serial_device_name, DEFAULT_SERIAL_DEVICE);  // Name of serial device
	strcpy(config->citizen_weather_id, "CW0000");               // Citizen Weather ID
	strcpy(config->citizen_weather_latitude, "5540.12N");       // latitude default Glostrup, DK
	strcpy(config->citizen_weather_longitude, "01224.60E");     // longitude default, Glostrup, DK
	strcpy(config->aprs_host[0].name, "aprswest.net");         // host1 name
	config->aprs_host[0].port = 23;                            // host1 port
	strcpy(config->aprs_host[1].name, "indiana.aprs2.net");    // host2 name
	config->aprs_host[1].port = 23;                            // host2 port
	config->num_hosts = 2;                                     // default number of defined hosts
	strcpy(config->weather_underground_id, "WUID");             // Weather Underground ID 
	strcpy(config->weather_underground_password, "WUPassword"); // Weather Underground Password
	strcpy(config->timezone, "1");                              // Timezone, default CET
	config->wind_speed_conv_factor = 1.0;                   // Speed dimention, m/s is default
	config->temperature_conv = 0;                           // Temperature in Celcius
	config->rain_conv_factor = 1.0;                         // Rain in mm
	config->pressure_conv_factor = 1.0;                     // Pressure in hPa (same as millibar)
	strcpy(config->mysql_host, "localhost");            // localhost, IP or domainname of server
	strcpy(config->mysql_user, "open2300");             // MySQL database user name
	strcpy(config->mysql_passwd, "mysql2300");          // Password for MySQL database user
	strcpy(config->mysql_database, "open2300");         // Name of MySQL database
	config->mysql_port = 0;                             // MySQL port. 0 means default port/socket
	strcpy(config->pgsql_connect, "hostaddr='127.0.0.1'dbname='open2300'user='postgres'"); // connection string
	strcpy(config->pgsql_table, "weather");             // PgSQL table name
	strcpy(config->pgsql_station, "open2300");          // Unique station id

	// open the config file

	fptr = NULL;
	if (path != NULL)
		fptr = fopen(path, "r");       //first try the parameter given
	if (fptr == NULL)                  //then try default search
	{
		if ((fptr = fopen("open2300.conf", "r")) == NULL)
		{
			if ((fptr = fopen("/usr/local/etc/open2300.conf", "r")) == NULL)
			{
				if ((fptr = fopen("/etc/open2300.conf", "r")) == NULL)
				{
					//Give up and use defaults
					return(-1);
				}
			}
		}
	}

	while (fscanf(fptr, "%[^\n]\n", inputline) != EOF)
	{
		sscanf(inputline, "%[^= \t]%*[ \t=]%s%*[, \t]%s%*[^\n]", token, val, val2);

		if (token[0] == '#')	// comment
			continue;

		if ((strcmp(token,"SERIAL_DEVICE")==0) && (strlen(val) != 0))
		{
			strcpy(config->serial_device_name,val);
			continue;
		}

		if ((strcmp(token,"CITIZEN_WEATHER_ID")==0) && (strlen(val) != 0))
		{
			strcpy(config->citizen_weather_id, val);
			continue;
		}
		
		if ((strcmp(token,"CITIZEN_WEATHER_LATITUDE")==0) && (strlen(val)!=0))
		{
			strcpy(config->citizen_weather_latitude, val);
			continue;
		}

		if ((strcmp(token,"CITIZEN_WEATHER_LONGITUDE")==0) && (strlen(val)!=0))
		{
			strcpy(config->citizen_weather_longitude, val);
			continue;
		}
		
		if ((strcmp(token,"APRS_SERVER")==0) && (strlen(val)!=0) && (strlen(val2)!=0))
		{
			if ( config->num_hosts >= MAX_APRS_HOSTS)
				continue;           // ignore host definitions over the defined max
			strcpy(config->aprs_host[config->num_hosts].name, val);
			config->aprs_host[config->num_hosts].port = atoi(val2);
			config->num_hosts++;    // increment for next
			continue;
		}

		if ((strcmp(token,"WEATHER_UNDERGROUND_ID")==0) && (strlen(val)!=0))
		{
			strcpy(config->weather_underground_id, val);
			continue;
		}

		if ((strcmp(token,"WEATHER_UNDERGROUND_PASSWORD")==0)&&(strlen(val)!=0))
		{
			strcpy(config->weather_underground_password, val);
			continue;
		}

		if ((strcmp(token,"TIMEZONE")==0) && (strlen(val) != 0))
		{
			strcpy(config->timezone, val);
			continue;
		}

		if ((strcmp(token,"WIND_SPEED") == 0) && (strlen(val) != 0))
		{
			if (strcmp(val, "m/s") == 0)
				config->wind_speed_conv_factor = METERS_PER_SECOND;
			else if (strcmp(val, "km/h") == 0)
				config->wind_speed_conv_factor = KILOMETERS_PER_HOUR;
			else if (strcmp(val, "MPH") == 0)
				config->wind_speed_conv_factor = MILES_PER_HOUR;
			continue; //else default remains
		}

		if ((strcmp(token,"TEMPERATURE") == 0) && (strlen(val) != 0))
		{
			if (strcmp(val, "C") == 0)
				config->temperature_conv = CELCIUS;
			else if (strcmp(val, "F") == 0)
				config->temperature_conv = FAHRENHEIT;
			continue; //else default remains
		}

		if ((strcmp(token,"RAIN") == 0) && (strlen(val) != 0))
		{
			if (strcmp(val, "mm") == 0)
				config->rain_conv_factor = MILLIMETERS;
			else if (strcmp(val, "IN") == 0)
				config->rain_conv_factor = INCHES;
			continue; //else default remains
		}

		if ((strcmp(token,"PRESSURE") == 0) && (strlen(val) != 0))
		{
			if ( (strcmp(val, "hPa") == 0) || (strcmp(val, "mb") == 0))
				config->pressure_conv_factor = HECTOPASCAL;
			else if (strcmp(val, "INHG") == 0)
				config->pressure_conv_factor = INCHES_HG;
			continue; //else default remains
		}

		if ((strcmp(token,"MYSQL_HOST") == 0) && (strlen(val) != 0))
		{
			strcpy(config->mysql_host, val);
			continue;
		}

		if ( (strcmp(token,"MYSQL_USERNAME") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->mysql_user, val);
			continue;
		}

		if ( (strcmp(token,"MYSQL_PASSWORD") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->mysql_passwd, val);
			continue;
		}

		if ( (strcmp(token,"MYSQL_DATABASE") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->mysql_database, val);
			continue;
		}
		
		if ( (strcmp(token,"MYSQL_PORT") == 0) && (strlen(val) != 0) )
		{
			config->mysql_port = atoi(val);
			continue;
		}

		if ( (strcmp(token,"PGSQL_CONNECT") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->pgsql_connect, val);
			continue;
		}
		
		if ( (strcmp(token,"PGSQL_TABLE") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->pgsql_table, val);
			continue;
		}
		
		if ( (strcmp(token,"PGSQL_STATION") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->pgsql_station, val);
			continue;
		}
		
	}

	return (0);
}


 /********************************************************************
 * address_encoder converts an 16 bit address to the form needed
 * by the WS-2300 when sending commands.
 *
 * Input:   address_in (interger - 16 bit)
 * 
 * Output:  address_out - Pointer to an unsigned character array.
 *          3 bytes, not zero terminated.
 * 
 * Returns: Nothing.
 *
 ********************************************************************/
void address_encoder(int address_in, unsigned char *address_out)
{
	int i = 0;
	int adrbytes = 4;
	unsigned char nibble;

	for (i = 0; i < adrbytes; i++)
	{
		nibble = (address_in >> (4 * (3 - i))) & 0x0F;
		address_out[i] = (unsigned char) (0x82 + (nibble * 4));
	}

	return;
}


/********************************************************************
 * data_encoder converts up to 15 data bytes to the form needed
 * by the WS-2300 when sending write commands.
 *
 * Input:   number - number of databytes (integer)
 *          encode_constant - unsigned char
 *                            0x12=set bit, 0x32=unset bit, 0x42=write nibble
 *          data_in - char array with up to 15 hex values
 * 
 * Output:  address_out - Pointer to an unsigned character array.
 * 
 * Returns: Nothing.
 *
 ********************************************************************/
void data_encoder(int number, unsigned char encode_constant,
                  unsigned char *data_in, unsigned char *data_out)
{
	int i = 0;

	for (i = 0; i < number; i++)
	{
		data_out[i] = (unsigned char) (encode_constant + (data_in[i] * 4));
	}

	return;
}


/********************************************************************
 * numberof_encoder converts the number of bytes we want to read
 * to the form needed by the WS-2300 when sending commands.
 *
 * Input:   number interger, max value 15
 * 
 * Returns: unsigned char which is the coded number of bytes
 *
 ********************************************************************/
unsigned char numberof_encoder(int number)
{
	int coded_number;

	coded_number = (unsigned char) (0xC2 + number * 4);
	if (coded_number > 0xfe)
		coded_number = 0xfe;

	return coded_number;
}


/********************************************************************
 * command_check0123 calculates the checksum for the first 4
 * commands sent to WS2300.
 *
 * Input:   pointer to char to check
 *          sequence of command - i.e. 0, 1, 2 or 3.
 * 
 * Returns: calculated checksum as unsigned char
 *
 ********************************************************************/
unsigned char command_check0123(unsigned char *command, int sequence)
{
	int response;

	response = sequence * 16 + ((*command) - 0x82) / 4;

	return (unsigned char) response;
}


/********************************************************************
 * command_check4 calculates the checksum for the last command
 * which is sent just before data is received from WS2300
 *
 * Input: number of bytes requested
 * 
 * Returns: expected response from requesting number of bytes
 *
 ********************************************************************/
unsigned char command_check4(int number)
{
	int response;

	response = 0x30 + number;

	return response;
}


/********************************************************************
 * data_checksum calculates the checksum for the data bytes received
 * from the WS2300
 *
 * Input:   pointer to array of data to check
 *          number of bytes in array
 * 
 * Returns: calculated checksum as unsigned char
 *
 ********************************************************************/
unsigned char data_checksum(unsigned char *data, int number)
{
	int checksum = 0;
	int i;

	for (i = 0; i < number; i++)
	{
		checksum += data[i];
	}

	checksum &= 0xFF;

	return (unsigned char) checksum;
}


/********************************************************************
 * initialize resets WS2300 to cold start (rewind and start over)
 * 
 * Input:   device number of the already open serial port
 *           
 * Returns: 0 if fail, 1 if success
 *
 ********************************************************************/
int initialize(WEATHERSTATION ws2300)
{
	unsigned char command = 0x06;
	unsigned char answer;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);
	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	if (answer != 2)
		return 0;

	return 1;
}


/********************************************************************
 * read_data reads data from the WS2300 based on a given address,
 * number of data read, and a an already open serial port
 *
 * Inputs:  serdevice - device number of the already open serial port
 *          address (interger - 16 bit)
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 *          commanddata - pointer to an array of chars containing
 *                     the commands that were sent to the station
 * 
 * Returns: number of bytes read, -1 if failed
 *
 ********************************************************************/
int read_data(WEATHERSTATION ws2300, int address, int number,
			  unsigned char *readdata, unsigned char *commanddata)
{

	unsigned char answer;
	int i;

	// First 4 bytes are populated with converted address range 0000-13B0
	address_encoder(address, commanddata);
	// Last populate the 5th byte with the converted number of bytes
	commanddata[4] = numberof_encoder(number);

	for (i = 0; i < 4; i++)
	{
		if (write_device(ws2300, commanddata + i, 1) != 1)
			return -1;
		if (read_device(ws2300, &answer, 1) != 1)
			return -1;
		if (answer != command_check0123(commanddata + i, i))
			return -1;
	}

	//Send the final command that asks for 'number' of bytes, check answer
	if (write_device(ws2300, commanddata + 4, 1) != 1)
		return -1;
	if (read_device(ws2300, &answer, 1) != 1)
		return -1;
	if (answer != command_check4(number))
		return -1;

	//Read the data bytes
	for (i = 0; i < number; i++)
	{
		if (read_device(ws2300, readdata + i, 1) != 1)
			return -1;
	}

	//Read and verify checksum
	if (read_device(ws2300, &answer, 1) != 1)
		return -1;
	if (answer != data_checksum(readdata, number))
		return -1;
		
	return i;

}


/********************************************************************
 * write_data writes data to the WS2300.
 * It can both write nibbles and set/unset bits
 *
 * Inputs:      ws2300 - device number of the already open serial port
 *              address (interger - 16 bit)
 *              number - number of nibbles to be written/changed
 *                       must 1 for bit modes (SETBIT and UNSETBIT)
 *                       max 80 for nibble mode (WRITENIB)
 *              encode_constant - unsigned char
 *                                (SETBIT, UNSETBIT or WRITENIB)
 *              writedata - pointer to an array of chars containing
 *                          data to write, not zero terminated
 *                          data must be in hex - one digit per byte
 *                          If bit mode value must be 0-3 and only
 *                          the first byte can be used.
 * 
 * Output:      commanddata - pointer to an array of chars containing
 *                            the commands that were sent to the station
 *
 * Returns:     number of bytes written, -1 if failed
 *
 ********************************************************************/
int write_data(WEATHERSTATION ws2300, int address, int number,
			   unsigned char encode_constant, unsigned char *writedata,
			   unsigned char *commanddata)
{
	unsigned char answer;
	unsigned char encoded_data[80];
	int i = 0;
	unsigned char ack_constant = WRITEACK;
	
	if (encode_constant == SETBIT)
	{
		ack_constant = SETACK;
	}
	else if (encode_constant == UNSETBIT)
	{
		ack_constant = UNSETACK;
	}

	// First 4 bytes are populated with converted address range 0000-13XX
	address_encoder(address, commanddata);
	// populate the encoded_data array
	data_encoder(number, encode_constant, writedata, encoded_data);

	//Write the 4 address bytes
	for (i = 0; i < 4; i++)
	{
		if (write_device(ws2300, commanddata + i, 1) != 1)
			return -1;
		if (read_device(ws2300, &answer, 1) != 1)
			return -1;
		if (answer != command_check0123(commanddata + i, i))
			return -1;
	}

	//Write the data nibbles or set/unset the bits
	for (i = 0; i < number; i++)
	{
		if (write_device(ws2300, encoded_data + i, 1) != 1)
			return -1;
		if (read_device(ws2300, &answer, 1) != 1)
			return -1;
		if (answer != (writedata[i] + ack_constant))
			return -1;
		commanddata[i + 4] = encoded_data[i];
	}

	return i;
}


/********************************************************************
 * read_safe Read data, retry until success or maxretries
 * Reads data from the WS2300 based on a given address,
 * number of data read, and a an already open serial port
 * Uses the read_data function and has same interface
 *
 * Inputs:  ws2300 - device number of the already open serial port
 *          address (interger - 16 bit)
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 *          commanddata - pointer to an array of chars containing
 *                     the commands that were sent to the station
 * 
 * Returns: number of bytes read, -1 if failed
 *
 ********************************************************************/
int read_safe(WEATHERSTATION ws2300, int address, int number,
			  unsigned char *readdata, unsigned char *commanddata)
{
	int j;

	for (j = 0; j < MAXRETRIES; j++)
	{
		reset_06(ws2300);
		
		// Read the data. If expected number of bytes read break out of loop.
		if (read_data(ws2300, address, number, readdata, commanddata)==number)
		{
			break;
		}
	}

	// If we have tried MAXRETRIES times to read we expect not to
	// have valid data
	if (j == MAXRETRIES)
	{
		return -1;
	}

	return number;
}


/********************************************************************
 * write_safe Write data, retry until success or maxretries
 * Writes data to the WS2300 based on a given address,
 * number of data to write, and a an already open serial port
 * Uses the write_data function and has same interface
 *
 * Inputs:      serdevice - device number of the already open serial port
 *              address (interger - 16 bit)
 *              number - number of nibbles to be written/changed
 *                       must 1 for bit modes (SETBIT and UNSETBIT)
 *                       unlimited for nibble mode (WRITENIB)
 *              encode_constant - unsigned char
 *                               (SETBIT, UNSETBIT or WRITENIB)
 *              writedata - pointer to an array of chars containing
 *                          data to write, not zero terminated
 *                          data must be in hex - one digit per byte
 *                          If bit mode value must be 0-3 and only
 *                          the first byte can be used.
 * 
 * Output:      commanddata - pointer to an array of chars containing
 *                            the commands that were sent to the station
 * 
 * Returns: number of bytes written, -1 if failed
 *
 ********************************************************************/
int write_safe(WEATHERSTATION ws2300, int address, int number,
               unsigned char encode_constant, unsigned char *writedata,
               unsigned char *commanddata)
{
	int j;

	for (j = 0; j < MAXRETRIES; j++)
	{
		// printf("Iteration = %d\n",j); // debug
		reset_06(ws2300);

		// Read the data. If expected number of bytes read break out of loop.
		if (write_data(ws2300, address, number, encode_constant, writedata,
		    commanddata)==number)
		{
			break;
		}
	}

	// If we have tried MAXRETRIES times to read we expect not to
	// have valid data
	if (j == MAXRETRIES)
	{
		return -1;
	}

	return number;
}


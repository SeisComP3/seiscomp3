/*
 * Copyright 1990 Science Applications International Corporation.
 */
	
/* 
 * FILENAME 
 *	db_site.h
 *
 * DESCRIPTION
 *	This contains a structure definition for the 3.0 release 
 *	site relation and is used in passing data to the database routines.
 *
 *
 *	Note that these definitions allow an additional byte for character
 *	fields to store the NULL.
 *
 *  SCCSId:    @(#)db_site.h	43.1	9/9/91 Copyright 1990 Science Applications International Corporation
 */
#ifndef DB_SITE_H
#define DB_SITE_H

typedef struct site
{
	char    sta[16];
	long    ondate;
	long    offdate;
	float   lat;
	float   lon;
	float   elev;
	char	staname[256];
	char	statype[5];
	char	refsta[7];
	float   dnorth;
	float   deast;
	char    lddate[18];
} Site;

#endif /* DB_SITE_H */

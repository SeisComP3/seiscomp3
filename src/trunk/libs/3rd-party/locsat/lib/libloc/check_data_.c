 
/*
 * NAME
 *	check_data_ -- Search input data file for valid seismic data.

 * FILE
 *	check_data_.c

 * SYNOPSIS
 *	Check seismic data for valid station, wave type, a priori standard
 *	deviation, and datum type (i.e., Slowness, Azimuth, or Travel-time).

 * DESCRIPTION
 *	Function.  Discriminate between valid and invalid data for use
 *	in determining a solution.  The valid data of interest include:
 *	slowness, azimuth and travel-times; valid station ID's, correct
 *	wave and datum types; and a proper a priori standard deviation.

 *	---- Indexing ----
 *	i = 0, num_sta-1;	j = 0, num_phase_types-1;   n = 0, num_data-1;

 *	---- On entry ----
 *	num_data:		Number of data
 *	num_sta:		Number of stations
 *	num_phase_types:	Number of wave-types allowed
 *	*data_sta_id:		Name of station for n'th datum
 *	*data_phase_type:	Name of phase for n'th datum
 *	*data_type:		Data type for n'th datum (time, slow, azim)
 *	*sta_id:		Name of i'th acceptible station
 *	*phase_type:		Name of j'th acceptible wave
 *	data_std_err[n]:	Standard deviation in value of n'th datum
 *	sta_len			Length of staid element.
 *	phase_len		Length of phase_type element.

 *	---- On return ----
 *	sta_index[n]:		Station index for n'th observation
 *	ipwav[n]:		Wave index for n'th observation
 *	idtyp[n]:		Type code for n'th observation
 *				  = 0, Data type unknown 
 *				  = 1, Arrival time datum 
 *				  = 2, Azimuth datum
 *				  = 3, Slowness datum
 *	data_error_code[n]:	Error code for n'th observation
 *				  = 0, No problem, datum is valid
 *				  = 1, No station information for this datum
 *				  = 2, No travel-time table for this datum
 *				  = 3, Data type unknown
 *				  = 4, Standard deviation <= 0.0 for this datum

 * DIAGNOSTICS
 *	If no travel-time information is available for datum, flag as such.

 * FILES
 *	None.

 * NOTES
 *	Change data_phase_type length check when FORTRAN to C conversion 
 *	is complete.

 * SEE ALSO
 *       

 * AUTHOR
 *	Walt Nagy, May 1991.
 */


#include <stdio.h>
#include <string.h>

#ifdef SCCSID
static char	SccsId[] = "@(#)check_data_.c	44.1	9/20/91";
#endif

void
check_data__ (data_sta_id, data_phase_type, data_type, data_std_err, num_data,
	     sta_id, num_sta, phase_type, num_phase_types, sta_len, phase_len,
	     sta_index, ipwav, idtyp, data_error_code)

int	*num_data, *num_phase_types, *num_sta, *sta_len, *phase_len;
int	data_error_code[], sta_index[];
int	ipwav[], idtyp[];
float	data_std_err[];
char	*data_phase_type, *data_sta_id, *data_type, *phase_type, *sta_id;
{

	int	i, j, n, len_d_p_t, len_pt, longer;
	int	len_d_t = 4, len_sta = *sta_len, len_p_t = *phase_len;

	for (n = 0; n < *num_data; n++)
	{
		sta_index[n] = 0;
		ipwav[n] = 0;
		idtyp[n] = 0;

		/* Check that the datum's station is valid */

		for (i = 0; i < *num_sta; i++)
		{
			if (! strncmp(data_sta_id + n*len_sta,
				      sta_id + i*len_sta, len_sta))
			{
				data_error_code[n] = 0;
				sta_index[n] = i + 1;

				for (len_d_p_t = 0; len_d_p_t < len_p_t; len_d_p_t++)
				{
					if (! strncmp (data_phase_type + n*len_p_t + len_d_p_t, " ", 1))
						break;
				}

				for (j = 0; j < *num_phase_types; j++)
				{
					for (len_pt = 0; len_pt < len_p_t; len_pt++)
					{
						if (! strncmp (phase_type + j*len_p_t + len_pt, "\0", 1))
							break;
					}
					if (len_d_p_t > len_pt)
						longer = len_d_p_t;
					else
						longer = len_pt;

					if (! strncmp(data_phase_type + n*len_p_t, phase_type + j*len_p_t, longer))
					{
						ipwav[n] = j + 1;

						/* Check for valid datum type */

						if (! strncmp (data_type + n*len_d_t, "t", 1))
							idtyp[n] = 1;
						else if (! strncmp (data_type + n*len_d_t, "a", 1))
							idtyp[n] = 2;
						else if (! strncmp (data_type + n*len_d_t, "s", 1))
							idtyp[n] = 3;
						else
						{
							idtyp[n] = 0;
							data_error_code[n] = 3;
						}
						if (data_std_err[n] < 0.0)
							data_error_code[n] = 4;
						goto done;
					}
				}

				/* The phase ID is invalid ! */

				if (! strncmp (data_type + n*len_d_t,
					       "t", 1))
				{
					idtyp[n] = 1;
					data_error_code[n] = 2;
					goto done;
				}
				else if (! strncmp (data_type + n*len_d_t,
					            "s", 1))
				{
					idtyp[n] = 3;
					data_error_code[n] = 2;
				}
				else if (! strncmp (data_type + n*len_d_t,
						    "a", 1))
					idtyp[n] = 2;
				else
				{
					idtyp[n] = 0;
					data_error_code[n] = 3;
				}
				/* Check that datum's a priori 
				   standard deviation is vaild */

				if (data_std_err[n] < 0.0)
					data_error_code[n] = 4;
				goto done;
			}
			else
				data_error_code[n] = 1;
		}
done:		;
	}
}


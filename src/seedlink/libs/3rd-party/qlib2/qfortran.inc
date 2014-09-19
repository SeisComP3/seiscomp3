c**********************************************************************
c	Fortran interludes to qlib2 routines.
c**********************************************************************

c	Fortran interludes to ms_pack routines.
	integer f_ms_pack_data, f_ms_pack_update_hdr
	external f_ms_pack_data, f_ms_pack_update_hdr

c	Fortran interludes to ms_utils routines.
	integer f_read_ms
	external f_read_ms

c	Fortran interludes to qtime routines.
	double precision f_dsamples_in_time, f_tdiff
	integer f_mdy_to_doy, f_parse_date
	external f_dsamples_in_time, f_tdiff, f_mdy_to_doy, f_parse_date
	external f_add_time, f_time_interval2, f_int_to_ext, f_ext_to_int
	external f_time_to_str, f_dy_to_mdy

c	Fortran interludes to qutils routines.
	integer f_get_my_wordorder, f_set_hdr_wordorder
	integer f_set_data_wordorder
	external f_get_my_wordorder, f_set_hdr_wordorder
	external f_set_data_wordorder

c	Fortran interludes to sdr_utils routines.
	external f_init_data_hdr, f_delete_blockette, f_copy_data_hdr

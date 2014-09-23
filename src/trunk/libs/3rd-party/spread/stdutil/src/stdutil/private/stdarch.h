/* Copyright (c) 2000-2009, The Johns Hopkins University
 * All rights reserved.
 *
 * The contents of this file are subject to a license (the ``License'').
 * You may not use this file except in compliance with the License. The
 * specific language governing the rights and limitations of the License
 * can be found in the file ``STDUTIL_LICENSE'' found in this
 * distribution.
 *
 * Software distributed under the License is distributed on an AS IS
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *
 * The Original Software is:
 *     The Stdutil Library
 *
 * Contributors:
 *     Creator - John Lane Schultz (jschultz@cnds.jhu.edu)
 *     The Center for Networking and Distributed Systems
 *         (CNDS - http://www.cnds.jhu.edu)
 */

#ifndef stdarch_h_2005_07_12_00_51_28_jschultz_at_cnds_jhu_edu
#define stdarch_h_2005_07_12_00_51_28_jschultz_at_cnds_jhu_edu

#ifdef _WIN32
#  include <stdutil/private/stdarch_wintel32.h>
#else
#  include <stdutil/private/stdarch_autoconf.h>
#endif

#endif

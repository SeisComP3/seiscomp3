/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include <stdio.h>
#include "f2c.h"
#define ftnlen int

/* Subroutine */ int elpcor_(phid, del, z__, azi, ecolat, ecorr, phid_len)
const char *phid;
real *del, *z__, *azi, *ecolat, *ecorr;
ftnlen phid_len;
{
    /* Initialized data */
/*fprintf(stderr, "called elpcor_ %s %.3f %.1f %.3f %.3f %d\n", phid,*del,*z__,*azi, *ecolat, phid_len);
 */
    static real degrad = (float).0174532925;
    static real depth[8] = { (float)0.,(float)35.,(float)50.,(float)100.,(
	    float)200.,(float)300.,(float)500.,(float)700. };
    static real delinc = (float)5.;
    static real delmin[11] = { (float)0.,(float)0.,(float)145.,(float)145.,(
	    float)115.,(float)0.,(float)0.,(float)65.,(float)105.,(float)0.,(
	    float)110. };
    static real delmax[11] = { (float)100.,(float)90.,(float)175.,(float)155.,
	    (float)180.,(float)100.,(float)90.,(float)140.,(float)180.,(float)
	    60.,(float)180. };
    static struct {
	real e_1[561];
	integer fill_2[6];
	real e_3[57];
	integer fill_4[6];
	real e_5[57];
	integer fill_6[6];
	real e_7[57];
	integer fill_8[6];
	real e_9[57];
	integer fill_10[6];
	real e_11[57];
	integer fill_12[6];
	real e_13[57];
	integer fill_14[6];
	real e_15[57];
	integer fill_16[6];
	real e_17[21];
	integer fill_18[42];
	real e_19[21];
	integer fill_20[42];
	real e_21[21];
	integer fill_22[42];
	real e_23[21];
	integer fill_24[42];
	real e_25[21];
	integer fill_26[42];
	real e_27[21];
	integer fill_28[42];
	real e_29[21];
	integer fill_30[42];
	real e_31[21];
	integer fill_32[42];
	real e_33[9];
	integer fill_34[54];
	real e_35[9];
	integer fill_36[54];
	real e_37[9];
	integer fill_38[54];
	real e_39[9];
	integer fill_40[54];
	real e_41[9];
	integer fill_42[54];
	real e_43[9];
	integer fill_44[54];
	real e_45[9];
	integer fill_46[54];
	real e_47[9];
	integer fill_48[54];
	real e_49[42];
	integer fill_50[21];
	real e_51[42];
	integer fill_52[21];
	real e_53[42];
	integer fill_54[21];
	real e_55[42];
	integer fill_56[21];
	real e_57[42];
	integer fill_58[21];
	real e_59[42];
	integer fill_60[21];
	real e_61[42];
	integer fill_62[21];
	real e_63[42];
	integer fill_64[21];
	real e_65[561];
	integer fill_66[6];
	real e_67[57];
	integer fill_68[6];
	real e_69[57];
	integer fill_70[6];
	real e_71[57];
	integer fill_72[6];
	real e_73[57];
	integer fill_74[6];
	real e_75[57];
	integer fill_76[6];
	real e_77[57];
	integer fill_78[6];
	real e_79[57];
	integer fill_80[6];
	real e_81[48];
	integer fill_82[15];
	real e_83[48];
	integer fill_84[15];
	real e_85[48];
	integer fill_86[15];
	real e_87[48];
	integer fill_88[15];
	real e_89[48];
	integer fill_90[15];
	real e_91[48];
	integer fill_92[15];
	real e_93[48];
	integer fill_94[15];
	real e_95[48];
	integer fill_96[15];
	real e_97[48];
	integer fill_98[15];
	real e_99[48];
	integer fill_100[15];
	real e_101[48];
	integer fill_102[15];
	real e_103[48];
	integer fill_104[15];
	real e_105[48];
	integer fill_106[15];
	real e_107[48];
	integer fill_108[15];
	real e_109[48];
	integer fill_110[15];
	real e_111[48];
	integer fill_112[15];
	real e_113[39];
	integer fill_114[24];
	real e_115[39];
	integer fill_116[24];
	real e_117[39];
	integer fill_118[24];
	real e_119[39];
	integer fill_120[24];
	real e_121[39];
	integer fill_122[24];
	real e_123[39];
	integer fill_124[24];
	real e_125[39];
	integer fill_126[24];
	real e_127[39];
	integer fill_128[24];
	real e_129[45];
	integer fill_130[18];
	real e_131[45];
	integer fill_132[18];
	real e_133[45];
	integer fill_134[18];
	real e_135[45];
	integer fill_136[18];
	real e_137[45];
	integer fill_138[18];
	real e_139[45];
	integer fill_140[18];
	real e_141[45];
	integer fill_142[18];
	real e_143[45];
	integer fill_144[18];
	} equiv_23 = { (float)0., (float)0., (float)0., (float)-.18, (float)
		-.01, (float)0., (float)-.32, (float)-.05, (float)-.01, (
		float)-.46, (float)-.1, (float)-.02, (float)-.56, (float)-.16,
		 (float)-.06, (float)-.62, (float)-.21, (float)-.1, (float)
		-.66, (float)-.3, (float)-.17, (float)-.66, (float)-.33, (
		float)-.21, (float)-.66, (float)-.37, (float)-.26, (float)
		-.64, (float)-.39, (float)-.33, (float)-.62, (float)-.4, (
		float)-.4, (float)-.58, (float)-.39, (float)-.47, (float)-.54,
		 (float)-.35, (float)-.54, (float)-.51, (float)-.3, (float)
		-.6, (float)-.48, (float)-.22, (float)-.66, (float)-.45, (
		float)-.12, (float)-.72, (float)-.44, (float)-.01, (float)
		-.76, (float)-.45, (float).12, (float)-.8, (float)-.47, (
		float).24, (float)-.81, (float)-.51, (float).38, (float)-.82, 
		(float)-.55, (float).46, (float)-.81, (float)-.02, (float)0., 
		(float)0., (float)-.16, (float)-.03, (float)0., (float)-.31, (
		float)-.07, (float)-.01, (float)-.43, (float)-.12, (float)
		-.02, (float)-.54, (float)-.18, (float)-.06, (float)-.6, (
		float)-.22, (float)-.1, (float)-.64, (float)-.31, (float)-.17,
		 (float)-.64, (float)-.34, (float)-.21, (float)-.64, (float)
		-.38, (float)-.26, (float)-.62, (float)-.4, (float)-.33, (
		float)-.6, (float)-.41, (float)-.4, (float)-.56, (float)-.39, 
		(float)-.47, (float)-.52, (float)-.36, (float)-.54, (float)
		-.49, (float)-.3, (float)-.6, (float)-.45, (float)-.22, (
		float)-.67, (float)-.43, (float)-.13, (float)-.72, (float)
		-.42, (float)-.01, (float)-.76, (float)-.43, (float).12, (
		float)-.8, (float)-.45, (float).23, (float)-.81, (float)-.49, 
		(float).37, (float)-.82, (float)-.53, (float).45, (float)-.81,
		 (float)-.03, (float)0., (float)0., (float)-.16, (float)-.04, 
		(float)0., (float)-.31, (float)-.07, (float)-.01, (float)-.44,
		 (float)-.13, (float)-.03, (float)-.53, (float)-.18, (float)
		-.06, (float)-.59, (float)-.22, (float)-.1, (float)-.63, (
		float)-.31, (float)-.17, (float)-.64, (float)-.35, (float)
		-.21, (float)-.63, (float)-.38, (float)-.26, (float)-.62, (
		float)-.41, (float)-.33, (float)-.59, (float)-.41, (float)-.4,
		 (float)-.55, (float)-.4, (float)-.47, (float)-.52, (float)
		-.36, (float)-.54, (float)-.48, (float)-.3, (float)-.6, (
		float)-.45, (float)-.22, (float)-.67, (float)-.43, (float)
		-.13, (float)-.72, (float)-.42, (float)-.01, (float)-.76, (
		float)-.42, (float).12, (float)-.8, (float)-.44, (float).23, (
		float)-.81, (float)-.48, (float).37, (float)-.82, (float)-.52,
		 (float).45, (float)-.81, (float)-.05, (float)0., (float)0., (
		float)-.17, (float)-.04, (float)0., (float)-.3, (float)-.09, (
		float)-.01, (float)-.42, (float)-.14, (float)-.03, (float)
		-.52, (float)-.2, (float)-.07, (float)-.57, (float)-.23, (
		float)-.1, (float)-.61, (float)-.32, (float)-.17, (float)-.62,
		 (float)-.36, (float)-.21, (float)-.61, (float)-.39, (float)
		-.27, (float)-.6, (float)-.42, (float)-.33, (float)-.57, (
		float)-.42, (float)-.4, (float)-.53, (float)-.4, (float)-.47, 
		(float)-.5, (float)-.37, (float)-.54, (float)-.46, (float)
		-.31, (float)-.61, (float)-.43, (float)-.23, (float)-.67, (
		float)-.4, (float)-.13, (float)-.72, (float)-.39, (float)-.02,
		 (float)-.77, (float)-.4, (float).11, (float)-.8, (float)-.42,
		 (float).23, (float)-.82, (float)-.46, (float).37, (float)
		-.82, (float)-.5, (float).44, (float)-.82, (float)-.09, (
		float)0., (float)0., (float)-.18, (float)-.08, (float)0., (
		float)-.29, (float)-.14, (float)-.02, (float)-.4, (float)-.18,
		 (float)-.04, (float)-.49, (float)-.23, (float)-.07, (float)
		-.53, (float)-.26, (float)-.1, (float)-.57, (float)-.34, (
		float)-.17, (float)-.58, (float)-.38, (float)-.21, (float)
		-.57, (float)-.42, (float)-.27, (float)-.56, (float)-.44, (
		float)-.33, (float)-.53, (float)-.44, (float)-.4, (float)-.49,
		 (float)-.42, (float)-.47, (float)-.45, (float)-.38, (float)
		-.54, (float)-.41, (float)-.32, (float)-.61, (float)-.38, (
		float)-.24, (float)-.67, (float)-.36, (float)-.14, (float)
		-.72, (float)-.35, (float)-.03, (float)-.77, (float)-.35, (
		float).1, (float)-.8, (float)-.37, (float).21, (float)-.82, (
		float)-.42, (float).36, (float)-.82, (float)-.45, (float).43, 
		(float)-.82, (float)-.13, (float)0., (float)0., (float)-.2, (
		float)-.11, (float)0., (float)-.29, (float)-.18, (float)-.02, 
		(float)-.39, (float)-.22, (float)-.04, (float)-.46, (float)
		-.27, (float)-.07, (float)-.51, (float)-.31, (float)-.11, (
		float)-.54, (float)-.37, (float)-.17, (float)-.55, (float)
		-.41, (float)-.22, (float)-.54, (float)-.44, (float)-.27, (
		float)-.52, (float)-.46, (float)-.34, (float)-.49, (float)
		-.46, (float)-.41, (float)-.45, (float)-.44, (float)-.48, (
		float)-.41, (float)-.4, (float)-.55, (float)-.37, (float)-.34,
		 (float)-.61, (float)-.34, (float)-.26, (float)-.67, (float)
		-.32, (float)-.15, (float)-.73, (float)-.31, (float)-.04, (
		float)-.77, (float)-.31, (float).09, (float)-.8, (float)-.33, 
		(float).22, (float)-.82, (float)-.38, (float).35, (float)-.82,
		 (float)-.4, (float).41, (float)-.82, (float)-.2, (float)0., (
		float)0., (float)-.25, (float)-.14, (float)-.01, (float)-.34, 
		(float)-.27, (float)-.04, (float)-.37, (float)-.29, (float)
		-.05, (float)-.41, (float)-.3, (float)-.07, (float)-.47, (
		float)-.39, (float)-.14, (float)-.48, (float)-.42, (float)
		-.17, (float)-.48, (float)-.46, (float)-.22, (float)-.47, (
		float)-.49, (float)-.28, (float)-.45, (float)-.5, (float)-.35,
		 (float)-.42, (float)-.5, (float)-.42, (float)-.38, (float)
		-.48, (float)-.49, (float)-.34, (float)-.43, (float)-.56, (
		float)-.3, (float)-.37, (float)-.62, (float)-.27, (float)-.28,
		 (float)-.68, (float)-.24, (float)-.18, (float)-.74, (float)
		-.23, (float)-.05, (float)-.78, (float)-.24, (float).08, (
		float)-.81, (float)-.26, (float).22, (float)-.83, (float)-.3, 
		(float).34, (float)-.83, (float)-.32, (float).38, (float)-.83,
		 (float)-.27, (float)0., (float)0., (float)-.3, (float)-.2, (
		float)-.01, (float)-.37, (float)-.34, (float)-.06, (float)
		-.33, (float)-.3, (float)-.04, (float)-.41, (float)-.4, (
		float)-.1, (float)-.43, (float)-.43, (float)-.13, (float)-.44,
		 (float)-.47, (float)-.18, (float)-.44, (float)-.51, (float)
		-.23, (float)-.42, (float)-.53, (float)-.29, (float)-.39, (
		float)-.54, (float)-.36, (float)-.36, (float)-.54, (float)
		-.43, (float)-.32, (float)-.51, (float)-.5, (float)-.27, (
		float)-.46, (float)-.57, (float)-.23, (float)-.39, (float)
		-.63, (float)-.2, (float)-.3, (float)-.69, (float)-.18, (
		float)-.19, (float)-.74, (float)-.16, (float)-.07, (float)
		-.79, (float)-.17, (float).08, (float)-.82, (float)-.19, (
		float).2, (float)-.83, (float)-.23, (float).32, (float)-.83, (
		float)-.24, (float).34, (float)-.83, (float)-1.5, (float)0., (
		float)0., (float)-1.49, (float)-.11, (float)-.01, (float)
		-1.46, (float)-.22, (float)-.03, (float)-1.41, (float)-.32, (
		float)-.06, (float)-1.34, (float)-.4, (float)-.1, (float)
		-1.26, (float)-.47, (float)-.16, (float)-1.17, (float)-.52, (
		float)-.22, (float)-1.07, (float)-.55, (float)-.28, (float)
		-.97, (float)-.56, (float)-.35, (float)-.88, (float)-.55, (
		float)-.42, (float)-.78, (float)-.51, (float)-.49, (float)
		-.69, (float)-.46, (float)-.56, (float)-.62, (float)-.39, (
		float)-.62, (float)-.55, (float)-.3, (float)-.67, (float)-.5, 
		(float)-.2, (float)-.72, (float)-.47, (float)-.09, (float)
		-.76, (float)-.45, (float).03, (float)-.79, (float)-.45, (
		float).15, (float)-.81, (float)-.47, (float).27, (float)-.82, 
		{0}, (float)-1.48, (float)0., (float)0., (float)-1.47, (float)
		-.11, (float)-.01, (float)-1.44, (float)-.22, (float)-.03, (
		float)-1.39, (float)-.32, (float)-.06, (float)-1.32, (float)
		-.41, (float)-.1, (float)-1.24, (float)-.47, (float)-.16, (
		float)-1.15, (float)-.53, (float)-.22, (float)-1.05, (float)
		-.55, (float)-.28, (float)-.95, (float)-.56, (float)-.35, (
		float)-.86, (float)-.55, (float)-.42, (float)-.76, (float)
		-.52, (float)-.49, (float)-.67, (float)-.46, (float)-.56, (
		float)-.6, (float)-.39, (float)-.62, (float)-.53, (float)-.3, 
		(float)-.67, (float)-.48, (float)-.21, (float)-.72, (float)
		-.45, (float)-.1, (float)-.76, (float)-.43, (float).02, (
		float)-.79, (float)-.43, (float).14, (float)-.81, (float)-.45,
		 (float).26, (float)-.82, {0}, (float)-1.47, (float)0., (
		float)0., (float)-1.46, (float)-.11, (float)-.01, (float)
		-1.43, (float)-.22, (float)-.03, (float)-1.38, (float)-.32, (
		float)-.06, (float)-1.32, (float)-.41, (float)-.1, (float)
		-1.24, (float)-.48, (float)-.16, (float)-1.14, (float)-.53, (
		float)-.22, (float)-1.05, (float)-.56, (float)-.28, (float)
		-.95, (float)-.56, (float)-.35, (float)-.85, (float)-.55, (
		float)-.42, (float)-.75, (float)-.52, (float)-.49, (float)
		-.67, (float)-.46, (float)-.56, (float)-.59, (float)-.39, (
		float)-.62, (float)-.52, (float)-.31, (float)-.67, (float)
		-.47, (float)-.21, (float)-.72, (float)-.44, (float)-.1, (
		float)-.76, (float)-.42, (float).02, (float)-.79, (float)-.43,
		 (float).14, (float)-.81, (float)-.45, (float).26, (float)
		-.82, {0}, (float)-1.45, (float)0., (float)0., (float)-1.44, (
		float)-.11, (float)-.01, (float)-1.41, (float)-.22, (float)
		-.03, (float)-1.36, (float)-.32, (float)-.06, (float)-1.29, (
		float)-.41, (float)-.1, (float)-1.21, (float)-.48, (float)
		-.16, (float)-1.12, (float)-.53, (float)-.22, (float)-1.03, (
		float)-.56, (float)-.28, (float)-.93, (float)-.57, (float)
		-.35, (float)-.83, (float)-.55, (float)-.42, (float)-.73, (
		float)-.52, (float)-.49, (float)-.64, (float)-.47, (float)
		-.56, (float)-.57, (float)-.4, (float)-.62, (float)-.5, (
		float)-.31, (float)-.68, (float)-.45, (float)-.21, (float)
		-.72, (float)-.42, (float)-.1, (float)-.76, (float)-.4, (
		float).02, (float)-.79, (float)-.4, (float).14, (float)-.81, (
		float)-.42, (float).26, (float)-.82, {0}, (float)-1.41, (
		float)0., (float)0., (float)-1.4, (float)-.11, (float)-.01, (
		float)-1.37, (float)-.22, (float)-.03, (float)-1.32, (float)
		-.32, (float)-.06, (float)-1.25, (float)-.41, (float)-.1, (
		float)-1.17, (float)-.48, (float)-.16, (float)-1.08, (float)
		-.53, (float)-.22, (float)-.98, (float)-.56, (float)-.28, (
		float)-.88, (float)-.57, (float)-.35, (float)-.78, (float)
		-.56, (float)-.42, (float)-.69, (float)-.53, (float)-.49, (
		float)-.6, (float)-.48, (float)-.56, (float)-.52, (float)-.4, 
		(float)-.62, (float)-.46, (float)-.32, (float)-.68, (float)
		-.41, (float)-.22, (float)-.72, (float)-.37, (float)-.11, (
		float)-.76, (float)-.36, (float).01, (float)-.79, (float)-.36,
		 (float).13, (float)-.81, (float)-.38, (float).25, (float)
		-.82, {0}, (float)-1.37, (float)0., (float)0., (float)-1.36, (
		float)-.12, (float)-.01, (float)-1.33, (float)-.23, (float)
		-.03, (float)-1.28, (float)-.33, (float)-.06, (float)-1.21, (
		float)-.42, (float)-.1, (float)-1.13, (float)-.49, (float)
		-.16, (float)-1.04, (float)-.54, (float)-.22, (float)-.94, (
		float)-.57, (float)-.29, (float)-.84, (float)-.58, (float)
		-.36, (float)-.74, (float)-.57, (float)-.43, (float)-.65, (
		float)-.54, (float)-.49, (float)-.56, (float)-.48, (float)
		-.56, (float)-.48, (float)-.41, (float)-.62, (float)-.42, (
		float)-.33, (float)-.68, (float)-.37, (float)-.23, (float)
		-.73, (float)-.33, (float)-.12, (float)-.77, (float)-.32, (
		float)0., (float)-.8, (float)-.32, (float).12, (float)-.82, (
		float)-.34, (float).24, (float)-.82, {0}, (float)-1.3, (float)
		0., (float)0., (float)-1.29, (float)-.12, (float)-.01, (float)
		-1.25, (float)-.23, (float)-.03, (float)-1.2, (float)-.33, (
		float)-.06, (float)-1.14, (float)-.42, (float)-.11, (float)
		-1.05, (float)-.5, (float)-.16, (float)-.96, (float)-.55, (
		float)-.22, (float)-.87, (float)-.58, (float)-.29, (float)
		-.77, (float)-.59, (float)-.36, (float)-.67, (float)-.58, (
		float)-.43, (float)-.57, (float)-.55, (float)-.5, (float)-.48,
		 (float)-.5, (float)-.56, (float)-.41, (float)-.43, (float)
		-.63, (float)-.34, (float)-.34, (float)-.68, (float)-.29, (
		float)-.24, (float)-.73, (float)-.26, (float)-.13, (float)
		-.77, (float)-.24, (float)-.02, (float)-.8, (float)-.24, (
		float).1, (float)-.82, (float)-.26, (float).22, (float)-.83, {
		0}, (float)-1.23, (float)0., (float)0., (float)-1.22, (float)
		-.12, (float)-.01, (float)-1.19, (float)-.24, (float)-.03, (
		float)-1.14, (float)-.34, (float)-.06, (float)-1.07, (float)
		-.43, (float)-.11, (float)-.99, (float)-.51, (float)-.16, (
		float)-.9, (float)-.56, (float)-.22, (float)-.8, (float)-.59, 
		(float)-.29, (float)-.7, (float)-.61, (float)-.36, (float)-.6,
		 (float)-.6, (float)-.43, (float)-.5, (float)-.56, (float)-.5,
		 (float)-.42, (float)-.51, (float)-.57, (float)-.34, (float)
		-.44, (float)-.63, (float)-.27, (float)-.36, (float)-.69, (
		float)-.22, (float)-.26, (float)-.73, (float)-.19, (float)
		-.15, (float)-.77, (float)-.17, (float)-.03, (float)-.8, (
		float)-.17, (float).09, (float)-.82, (float)-.19, (float).21, 
		(float)-.83, {0}, (float)-1.88, (float).99, (float)-.42, (
		float)-1.94, (float).85, (float)-.39, (float)-2.01, (float)
		.72, (float)-.37, (float)-2.08, (float).59, (float)-.35, (
		float)-2.13, (float).45, (float)-.34, (float)-2.17, (float).3,
		 (float)-.34, (float)-2.19, (float).15, (float)-.35, {0}, (
		float)-1.85, (float).99, (float)-.42, (float)-1.92, (float)
		.85, (float)-.4, (float)-1.99, (float).72, (float)-.37, (
		float)-2.06, (float).59, (float)-.35, (float)-2.11, (float)
		.45, (float)-.34, (float)-2.15, (float).3, (float)-.34, (
		float)-2.17, (float).15, (float)-.35, {0}, (float)-1.85, (
		float).99, (float)-.42, (float)-1.91, (float).85, (float)-.4, 
		(float)-1.98, (float).72, (float)-.37, (float)-2.05, (float)
		.59, (float)-.35, (float)-2.1, (float).45, (float)-.34, (
		float)-2.14, (float).3, (float)-.34, (float)-2.16, (float).15,
		 (float)-.35, {0}, (float)-1.82, (float).98, (float)-.42, (
		float)-1.88, (float).84, (float)-.4, (float)-1.96, (float).72,
		 (float)-.37, (float)-2.03, (float).58, (float)-.35, (float)
		-2.08, (float).44, (float)-.34, (float)-2.12, (float).3, (
		float)-.34, (float)-2.14, (float).14, (float)-.35, {0}, (
		float)-1.77, (float).97, (float)-.43, (float)-1.84, (float)
		.84, (float)-.4, (float)-1.92, (float).71, (float)-.37, (
		float)-1.98, (float).58, (float)-.35, (float)-2.04, (float)
		.44, (float)-.34, (float)-2.07, (float).29, (float)-.34, (
		float)-2.09, (float).14, (float)-.35, {0}, (float)-1.72, (
		float).96, (float)-.43, (float)-1.79, (float).83, (float)-.4, 
		(float)-1.87, (float).7, (float)-.38, (float)-1.94, (float)
		.57, (float)-.36, (float)-1.99, (float).43, (float)-.35, (
		float)-2.03, (float).28, (float)-.34, (float)-2.05, (float)
		.13, (float)-.36, {0}, (float)-1.64, (float).94, (float)-.44, 
		(float)-1.71, (float).82, (float)-.41, (float)-1.79, (float)
		.69, (float)-.38, (float)-1.86, (float).56, (float)-.36, (
		float)-1.92, (float).42, (float)-.35, (float)-1.96, (float)
		.27, (float)-.35, (float)-.32, (float).38, (float)-.83, {0}, (
		float)-1.55, (float).93, (float)-.45, (float)-1.64, (float).8,
		 (float)-.42, (float)-1.72, (float).68, (float)-.39, (float)
		-1.79, (float).55, (float)-.37, (float)-1.85, (float).41, (
		float)-.36, (float)-1.89, (float).26, (float)-.35, (float)
		-1.89, (float).26, (float)-.35, {0}, (float)-1.93, (float)
		1.03, (float)-.39, (float)-2.15, (float).99, (float)-.29, (
		float)-2.32, (float).89, (float)-.21, {0}, (float)-1.92, (
		float)1.03, (float)-.39, (float)-2.13, (float).99, (float)
		-.29, (float)-2.3, (float).89, (float)-.21, {0}, (float)-1.91,
		 (float)1.03, (float)-.39, (float)-2.12, (float).99, (float)
		-.29, (float)-2.29, (float).89, (float)-.21, {0}, (float)
		-1.89, (float)1.04, (float)-.39, (float)-2.1, (float).99, (
		float)-.29, (float)-2.27, (float).89, (float)-.21, {0}, (
		float)-1.85, (float)1.04, (float)-.39, (float)-2.06, (float)
		.99, (float)-.29, (float)-2.23, (float).88, (float)-.21, {0}, 
		(float)-1.81, (float)1.04, (float)-.39, (float)-2.02, (float)
		.98, (float)-.29, (float)-2.19, (float).88, (float)-.21, {0}, 
		(float)-1.75, (float)1.04, (float)-.38, (float)-1.95, (float)
		.98, (float)-.29, (float)-1.95, (float).98, (float)-.29, {0}, 
		(float)-1.69, (float)1.05, (float)-.38, (float)-1.88, (float)
		.99, (float)-.29, (float)-1.88, (float).99, (float)-.29, {0}, 
		(float)-1.03, (float).98, (float)-.87, (float)-1.18, (float)
		1.09, (float)-.8, (float)-1.35, (float)1.16, (float)-.71, (
		float)-1.52, (float)1.2, (float)-.62, (float)-1.69, (float)
		1.2, (float)-.53, (float)-1.87, (float)1.17, (float)-.44, (
		float)-2.03, (float)1.11, (float)-.35, (float)-2.19, (float)
		1.02, (float)-.27, (float)-2.34, (float).9, (float)-.19, (
		float)-2.46, (float).75, (float)-.13, (float)-2.56, (float)
		.58, (float)-.07, (float)-2.64, (float).4, (float)-.03, (
		float)-2.69, (float).2, (float)-.01, (float)-2.7, (float)0., (
		float)0., {0}, (float)-1.01, (float).98, (float)-.87, (float)
		-1.16, (float)1.08, (float)-.8, (float)-1.33, (float)1.16, (
		float)-.71, (float)-1.5, (float)1.19, (float)-.62, (float)
		-1.67, (float)1.2, (float)-.53, (float)-1.85, (float)1.17, (
		float)-.44, (float)-2.01, (float)1.11, (float)-.35, (float)
		-2.17, (float)1.02, (float)-.27, (float)-2.32, (float).9, (
		float)-.19, (float)-2.44, (float).75, (float)-.13, (float)
		-2.54, (float).58, (float)-.07, (float)-2.62, (float).4, (
		float)-.03, (float)-2.67, (float).2, (float)-.01, (float)
		-2.68, (float)0., (float)0., {0}, (float)-1.01, (float).98, (
		float)-.87, (float)-1.16, (float)1.08, (float)-.8, (float)
		-1.32, (float)1.16, (float)-.71, (float)-1.49, (float)1.19, (
		float)-.62, (float)-1.67, (float)1.2, (float)-.53, (float)
		-1.84, (float)1.17, (float)-.44, (float)-2.01, (float)1.11, (
		float)-.35, (float)-2.16, (float)1.02, (float)-.27, (float)
		-2.31, (float).9, (float)-.19, (float)-2.44, (float).75, (
		float)-.13, (float)-2.54, (float).58, (float)-.07, (float)
		-2.61, (float).4, (float)-.03, (float)-2.66, (float).2, (
		float)-.01, (float)-2.68, (float)0., (float)0., {0}, (float)
		-.99, (float).98, (float)-.87, (float)-1.13, (float)1.08, (
		float)-.8, (float)-1.3, (float)1.15, (float)-.71, (float)
		-1.47, (float)1.19, (float)-.62, (float)-1.64, (float)1.2, (
		float)-.53, (float)-1.82, (float)1.17, (float)-.44, (float)
		-1.99, (float)1.11, (float)-.35, (float)-2.14, (float)1.02, (
		float)-.27, (float)-2.29, (float).9, (float)-.19, (float)
		-2.41, (float).75, (float)-.13, (float)-2.52, (float).58, (
		float)-.07, (float)-2.59, (float).4, (float)-.03, (float)
		-2.64, (float).2, (float)-.01, (float)-2.66, (float)0., (
		float)0., {0}, (float)-.94, (float).98, (float)-.87, (float)
		-1.09, (float)1.08, (float)-.8, (float)-1.25, (float)1.15, (
		float)-.71, (float)-1.43, (float)1.19, (float)-.62, (float)
		-1.6, (float)1.2, (float)-.53, (float)-1.77, (float)1.17, (
		float)-.44, (float)-1.94, (float)1.11, (float)-.35, (float)
		-2.1, (float)1.02, (float)-.27, (float)-2.25, (float).9, (
		float)-.19, (float)-2.37, (float).75, (float)-.13, (float)
		-2.47, (float).58, (float)-.07, (float)-2.55, (float).4, (
		float)-.03, (float)-2.6, (float).2, (float)-.01, (float)-2.61,
		 (float)0., (float)0., {0}, (float)-.9, (float).98, (float)
		-.87, (float)-1.05, (float)1.08, (float)-.8, (float)-1.21, (
		float)1.15, (float)-.71, (float)-1.39, (float)1.19, (float)
		-.62, (float)-1.56, (float)1.19, (float)-.53, (float)-1.73, (
		float)1.17, (float)-.44, (float)-1.9, (float)1.11, (float)
		-.35, (float)-2.06, (float)1.01, (float)-.27, (float)-2.21, (
		float).89, (float)-.19, (float)-2.33, (float).75, (float)-.13,
		 (float)-2.43, (float).58, (float)-.07, (float)-2.51, (float)
		.4, (float)-.03, (float)-2.56, (float).2, (float)-.01, (float)
		-2.57, (float)0., (float)0., {0}, (float)-.83, (float).98, (
		float)-.87, (float)-.98, (float)1.08, (float)-.8, (float)
		-1.14, (float)1.15, (float)-.71, (float)-1.31, (float)1.19, (
		float)-.62, (float)-1.49, (float)1.19, (float)-.53, (float)
		-1.66, (float)1.16, (float)-.44, (float)-1.83, (float)1.1, (
		float)-.35, (float)-1.99, (float)1.01, (float)-.27, (float)
		-2.13, (float).89, (float)-.19, (float)-2.26, (float).75, (
		float)-.13, (float)-2.36, (float).58, (float)-.07, (float)
		-2.44, (float).4, (float)-.03, (float)-2.48, (float).2, (
		float)-.01, (float)-2.5, (float)0., (float)0., {0}, (float)
		-.76, (float).97, (float)-.87, (float)-.91, (float)1.08, (
		float)-.8, (float)-1.07, (float)1.15, (float)-.71, (float)
		-1.25, (float)1.19, (float)-.62, (float)-1.42, (float)1.19, (
		float)-.53, (float)-1.59, (float)1.16, (float)-.44, (float)
		-1.76, (float)1.1, (float)-.35, (float)-1.92, (float)1.01, (
		float)-.27, (float)-2.07, (float).89, (float)-.19, (float)
		-2.19, (float).75, (float)-.13, (float)-2.3, (float).58, (
		float)-.07, (float)-2.37, (float).4, (float)-.03, (float)
		-2.42, (float).2, (float)-.01, (float)-2.43, (float)0., (
		float)0., {0}, (float)0., (float)0., (float)0., (float)-.31, (
		float)-.02, (float)0., (float)-.57, (float)-.09, (float)-.01, 
		(float)-.81, (float)-.19, (float)-.04, (float)-1.02, (float)
		-.3, (float)-.11, (float)-1.15, (float)-.42, (float)-.21, (
		float)-1.19, (float)-.51, (float)-.28, (float)-1.2, (float)
		-.6, (float)-.37, (float)-1.19, (float)-.67, (float)-.47, (
		float)-1.16, (float)-.72, (float)-.59, (float)-1.12, (float)
		-.73, (float)-.71, (float)-1.06, (float)-.71, (float)-.83, (
		float)-.99, (float)-.65, (float)-.96, (float)-.92, (float)
		-.56, (float)-1.08, (float)-.87, (float)-.42, (float)-1.19, (
		float)-.83, (float)-.26, (float)-1.29, (float)-.81, (float)
		-.05, (float)-1.38, (float)-.81, (float).17, (float)-1.44, (
		float)-.86, (float).42, (float)-1.48, (float)-.93, (float).67,
		 (float)-1.5, (float)-1.02, (float).86, (float)-1.49, (float)
		-.03, (float)0., (float)0., (float)-.29, (float)-.05, (float)
		0., (float)-.55, (float)-.12, (float)-.01, (float)-.79, (
		float)-.21, (float)-.04, (float)-.99, (float)-.32, (float)
		-.12, (float)-1.12, (float)-.44, (float)-.21, (float)-1.16, (
		float)-.53, (float)-.28, (float)-1.17, (float)-.62, (float)
		-.37, (float)-1.16, (float)-.69, (float)-.47, (float)-1.13, (
		float)-.73, (float)-.59, (float)-1.08, (float)-.74, (float)
		-.71, (float)-1.02, (float)-.72, (float)-.83, (float)-.96, (
		float)-.67, (float)-.96, (float)-.89, (float)-.57, (float)
		-1.08, (float)-.83, (float)-.43, (float)-1.2, (float)-.79, (
		float)-.26, (float)-1.3, (float)-.77, (float)-.06, (float)
		-1.38, (float)-.78, (float).17, (float)-1.44, (float)-.82, (
		float).42, (float)-1.48, (float)-.9, (float).66, (float)-1.5, 
		(float)-.98, (float).85, (float)-1.49, (float)-.05, (float)0.,
		 (float)0., (float)-.29, (float)-.07, (float)0., (float)-.55, 
		(float)-.13, (float)-.01, (float)-.77, (float)-.22, (float)
		-.04, (float)-.98, (float)-.33, (float)-.12, (float)-1.11, (
		float)-.44, (float)-.21, (float)-1.14, (float)-.54, (float)
		-.28, (float)-1.16, (float)-.62, (float)-.37, (float)-1.15, (
		float)-.69, (float)-.48, (float)-1.12, (float)-.74, (float)
		-.59, (float)-1.07, (float)-.75, (float)-.71, (float)-1.01, (
		float)-.73, (float)-.84, (float)-.94, (float)-.67, (float)
		-.96, (float)-.88, (float)-.57, (float)-1.08, (float)-.82, (
		float)-.44, (float)-1.2, (float)-.78, (float)-.27, (float)
		-1.3, (float)-.76, (float)-.06, (float)-1.38, (float)-.77, (
		float).16, (float)-1.44, (float)-.81, (float).41, (float)
		-1.48, (float)-.88, (float).66, (float)-1.5, (float)-.97, (
		float).85, (float)-1.49, (float)-.08, (float)0., (float)0., (
		float)-.3, (float)-.06, (float)0., (float)-.54, (float)-.17, (
		float)-.02, (float)-.77, (float)-.26, (float)-.04, (float)
		-.95, (float)-.36, (float)-.12, (float)-1.07, (float)-.46, (
		float)-.21, (float)-1.11, (float)-.56, (float)-.28, (float)
		-1.12, (float)-.64, (float)-.38, (float)-1.11, (float)-.71, (
		float)-.48, (float)-1.08, (float)-.76, (float)-.59, (float)
		-1.03, (float)-.77, (float)-.71, (float)-.97, (float)-.75, (
		float)-.84, (float)-.9, (float)-.68, (float)-.97, (float)-.84,
		 (float)-.59, (float)-1.09, (float)-.78, (float)-.45, (float)
		-1.2, (float)-.74, (float)-.28, (float)-1.3, (float)-.72, (
		float)-.07, (float)-1.38, (float)-.73, (float).16, (float)
		-1.45, (float)-.77, (float).41, (float)-1.49, (float)-.84, (
		float).65, (float)-1.5, (float)-.93, (float).83, (float)-1.49,
		 (float)-.16, (float)0., (float)0., (float)-.33, (float)-.14, 
		(float)-.01, (float)-.71, (float)-.33, (float)-.05, (float)
		-.74, (float)-.34, (float)-.06, (float)-.9, (float)-.43, (
		float)-.13, (float)-1., (float)-.5, (float)-.2, (float)-1.04, 
		(float)-.61, (float)-.29, (float)-1.05, (float)-.69, (float)
		-.38, (float)-1.04, (float)-.75, (float)-.48, (float)-1.01, (
		float)-.8, (float)-.6, (float)-.96, (float)-.8, (float)-.72, (
		float)-.9, (float)-.78, (float)-.85, (float)-.83, (float)-.72,
		 (float)-.97, (float)-.76, (float)-.61, (float)-1.09, (float)
		-.7, (float)-.47, (float)-1.21, (float)-.66, (float)-.3, (
		float)-1.31, (float)-.64, (float)-.09, (float)-1.39, (float)
		-.65, (float).14, (float)-1.45, (float)-.69, (float).39, (
		float)-1.49, (float)-.76, (float).64, (float)-1.5, (float)
		-.84, (float).81, (float)-1.5, (float)-.24, (float)0., (float)
		0., (float)-.37, (float)-.2, (float)-.01, (float)-.53, (float)
		-.33, (float)-.03, (float)-.72, (float)-.4, (float)-.08, (
		float)-.88, (float)-.47, (float)-.15, (float)-.94, (float)
		-.56, (float)-.21, (float)-.98, (float)-.66, (float)-.3, (
		float)-.99, (float)-.74, (float)-.39, (float)-.97, (float)-.8,
		 (float)-.49, (float)-.94, (float)-.84, (float)-.61, (float)
		-.89, (float)-.84, (float)-.73, (float)-.82, (float)-.81, (
		float)-.85, (float)-.75, (float)-.75, (float)-.98, (float)
		-.69, (float)-.64, (float)-1.1, (float)-.63, (float)-.5, (
		float)-1.21, (float)-.58, (float)-.32, (float)-1.31, (float)
		-.56, (float)-.11, (float)-1.39, (float)-.57, (float).12, (
		float)-1.46, (float)-.61, (float).38, (float)-1.5, (float)
		-.69, (float).62, (float)-1.51, (float)-.75, (float).78, (
		float)-1.5, (float)-.37, (float)0., (float)0., (float)-.46, (
		float)-.26, (float)-.01, (float)-.6, (float)-.48, (float)-.07,
		 (float)-.67, (float)-.52, (float)-.09, (float)-.8, (float)
		-.59, (float)-.16, (float)-.84, (float)-.67, (float)-.23, (
		float)-.87, (float)-.76, (float)-.31, (float)-.87, (float)
		-.83, (float)-.4, (float)-.86, (float)-.89, (float)-.51, (
		float)-.82, (float)-.92, (float)-.62, (float)-.76, (float)
		-.91, (float)-.74, (float)-.69, (float)-.88, (float)-.87, (
		float)-.62, (float)-.81, (float)-1., (float)-.55, (float)-.69,
		 (float)-1.12, (float)-.49, (float)-.55, (float)-1.23, (float)
		-.45, (float)-.36, (float)-1.33, (float)-.43, (float)-.15, (
		float)-1.41, (float)-.43, (float).09, (float)-1.47, (float)
		-.48, (float).35, (float)-1.51, (float)-.55, (float).6, (
		float)-1.52, (float)-.6, (float).71, (float)-1.51, (float)
		-.49, (float)0., (float)0., (float)-.55, (float)-.28, (float)
		-.01, (float)-.63, (float)-.58, (float)-.09, (float)-.65, (
		float)-.6, (float)-.1, (float)-.73, (float)-.69, (float)-.17, 
		(float)-.77, (float)-.77, (float)-.23, (float)-.79, (float)
		-.86, (float)-.32, (float)-.79, (float)-.92, (float)-.41, (
		float)-.76, (float)-.97, (float)-.52, (float)-.71, (float)
		-.99, (float)-.64, (float)-.65, (float)-.99, (float)-.76, (
		float)-.58, (float)-.94, (float)-.89, (float)-.5, (float)-.86,
		 (float)-1.01, (float)-.43, (float)-.74, (float)-1.13, (float)
		-.37, (float)-.59, (float)-1.25, (float)-.32, (float)-.4, (
		float)-1.34, (float)-.3, (float)-.18, (float)-1.42, (float)
		-.31, (float).07, (float)-1.48, (float)-.35, (float).33, (
		float)-1.52, (float)-.43, (float).57, (float)-1.53, (float)
		-.45, (float).64, (float)-1.52, (float)-2.74, (float)0., (
		float)0., (float)-2.72, (float)-.21, (float)-.01, (float)
		-2.67, (float)-.4, (float)-.05, (float)-2.57, (float)-.58, (
		float)-.11, (float)-2.45, (float)-.74, (float)-.19, (float)
		-2.31, (float)-.86, (float)-.29, (float)-2.14, (float)-.96, (
		float)-.4, (float)-1.96, (float)-1.01, (float)-.52, (float)
		-1.78, (float)-1.03, (float)-.64, (float)-1.6, (float)-1., (
		float)-.77, (float)-1.43, (float)-.94, (float)-.9, (float)
		-1.27, (float)-.84, (float)-1.02, (float)-1.13, (float)-.71, (
		float)-1.13, (float)-1.01, (float)-.56, (float)-1.23, (float)
		-.92, (float)-.38, (float)-1.32, (float)-.86, (float)-.17, (
		float)-1.39, (float)-.83, (float).04, (float)-1.45, (float)
		-.83, (float).26, (float)-1.48, (float)-.87, (float).48, (
		float)-1.5, {0}, (float)-2.71, (float)0., (float)0., (float)
		-2.69, (float)-.21, (float)-.01, (float)-2.63, (float)-.4, (
		float)-.05, (float)-2.54, (float)-.58, (float)-.11, (float)
		-2.42, (float)-.74, (float)-.19, (float)-2.27, (float)-.87, (
		float)-.29, (float)-2.11, (float)-.96, (float)-.4, (float)
		-1.93, (float)-1.01, (float)-.52, (float)-1.75, (float)-1.03, 
		(float)-.64, (float)-1.57, (float)-1.01, (float)-.77, (float)
		-1.4, (float)-.95, (float)-.9, (float)-1.24, (float)-.85, (
		float)-1.02, (float)-1.09, (float)-.72, (float)-1.13, (float)
		-.98, (float)-.56, (float)-1.23, (float)-.88, (float)-.38, (
		float)-1.32, (float)-.82, (float)-.18, (float)-1.39, (float)
		-.79, (float).03, (float)-1.45, (float)-.8, (float).25, (
		float)-1.48, (float)-.83, (float).47, (float)-1.5, {0}, (
		float)-2.7, (float)0., (float)0., (float)-2.68, (float)-.21, (
		float)-.01, (float)-2.62, (float)-.4, (float)-.05, (float)
		-2.53, (float)-.59, (float)-.11, (float)-2.41, (float)-.74, (
		float)-.19, (float)-2.26, (float)-.87, (float)-.29, (float)
		-2.1, (float)-.96, (float)-.4, (float)-1.92, (float)-1.02, (
		float)-.52, (float)-1.74, (float)-1.03, (float)-.64, (float)
		-1.56, (float)-1.01, (float)-.77, (float)-1.38, (float)-.95, (
		float)-.9, (float)-1.22, (float)-.85, (float)-1.02, (float)
		-1.08, (float)-.72, (float)-1.13, (float)-.96, (float)-.57, (
		float)-1.23, (float)-.87, (float)-.38, (float)-1.32, (float)
		-.81, (float)-.18, (float)-1.39, (float)-.78, (float).03, (
		float)-1.45, (float)-.78, (float).25, (float)-1.48, (float)
		-.82, (float).47, (float)-1.5, {0}, (float)-2.66, (float)0., (
		float)0., (float)-2.64, (float)-.21, (float)-.01, (float)
		-2.58, (float)-.41, (float)-.05, (float)-2.49, (float)-.59, (
		float)-.11, (float)-2.37, (float)-.75, (float)-.19, (float)
		-2.22, (float)-.87, (float)-.29, (float)-2.06, (float)-.97, (
		float)-.4, (float)-1.88, (float)-1.02, (float)-.52, (float)
		-1.7, (float)-1.04, (float)-.64, (float)-1.52, (float)-1.02, (
		float)-.77, (float)-1.34, (float)-.95, (float)-.9, (float)
		-1.18, (float)-.86, (float)-1.02, (float)-1.04, (float)-.73, (
		float)-1.13, (float)-.92, (float)-.57, (float)-1.23, (float)
		-.83, (float)-.39, (float)-1.32, (float)-.77, (float)-.19, (
		float)-1.39, (float)-.74, (float).02, (float)-1.45, (float)
		-.74, (float).24, (float)-1.49, (float)-.78, (float).46, (
		float)-1.5, {0}, (float)-2.58, (float)0., (float)0., (float)
		-2.56, (float)-.21, (float)-.01, (float)-2.5, (float)-.41, (
		float)-.05, (float)-2.41, (float)-.59, (float)-.11, (float)
		-2.29, (float)-.75, (float)-.19, (float)-2.14, (float)-.88, (
		float)-.29, (float)-1.98, (float)-.98, (float)-.4, (float)
		-1.8, (float)-1.03, (float)-.52, (float)-1.62, (float)-1.05, (
		float)-.65, (float)-1.44, (float)-1.03, (float)-.77, (float)
		-1.26, (float)-.97, (float)-.9, (float)-1.1, (float)-.87, (
		float)-1.02, (float)-.96, (float)-.74, (float)-1.14, (float)
		-.85, (float)-.59, (float)-1.24, (float)-.75, (float)-.41, (
		float)-1.32, (float)-.69, (float)-.21, (float)-1.4, (float)
		-.66, (float).01, (float)-1.45, (float)-.66, (float).23, (
		float)-1.49, (float)-.7, (float).45, (float)-1.51, {0}, (
		float)-2.51, (float)0., (float)0., (float)-2.49, (float)-.21, 
		(float)-.01, (float)-2.43, (float)-.41, (float)-.05, (float)
		-2.43, (float)-.6, (float)-.11, (float)-2.21, (float)-.76, (
		float)-.19, (float)-2.07, (float)-.89, (float)-.29, (float)
		-1.9, (float)-.99, (float)-.4, (float)-1.72, (float)-1.04, (
		float)-.52, (float)-1.54, (float)-1.06, (float)-.65, (float)
		-1.36, (float)-1.04, (float)-.78, (float)-1.19, (float)-.98, (
		float)-.9, (float)-1.03, (float)-.89, (float)-1.03, (float)
		-.89, (float)-.76, (float)-1.14, (float)-.77, (float)-.6, (
		float)-1.24, (float)-.68, (float)-.42, (float)-1.33, (float)
		-.61, (float)-.22, (float)-1.4, (float)-.58, (float)-.01, (
		float)-1.46, (float)-.59, (float).21, (float)-1.49, (float)
		-.62, (float).43, (float)-1.51, {0}, (float)-2.37, (float)0., 
		(float)0., (float)-2.35, (float)-.22, (float)-.01, (float)
		-2.29, (float)-.42, (float)-.05, (float)-2.2, (float)-.61, (
		float)-.11, (float)-2.08, (float)-.77, (float)-.19, (float)
		-1.93, (float)-.91, (float)-.29, (float)-1.76, (float)-1.01, (
		float)-.4, (float)-1.59, (float)-1.07, (float)-.52, (float)
		-1.4, (float)-1.09, (float)-.65, (float)-1.22, (float)-1.07, (
		float)-.78, (float)-1.05, (float)-1.01, (float)-.91, (float)
		-.89, (float)-.92, (float)-1.03, (float)-.75, (float)-.79, (
		float)-1.14, (float)-.63, (float)-.63, (float)-1.25, (float)
		-.54, (float)-.45, (float)-1.33, (float)-.47, (float)-.25, (
		float)-1.41, (float)-.44, (float)-.04, (float)-1.46, (float)
		-.45, (float).18, (float)-1.5, (float)-.48, (float).4, (float)
		-1.52, {0}, (float)-2.25, (float)0., (float)0., (float)-2.23, 
		(float)-.22, (float)-.01, (float)-2.17, (float)-.43, (float)
		-.05, (float)-2.08, (float)-.62, (float)-.11, (float)-1.96, (
		float)-.79, (float)-.19, (float)-1.81, (float)-.93, (float)
		-.29, (float)-1.64, (float)-1.03, (float)-.41, (float)-1.46, (
		float)-1.09, (float)-.53, (float)-1.28, (float)-1.11, (float)
		-.66, (float)-1.1, (float)-1.09, (float)-.79, (float)-.92, (
		float)-1.04, (float)-.91, (float)-.76, (float)-.94, (float)
		-1.04, (float)-.62, (float)-.82, (float)-1.15, (float)-.5, (
		float)-.66, (float)-1.25, (float)-.41, (float)-.48, (float)
		-1.34, (float)-.35, (float)-.28, (float)-1.42, (float)-.32, (
		float)-.07, (float)-1.47, (float)-.32, (float).15, (float)
		-1.51, (float)-.36, (float).37, (float)-1.53, {0}, (float)
		-1.01, (float)-.55, (float)-1.23, (float)-.92, (float)-.37, (
		float)-1.33, (float)-.86, (float)-.17, (float)-1.4, (float)
		-.83, (float).05, (float)-1.46, (float)-.84, (float).29, (
		float)-1.5, (float)-.88, (float).53, (float)-1.51, (float)
		-.95, (float).77, (float)-1.51, (float)-1.05, (float)1., (
		float)-1.48, (float)-1.2, (float)1.22, (float)-1.42, (float)
		-1.37, (float)1.41, (float)-1.35, (float)-1.57, (float)1.57, (
		float)-1.25, (float)-1.79, (float)1.7, (float)-1.14, (float)
		-2.03, (float)1.78, (float)-1.03, (float)-2.28, (float)1.82, (
		float)-.9, (float)-2.53, (float)1.81, (float)-.77, (float)
		-2.79, (float)1.76, (float)-.63, {0}, (float)-.98, (float)
		-.56, (float)-1.23, (float)-.88, (float)-.37, (float)-1.33, (
		float)-.82, (float)-.17, (float)-1.4, (float)-.8, (float).04, 
		(float)-1.46, (float)-.8, (float).29, (float)-1.5, (float)
		-.84, (float).53, (float)-1.51, (float)-.91, (float).77, (
		float)-1.51, (float)-1.02, (float)1., (float)-1.48, (float)
		-1.16, (float)1.22, (float)-1.42, (float)-1.34, (float)1.41, (
		float)-1.35, (float)-1.53, (float)1.57, (float)-1.25, (float)
		-1.76, (float)1.69, (float)-1.14, (float)-1.99, (float)1.78, (
		float)-1.03, (float)-2.25, (float)1.82, (float)-.9, (float)
		-2.5, (float)1.81, (float)-.77, (float)-2.76, (float)1.76, (
		float)-.63, {0}, (float)-.96, (float)-.56, (float)-1.24, (
		float)-.87, (float)-.38, (float)-1.33, (float)-.81, (float)
		-.18, (float)-1.4, (float)-.78, (float).04, (float)-1.46, (
		float)-.79, (float).28, (float)-1.5, (float)-.83, (float).53, 
		(float)-1.52, (float)-.9, (float).77, (float)-1.51, (float)
		-1.01, (float)1., (float)-1.48, (float)-1.15, (float)1.22, (
		float)-1.42, (float)-1.33, (float)1.41, (float)-1.35, (float)
		-1.52, (float)1.57, (float)-1.25, (float)-1.75, (float)1.69, (
		float)-1.14, (float)-1.98, (float)1.78, (float)-1.03, (float)
		-2.24, (float)1.82, (float)-.9, (float)-2.49, (float)1.81, (
		float)-.77, (float)-2.74, (float)1.76, (float)-.63, {0}, (
		float)-.92, (float)-.57, (float)-1.24, (float)-.83, (float)
		-.38, (float)-1.33, (float)-.77, (float)-.18, (float)-1.4, (
		float)-.74, (float).03, (float)-1.46, (float)-.75, (float).28,
		 (float)-1.5, (float)-.79, (float).52, (float)-1.52, (float)
		-.86, (float).76, (float)-1.51, (float)-.97, (float).99, (
		float)-1.48, (float)-1.11, (float)1.21, (float)-1.42, (float)
		-1.29, (float)1.41, (float)-1.35, (float)-1.48, (float)1.56, (
		float)-1.26, (float)-1.71, (float)1.69, (float)-1.14, (float)
		-1.94, (float)1.77, (float)-1.03, (float)-2.2, (float)1.82, (
		float)-.9, (float)-2.45, (float)1.81, (float)-.77, (float)
		-2.71, (float)1.76, (float)-.63, {0}, (float)-.84, (float)
		-.58, (float)-1.24, (float)-.75, (float)-.4, (float)-1.33, (
		float)-.69, (float)-.2, (float)-1.4, (float)-.67, (float).03, 
		(float)-1.46, (float)-.67, (float).27, (float)-1.5, (float)
		-.71, (float).51, (float)-1.52, (float)-.78, (float).76, (
		float)-1.51, (float)-.89, (float).98, (float)-1.48, (float)
		-1.03, (float)1.2, (float)-1.42, (float)-1.21, (float)1.4, (
		float)-1.35, (float)-1.4, (float)1.56, (float)-1.26, (float)
		-1.63, (float)1.69, (float)-1.15, (float)-1.86, (float)1.77, (
		float)-1.03, (float)-2.12, (float)1.81, (float)-.9, (float)
		-2.37, (float)1.81, (float)-.77, (float)-2.63, (float)1.75, (
		float)-.63, {0}, (float)-.77, (float)-.6, (float)-1.24, (
		float)-.68, (float)-.41, (float)-1.33, (float)-.62, (float)
		-.21, (float)-1.41, (float)-.59, (float).02, (float)-1.47, (
		float)-.6, (float).25, (float)-1.5, (float)-.63, (float).5, (
		float)-1.52, (float)-.71, (float).75, (float)-1.51, (float)
		-.82, (float).98, (float)-1.48, (float)-.96, (float)1.2, (
		float)-1.42, (float)-1.13, (float)1.39, (float)-1.35, (float)
		-1.33, (float)1.55, (float)-1.26, (float)-1.56, (float)1.68, (
		float)-1.15, (float)-1.79, (float)1.77, (float)-1.03, (float)
		-2.05, (float)1.81, (float)-.9, (float)-2.3, (float)1.8, (
		float)-.77, (float)-2.55, (float)1.75, (float)-.63, {0}, (
		float)-.63, (float)-.63, (float)-1.25, (float)-.54, (float)
		-.44, (float)-1.34, (float)-.48, (float)-.23, (float)-1.42, (
		float)-.45, (float)-.01, (float)-1.47, (float)-.45, (float)
		.22, (float)-1.51, (float)-.49, (float).48, (float)-1.53, (
		float)-.57, (float).73, (float)-1.52, (float)-.68, (float).97,
		 (float)-1.48, (float)-.82, (float)1.18, (float)-1.43, (float)
		-.99, (float)1.38, (float)-1.35, (float)-1.19, (float)1.54, (
		float)-1.26, (float)-1.42, (float)1.67, (float)-1.15, (float)
		-1.66, (float)1.76, (float)-1.03, (float)-1.91, (float)1.8, (
		float)-.9, (float)-2.16, (float)1.8, (float)-.77, (float)
		-2.42, (float)1.74, (float)-.63, {0}, (float)-.5, (float)-.65,
		 (float)-1.26, (float)-.41, (float)-.47, (float)-1.35, (float)
		-.35, (float)-.26, (float)-1.42, (float)-.33, (float)-.03, (
		float)-1.48, (float)-.33, (float).22, (float)-1.52, (float)
		-.37, (float).46, (float)-1.53, (float)-.45, (float).71, (
		float)-1.52, (float)-.56, (float).95, (float)-1.49, (float)
		-.7, (float)1.17, (float)-1.43, (float)-.87, (float)1.37, (
		float)-1.36, (float)-1.07, (float)1.53, (float)-1.26, (float)
		-1.3, (float)1.66, (float)-1.15, (float)-1.54, (float)1.75, (
		float)-1.03, (float)-1.79, (float)1.79, (float)-.9, (float)
		-2.05, (float)1.79, (float)-.77, (float)-2.3, (float)1.74, (
		float)-.64, {0}, (float)-1.2, (float)1.12, (float)-1.48, (
		float)-1.37, (float)1.34, (float)-1.4, (float)-1.57, (float)
		1.52, (float)-1.29, (float)-1.79, (float)1.66, (float)-1.17, (
		float)-2.03, (float)1.76, (float)-1.04, (float)-2.28, (float)
		1.81, (float)-.91, (float)-2.53, (float)1.81, (float)-.77, (
		float)-2.78, (float)1.75, (float)-.64, (float)-3.02, (float)
		1.65, (float)-.5, (float)-3.24, (float)1.51, (float)-.38, (
		float)-3.44, (float)1.33, (float)-.27, (float)-3.62, (float)
		1.11, (float)-.18, (float)-3.76, (float).86, (float)-.1, (
		float)-3.86, (float).59, (float)-.05, (float)-3.92, (float).3,
		 (float)-.01, (float)-3.94, (float)0., (float)0., {0}, (float)
		-1.17, (float)1.12, (float)-1.48, (float)-1.34, (float)1.34, (
		float)-1.4, (float)-1.53, (float)1.52, (float)-1.29, (float)
		-1.75, (float)1.66, (float)-1.17, (float)-1.99, (float)1.76, (
		float)-1.04, (float)-2.24, (float)1.81, (float)-.91, (float)
		-2.49, (float)1.8, (float)-.77, (float)-2.75, (float)1.75, (
		float)-.64, (float)-2.99, (float)1.65, (float)-.5, (float)
		-3.21, (float)1.51, (float)-.38, (float)-3.41, (float)1.33, (
		float)-.27, (float)-3.58, (float)1.11, (float)-.18, (float)
		-3.72, (float).86, (float)-.1, (float)-3.83, (float).59, (
		float)-.05, (float)-3.89, (float).3, (float)-.01, (float)
		-3.91, (float)0., (float)0., {0}, (float)-1.16, (float)1.12, (
		float)-1.48, (float)-1.32, (float)1.34, (float)-1.4, (float)
		-1.52, (float)1.52, (float)-1.29, (float)-1.74, (float)1.66, (
		float)-1.17, (float)-1.98, (float)1.76, (float)-1.04, (float)
		-2.23, (float)1.81, (float)-.91, (float)-2.48, (float)1.8, (
		float)-.77, (float)-2.73, (float)1.75, (float)-.64, (float)
		-2.97, (float)1.65, (float)-.5, (float)-3.2, (float)1.51, (
		float)-.38, (float)-3.4, (float)1.33, (float)-.27, (float)
		-3.57, (float)1.11, (float)-.18, (float)-3.71, (float).86, (
		float)-.1, (float)-3.81, (float).59, (float)-.05, (float)
		-3.88, (float).3, (float)-.01, (float)-3.9, (float)0., (float)
		0., {0}, (float)-1.12, (float)1.12, (float)-1.48, (float)
		-1.28, (float)1.34, (float)-1.4, (float)-1.48, (float)1.52, (
		float)-1.29, (float)-1.7, (float)1.66, (float)-1.17, (float)
		-1.94, (float)1.76, (float)-1.04, (float)-2.19, (float)1.81, (
		float)-.91, (float)-2.44, (float)1.8, (float)-.77, (float)
		-2.7, (float)1.75, (float)-.64, (float)-2.94, (float)1.65, (
		float)-.5, (float)-3.16, (float)1.51, (float)-.38, (float)
		-3.36, (float)1.33, (float)-.27, (float)-3.53, (float)1.11, (
		float)-.18, (float)-3.67, (float).86, (float)-.1, (float)
		-3.78, (float).59, (float)-.05, (float)-3.84, (float).3, (
		float)-.01, (float)-3.86, (float)0., (float)0., {0}, (float)
		-1.04, (float)1.11, (float)-1.48, (float)-1.21, (float)1.33, (
		float)-1.4, (float)-1.4, (float)1.52, (float)-1.29, (float)
		-1.63, (float)1.66, (float)-1.17, (float)-1.87, (float)1.76, (
		float)-1.04, (float)-2.12, (float)1.8, (float)-.91, (float)
		-2.37, (float)1.8, (float)-.77, (float)-2.62, (float)1.75, (
		float)-.64, (float)-2.86, (float)1.65, (float)-.51, (float)
		-3.08, (float)1.51, (float)-.38, (float)-3.28, (float)1.32, (
		float)-.27, (float)-3.46, (float)1.1, (float)-.18, (float)
		-3.6, (float).86, (float)-.1, (float)-3.7, (float).58, (float)
		-.05, (float)-3.76, (float).3, (float)-.01, (float)-3.78, (
		float)0., (float)0., {0}, (float)-.97, (float)1.11, (float)
		-1.48, (float)-1.13, (float)1.33, (float)-1.4, (float)-1.33, (
		float)1.51, (float)-1.29, (float)-1.55, (float)1.66, (float)
		-1.17, (float)-1.79, (float)1.75, (float)-1.04, (float)-2.04, 
		(float)1.8, (float)-.91, (float)-2.29, (float)1.8, (float)
		-.77, (float)-2.54, (float)1.75, (float)-.64, (float)-2.78, (
		float)1.65, (float)-.51, (float)-3.01, (float)1.51, (float)
		-.38, (float)-3.21, (float)1.32, (float)-.27, (float)-3.38, (
		float)1.1, (float)-.18, (float)-3.52, (float).86, (float)-.1, 
		(float)-3.62, (float).58, (float)-.05, (float)-3.69, (float)
		.3, (float)-.01, (float)-3.71, (float)0., (float)0., {0}, (
		float)-.83, (float)1.11, (float)-1.49, (float)-1., (float)
		1.33, (float)-1.4, (float)-1.19, (float)1.51, (float)-1.29, (
		float)-1.41, (float)1.65, (float)-1.18, (float)-1.65, (float)
		1.75, (float)-1.05, (float)-1.9, (float)1.79, (float)-.91, (
		float)-2.16, (float)1.79, (float)-.77, (float)-2.41, (float)
		1.74, (float)-.64, (float)-2.65, (float)1.64, (float)-.51, (
		float)-2.87, (float)1.5, (float)-.38, (float)-3.07, (float)
		1.32, (float)-.27, (float)-3.25, (float)1.1, (float)-.18, (
		float)-3.39, (float).85, (float)-.1, (float)-3.49, (float).58,
		 (float)-.05, (float)-3.55, (float).29, (float)-.01, (float)
		-3.57, (float)0., (float)0., {0}, (float)-.71, (float)1.1, (
		float)-1.49, (float)-.88, (float)1.32, (float)-1.4, (float)
		-1.07, (float)1.5, (float)-1.3, (float)-1.3, (float)1.65, (
		float)-1.18, (float)-1.54, (float)1.74, (float)-1.05, (float)
		-1.79, (float)1.79, (float)-.91, (float)-2.04, (float)1.79, (
		float)-.77, (float)-2.29, (float)1.74, (float)-.64, (float)
		-2.53, (float)1.64, (float)-.51, (float)-2.75, (float)1.5, (
		float)-.38, (float)-2.95, (float)1.32, (float)-.27, (float)
		-3.13, (float)1.1, (float)-.18, (float)-3.27, (float).85, (
		float)-.1, (float)-3.37, (float).58, (float)-.05, (float)
		-3.43, (float).29, (float)-.01, (float)-3.45, (float)0., (
		float)0., {0}, (float)-2.12, (float)0., (float)0., (float)
		-2.11, (float)-.11, (float)-.01, (float)-2.08, (float)-.22, (
		float)-.03, (float)-2.02, (float)-.32, (float)-.07, (float)
		-1.95, (float)-.4, (float)-.11, (float)-1.87, (float)-.47, (
		float)-.17, (float)-1.78, (float)-.51, (float)-.23, (float)
		-1.68, (float)-.54, (float)-.3, (float)-1.58, (float)-.54, (
		float)-.38, (float)-1.48, (float)-.53, (float)-.45, (float)
		-1.39, (float)-.49, (float)-.52, (float)-1.3, (float)-.43, (
		float)-.59, (float)-1.23, (float)-.36, (float)-.66, {0}, (
		float)-2.09, (float)0., (float)0., (float)-2.08, (float)-.11, 
		(float)-.01, (float)-2.04, (float)-.22, (float)-.03, (float)
		-1.99, (float)-.32, (float)-.07, (float)-1.92, (float)-.41, (
		float)-.11, (float)-1.83, (float)-.47, (float)-.17, (float)
		-1.74, (float)-.52, (float)-.24, (float)-1.64, (float)-.54, (
		float)-.31, (float)-1.54, (float)-.55, (float)-.38, (float)
		-1.45, (float)-.53, (float)-.45, (float)-1.35, (float)-.49, (
		float)-.52, (float)-1.27, (float)-.44, (float)-.59, (float)
		-1.19, (float)-.36, (float)-.66, {0}, (float)-2.07, (float)0.,
		 (float)0., (float)-2.06, (float)-.12, (float)-.01, (float)
		-2.03, (float)-.23, (float)-.03, (float)-1.97, (float)-.33, (
		float)-.07, (float)-1.9, (float)-.41, (float)-.12, (float)
		-1.82, (float)-.48, (float)-.17, (float)-1.72, (float)-.52, (
		float)-.24, (float)-1.62, (float)-.54, (float)-.31, (float)
		-1.52, (float)-.55, (float)-.38, (float)-1.43, (float)-.53, (
		float)-.46, (float)-1.33, (float)-.49, (float)-.53, (float)
		-1.25, (float)-.43, (float)-.6, (float)-1.18, (float)-.36, (
		float)-.66, {0}, (float)-2.04, (float)0., (float)0., (float)
		-2.02, (float)-.12, (float)-.01, (float)-1.99, (float)-.23, (
		float)-.03, (float)-1.93, (float)-.33, (float)-.07, (float)
		-1.86, (float)-.42, (float)-.12, (float)-1.77, (float)-.48, (
		float)-.18, (float)-1.68, (float)-.53, (float)-.24, (float)
		-1.58, (float)-.55, (float)-.32, (float)-1.48, (float)-.55, (
		float)-.39, (float)-1.38, (float)-.53, (float)-.46, (float)
		-1.29, (float)-.49, (float)-.54, (float)-1.2, (float)-.43, (
		float)-.6, (float)-1.13, (float)-.35, (float)-.67, {0}, (
		float)-1.96, (float)0., (float)0., (float)-1.95, (float)-.12, 
		(float)-.01, (float)-1.92, (float)-.23, (float)-.03, (float)
		-1.86, (float)-.33, (float)-.06, (float)-1.79, (float)-.41, (
		float)-.11, (float)-1.71, (float)-.48, (float)-.17, (float)
		-1.62, (float)-.53, (float)-.23, (float)-1.52, (float)-.55, (
		float)-.3, (float)-1.42, (float)-.56, (float)-.37, (float)
		-1.33, (float)-.54, (float)-.45, (float)-1.23, (float)-.51, (
		float)-.52, (float)-1.15, (float)-.46, (float)-.59, (float)
		-1.07, (float)-.38, (float)-.65, {0}, (float)-1.89, (float)0.,
		 (float)0., (float)-1.87, (float)-.12, (float)-.01, (float)
		-1.84, (float)-.23, (float)-.03, (float)-1.79, (float)-.33, (
		float)-.06, (float)-1.72, (float)-.42, (float)-.11, (float)
		-1.64, (float)-.48, (float)-.17, (float)-1.54, (float)-.53, (
		float)-.23, (float)-1.45, (float)-.56, (float)-.3, (float)
		-1.35, (float)-.57, (float)-.37, (float)-1.25, (float)-.55, (
		float)-.45, (float)-1.16, (float)-.52, (float)-.52, (float)
		-1.07, (float)-.47, (float)-.59, (float)-1., (float)-.39, (
		float)-.65, {0}, (float)-1.75, (float)0., (float)0., (float)
		-1.74, (float)-.12, (float)-.01, (float)-1.71, (float)-.23, (
		float)-.03, (float)-1.65, (float)-.34, (float)-.07, (float)
		-1.58, (float)-.43, (float)-.11, (float)-1.5, (float)-.5, (
		float)-.17, (float)-1.41, (float)-.55, (float)-.23, (float)
		-1.31, (float)-.58, (float)-.3, (float)-1.21, (float)-.58, (
		float)-.38, (float)-1.11, (float)-.57, (float)-.45, (float)
		-1.02, (float)-.54, (float)-.52, (float)-.93, (float)-.48, (
		float)-.59, (float)-.86, (float)-.41, (float)-.66, {0}, (
		float)-1.63, (float)0., (float)0., (float)-1.62, (float)-.12, 
		(float)-.01, (float)-1.59, (float)-.24, (float)-.03, (float)
		-1.53, (float)-.34, (float)-.07, (float)-1.46, (float)-.44, (
		float)-.11, (float)-1.38, (float)-.51, (float)-.17, (float)
		-1.29, (float)-.56, (float)-.24, (float)-1.19, (float)-.59, (
		float)-.31, (float)-1.09, (float)-.6, (float)-.38, (float)
		-.99, (float)-.59, (float)-.45, (float)-.9, (float)-.55, (
		float)-.52, (float)-.81, (float)-.5, (float)-.59, (float)-.74,
		 (float)-.43, (float)-.66, {0}, (float)-1.52, (float).94, (
		float)-.95, (float)-1.65, (float)1.07, (float)-.88, (float)
		-1.8, (float)1.17, (float)-.8, (float)-1.96, (float)1.24, (
		float)-.72, (float)-2., (float)1.24, (float)-.67, (float)
		-2.23, (float)1.28, (float)-.56, (float)-2.48, (float)1.25, (
		float)-.44, (float)-2.65, (float)1.18, (float)-.36, (float)
		-2.81, (float)1.08, (float)-.27, (float)-2.96, (float).95, (
		float)-.19, (float)-3.08, (float).8, (float)-.13, (float)
		-3.19, (float).62, (float)-.07, (float)-3.26, (float).42, (
		float)-.03, (float)-3.31, (float).21, (float)-.01, (float)
		-3.32, (float)0., (float)0., {0}, (float)-1.48, (float).94, (
		float)-.95, (float)-1.61, (float)1.07, (float)-.88, (float)
		-1.76, (float)1.17, (float)-.8, (float)-1.93, (float)1.24, (
		float)-.72, (float)-1.97, (float)1.24, (float)-.67, (float)
		-2.2, (float)1.28, (float)-.57, (float)-2.45, (float)1.25, (
		float)-.44, (float)-2.62, (float)1.18, (float)-.36, (float)
		-2.78, (float)1.08, (float)-.27, (float)-2.92, (float).95, (
		float)-.19, (float)-3.05, (float).8, (float)-.13, (float)
		-3.15, (float).62, (float)-.07, (float)-3.23, (float).42, (
		float)-.03, (float)-3.27, (float).21, (float)-.01, (float)
		-3.29, (float)0., (float)0., {0}, (float)-1.47, (float).94, (
		float)-.95, (float)-1.6, (float)1.06, (float)-.88, (float)
		-1.75, (float)1.17, (float)-.8, (float)-1.91, (float)1.24, (
		float)-.72, (float)-1.96, (float)1.24, (float)-.67, (float)
		-2.18, (float)1.28, (float)-.57, (float)-2.44, (float)1.25, (
		float)-.44, (float)-2.61, (float)1.18, (float)-.36, (float)
		-2.77, (float)1.08, (float)-.27, (float)-2.91, (float).95, (
		float)-.19, (float)-3.04, (float).8, (float)-.13, (float)
		-3.14, (float).62, (float)-.07, (float)-3.22, (float).42, (
		float)-.03, (float)-3.26, (float).21, (float)-.01, (float)
		-3.28, (float)0., (float)0., {0}, (float)-1.43, (float).93, (
		float)-.95, (float)-1.56, (float)1.06, (float)-.88, (float)
		-1.71, (float)1.16, (float)-.8, (float)-1.88, (float)1.24, (
		float)-.72, (float)-1.92, (float)1.24, (float)-.67, (float)
		-2.14, (float)1.28, (float)-.57, (float)-2.4, (float)1.24, (
		float)-.44, (float)-2.57, (float)1.18, (float)-.36, (float)
		-2.73, (float)1.08, (float)-.27, (float)-2.87, (float).95, (
		float)-.19, (float)-3., (float).8, (float)-.13, (float)-3.1, (
		float).62, (float)-.07, (float)-3.18, (float).42, (float)-.03,
		 (float)-3.22, (float).21, (float)-.01, (float)-3.24, (float)
		0., (float)0., {0}, (float)-1.35, (float).93, (float)-.95, (
		float)-1.49, (float)1.06, (float)-.88, (float)-1.64, (float)
		1.16, (float)-.81, (float)-1.8, (float)1.23, (float)-.72, (
		float)-1.84, (float)1.24, (float)-.67, (float)-2.07, (float)
		1.27, (float)-.57, (float)-2.32, (float)1.24, (float)-.45, (
		float)-2.49, (float)1.18, (float)-.36, (float)-2.65, (float)
		1.08, (float)-.27, (float)-2.8, (float).95, (float)-.19, (
		float)-2.92, (float).79, (float)-.13, (float)-3.02, (float)
		.62, (float)-.07, (float)-3.1, (float).42, (float)-.03, (
		float)-3.15, (float).21, (float)-.01, (float)-3.16, (float)0.,
		 (float)0., {0}, (float)-1.28, (float).93, (float)-.95, (
		float)-1.41, (float)1.06, (float)-.88, (float)-1.56, (float)
		1.16, (float)-.81, (float)-1.72, (float)1.23, (float)-.72, (
		float)-1.77, (float)1.23, (float)-.67, (float)-1.99, (float)
		1.27, (float)-.57, (float)-2.25, (float)1.24, (float)-.45, (
		float)-2.41, (float)1.17, (float)-.36, (float)-2.57, (float)
		1.08, (float)-.27, (float)-2.72, (float).95, (float)-.19, (
		float)-2.85, (float).79, (float)-.13, (float)-2.95, (float)
		.62, (float)-.07, (float)-3.03, (float).42, (float)-.03, (
		float)-3.07, (float).21, (float)-.01, (float)-3.09, (float)0.,
		 (float)0., {0}, (float)-1.14, (float).92, (float)-.95, (
		float)-1.27, (float)1.05, (float)-.88, (float)-1.42, (float)
		1.15, (float)-.81, (float)-1.59, (float)1.22, (float)-.72, (
		float)-1.64, (float)1.23, (float)-.67, (float)-1.86, (float)
		1.26, (float)-.57, (float)-2.11, (float)1.23, (float)-.45, (
		float)-2.28, (float)1.17, (float)-.36, (float)-2.44, (float)
		1.07, (float)-.27, (float)-2.59, (float).94, (float)-.19, (
		float)-2.71, (float).79, (float)-.13, (float)-2.81, (float)
		.61, (float)-.07, (float)-2.89, (float).42, (float)-.03, (
		float)-2.94, (float).21, (float)-.01, (float)-2.95, (float)0.,
		 (float)0., {0}, (float)-1.02, (float).92, (float)-.95, (
		float)-1.16, (float)1.05, (float)-.88, (float)-1.3, (float)
		1.15, (float)-.81, (float)-1.47, (float)1.22, (float)-.72, (
		float)-1.53, (float)1.22, (float)-.67, (float)-1.75, (float)
		1.25, (float)-.57, (float)-1.99, (float)1.23, (float)-.45, (
		float)-2.16, (float)1.17, (float)-.36, (float)-2.32, (float)
		1.07, (float)-.27, (float)-2.47, (float).94, (float)-.19, (
		float)-2.59, (float).79, (float)-.13, (float)-2.7, (float).61,
		 (float)-.07, (float)-2.77, (float).42, (float)-.03, (float)
		-2.82, (float).21, (float)-.01, (float)-2.83, (float)0., (
		float)0. };

#define tau ((real *)&equiv_23)


    /* System generated locals */
    integer i__1, i__2;

    /* Builtin functions */
    integer s_cmp();
    double cos(), sin();

    /* Local variables */
    static integer idel, idep;
    static real azir;
    static integer idel1, idep1, i__;
    static real t[3], delta, t0, t1, t2, dd, dz;
    static integer ips;
    static real ecolatr, anumber;



/* 	This subroutine computes an ellipticity correction based */
/* 	on the source to receiver distance and azimuth, and the source */
/* 	depth and colatitude.  The correction scheme was developed by */
/* 	Dziewonski and Gilbert (Geophys. J. R. astr. Soc, 44, 7-17, 1976), */
/* 	and the tabulated values were developed by Brian Kennett (IASPEI */
/* 	1991 Seismological Tables, B.L.N. Kennett, Editor, Research */
/* 	School of Earth Sciences, Australian National University, p149-163). */
/* 	This travel time correction is to be added to the value estimated */
/* 	for a spherical earth. */

/* 	The ellipticity correction is given by: */

/* 	ecorr = 0.25(1+3cos(2*ecolat))*t1 */

/* 		 + sqrt(3)/2 * sin(2*ecolat)*cos(azi)*t1 */

/* 		 + sqrt(3)/2 * (sin(ecolat))**2 * cos(2*azi)*t2 */

/* 	where t0, t1, and t2 are the distance and depth dependent tabulated */
/* 	values. */

/* 	INPUT: */

/* 		phid	- Phase id (character*8). */

/* 		del	- Epicenter to receiver, distance in degrees. */

/* 		z	- Source depth, in km. */

/* 		azi	- Epicenter to receiver azimuth, in degrees. */

/* 		ecolat	- Epicenter colatitude, in degrees. */

/* 	OUTPUT: */

/* 		ecorr	- Travel time correction due to ellipticity, */
/* 			  in seconds. */

/* 	Written 28 May 1991 by David Taylor, ENSCO, Inc. */
/* ------------------------------------------------------------------------------ */


/* ----  tau(t0 t1 and t2,distance,depth,phase) */
/* ----  the 11 phases are:	ips = 1		P */
/* 				ips = 2		PcP */
/* 				ips = 3		PKPab */
/* 				ips = 4		PKPbc */
/* 				ips = 5		PKPdf */
/* 				ips = 6		S */
/* 				ips = 7		ScS */
/* 				ips = 8		SKSac */
/* 				ips = 9		SKSdf */
/* 				ips = 10	ScP */
/* 				ips = 11	SKP */

/* ----  convert degrees to radians */


/* ----  source depths for all tables */


/* ----  distance increment for all the tables */


/* ----  minimum and maximum distances for each table */



/* ----  Here are all the ellipticity tables in Kennett's book: */

/* ----  P 0 km */
/* ----  P 35 km */
/* ----  P 50 km */
/* ---- P 100 km */
/* ----  P 200 km */
/* ----  P 300 km */
/* ----  P 500 km */
/* ----  P 700 km */
/* ---- PcP 0 km */
/* ---- PcP 35 km */
/* ---- PcP 50 km */
/* ---- PcP 100 km */
/* ---- PcP 200 km */
/* ---- PcP 300 km */
/* ---- PcP 500 km */
/* ---- PcP 700 km */
/* ---- PKPab 0 km */
/* ---- PKPab 35 km */
/* ---- PKPab 50 km */
/* ---- PKPab 100 km */
/* ---- PKPab 200 km */
/* ---- PKPab 300 km */
/* ---- PKPab 500 km */
/* ---- PKPab 700 km */
/* ---- PKPbc 0 km */
/* ---- PKPbc 35 km */
/* ---- PKPbc 50 km */
/* ---- PKPbc 100 km */
/* ---- PKPbc 200 km */
/* ---- PKPbc 300 km */
/* ---- PKPbc 500 km */
/* ---- PKPbc 700 km */
/* ---- PKPdf 0 km */
/* ---- PKPdf 35 km */
/* ---- PKPdf 50 km */
/* ---- PKPdf 100 km */
/* ---- PKPdf 200 km */
/* ---- PKPdf 300 km */
/* ---- PKPdf 500 km */
/* ---- PKPdf 700 km */
/* ---- S 0 km */
/* ---- S 35 km */
/* ---- S 50 km */
/* ---- S 100 km */
/* ---- S 200 km */
/* ---- S 300 km */
/* ---- S 500 km */
/* ---- S 700 km */
/* ---- ScS 0 km */
/* ---- ScS 35 km */
/* ---- ScS 50 km */
/* ---- ScS 100 km */
/* ---- ScS 200 km */
/* ---- ScS 300 km */
/* ---- ScS 500 km */
/* ---- ScS 700 km */
/* ---- SKSac 0 km */
/* ---- SKSac 35 km */
/* ---- SKSac 50 km */
/* ---- SKSac 100 km */
/* ---- SKSac 200 km */
/* ---- SKSac 300 km */
/* ---- SKSac 500 km */
/* ---- SKSac 700 km */
/* ---- SKSdf 0 km */
/* ---- SKSdf 35 km */
/* ---- SKSdf 50 km */
/* ---- SKSdf 100 km */
/* ---- SKSdf 200 km */
/* ---- SKSdf 300 km */
/* ---- SKSdf 500 km */
/* ---- SKSdf 700 km */
/* ---- ScP 0 km */
/* ---- ScP 35 km */
/* ---- ScP 50 km */
/* ---- ScP 100 km */
/* ---- ScP 200 km */
/* ---- ScP 300 km */
/* ---- ScP 500 km */
/* ---- ScP 700 km */
/* ---- SKP 0 km */
/* ---- SKP 35 km */
/* ---- SKP 50 km */
/* ---- SKP 100 km */
/* ---- SKP 200 km */
/* ---- SKP 300 km */
/* ---- SKP 500 km */
/* ---- SKP 700 km */


/*       End of Declarations */
/* ------------------------------------------------------------------------------ */

/* ----  set-up */

    *ecorr = (float)0.;
    t0 = (float)0.;
    t1 = (float)0.;
    t2 = (float)0.;
    azir = *azi * degrad;
    ecolatr = *ecolat * degrad;

/* ----	Get the index for the phase name.  If no table for the phase, return. */

    if (s_cmp(phid, "P ", (ftnlen)2, (ftnlen)2) == 0 || s_cmp(phid, "Pn ", (
	    ftnlen)3, (ftnlen)3) == 0 || s_cmp(phid, "Pg ", (ftnlen)3, (
	    ftnlen)3) == 0 || s_cmp(phid, "Pb ", (ftnlen)3, (ftnlen)3) == 0 ||
	     s_cmp(phid, "Pdi", (ftnlen)3, (ftnlen)3) == 0) {
	ips = 1;
    } else if (s_cmp(phid, "PcP ", (ftnlen)4, (ftnlen)4) == 0) {
	ips = 2;
    } else if (s_cmp(phid, "PKPab ", (ftnlen)6, (ftnlen)6) == 0) {
	ips = 3;
    } else if (s_cmp(phid, "PKPbc ", (ftnlen)6, (ftnlen)6) == 0) {
	ips = 4;
    } else if (s_cmp(phid, "PKPdf ", (ftnlen)6, (ftnlen)6) == 0) {
	ips = 5;
    } else if (s_cmp(phid, "PKiKP  ", (ftnlen)6, (ftnlen)7) == 0) {
	ips = 5;
    } else if (s_cmp(phid, "S ", (ftnlen)2, (ftnlen)2) == 0 || s_cmp(phid, 
	    "Sn ", (ftnlen)3, (ftnlen)3) == 0 || s_cmp(phid, "Sg ", (ftnlen)3,
	     (ftnlen)3) == 0 || s_cmp(phid, "Sb ", (ftnlen)3, (ftnlen)3) == 0 
	    || s_cmp(phid, "Sdi", (ftnlen)3, (ftnlen)3) == 0 || s_cmp(phid, 
	    "Lg ", (ftnlen)3, (ftnlen)3) == 0) {
	ips = 6;
    } else if (s_cmp(phid, "ScS ", (ftnlen)4, (ftnlen)4) == 0) {
	ips = 7;
    } else if (s_cmp(phid, "SKSac ", (ftnlen)6, (ftnlen)6) == 0) {
	ips = 8;
    } else if (s_cmp(phid, "SKSdf ", (ftnlen)6, (ftnlen)6) == 0) {
	ips = 9;
    } else if (s_cmp(phid, "ScP ", (ftnlen)4, (ftnlen)4) == 0) {
	ips = 10;
    } else if (s_cmp(phid, "SKP ", (ftnlen)4, (ftnlen)4) == 0) {
	ips = 11;
    } else {
	return 0;
    }

/* ----  check depth and distance to be within limits */

    if (*del < delmin[ips - 1] || *del > delmax[ips - 1]) {
/*	fprintf(stderr,"elpcor error 1  %.3f\n", *del); */
	return 0;
    }
    if (*z__ < (float)0. || *z__ > (float)700.) {
/*	fprintf(stderr,"elpcor error 2  %.3f\n", *z__); */
	return 0;
    }


/* ----  find the depth index for the value just less than the input */

    for (i__ = 2; i__ <= 8; ++i__) {
	if (*z__ <= depth[i__ - 1]) {
	    idep = i__ - 1;
	    goto L2;
	}
    }

/* ----  idel is the depth index just less than the input depth */
/* ----  dd is the fractional change in distance between the input distance */
/*       and the next closest depth index */
/* ----  dz is the fractional change in depth between the depth at idel */
/*       and the input depth */

L2:
    delta = *del - delmin[ips - 1];
    idel = (integer) ((delta + (float)1e-4) / delinc) + 1;
/* Computing MIN */
    i__1 = idel, i__2 = (integer) ((delmax[ips - 1] - delmin[ips - 1]) / 
	    delinc);
    idel = min(i__1,i__2);
    dd = delta / delinc + (float)1. - (real) idel;
    dz = (*z__ - depth[idep - 1]) / (depth[idep] - depth[idep - 1]);

/* ----  compute the three t terms using bilinear interpolation */

    idel1 = idel + 1;
    idep1 = idep + 1;
    for (i__ = 1; i__ <= 3; ++i__) {
	t[i__ - 1] = ((float)1. - dz) * dd * tau[i__ + (idel1 + (idep + (ips 
		<< 3)) * 21) * 3 - 571] + dz * dd * tau[i__ + (idel1 + (idep1 
		+ (ips << 3)) * 21) * 3 - 571] + dz * ((float)1. - dd) * tau[
		i__ + (idel + (idep1 + (ips << 3)) * 21) * 3 - 571] + ((float)
		1. - dz) * ((float)1. - dd) * tau[i__ + (idel + (idep + (ips 
		<< 3)) * 21) * 3 - 571];
    }

/* ----  compute the correction */

    anumber = (float).86602540375;
    *ecorr = (cos(ecolatr * (float)2.) * (float)3. + (float)1.) * (float).25 *
	     t[0] + anumber * sin(ecolatr * (float)2.) * cos(azir) * t[1] + 
	    anumber * sin(ecolatr) * sin(ecolatr) * cos(azir * (float)2.) * t[
	    2];

/* ----	all done */

    return 0;
} /* elpcor_ */

#undef tau



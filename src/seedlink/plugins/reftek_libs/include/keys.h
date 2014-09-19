#pragma ident "$Id: keys.h 165 2005-12-23 12:34:58Z andres $"
/* ASCII keys are returned with the ASCII code in the LSB and 00 in the MSB */
/* Extended key are returned with the scan code in the MSB and 00 in the LSB */

#ifndef _KEY_CODES_DEFINED
#define _KEY_CODES_DEFINED

/* Common extended keystrokes */
#define KEY_NONE         0x0000        /* No keypress */
#define KEY_ESCAPE       0x001B        /* Escape */
#define KEY_RETURN       0x000D        /* Return or Enter */
#define KEY_CTRL_RET     0x000A        /* Ctrl+Return or Ctrl+Enter */
#define KEY_BACKSPACE    0x0008        /* Backspace */
#define KEY_SPACEBAR     0x0020        /* Spacebar */
#define KEY_INSERT       0x5200        /* Insert */
#define KEY_CTRL_INS     0x9200        /* Ctrl+Insert */
#define KEY_ALT_INS      0xA200        /* Alt+Insert */
#define KEY_DELETE       0x5300        /* Delete */
#define KEY_CTRL_DEL     0x9300        /* Ctrl+Delete */
#define KEY_ALT_DEL      0xA300        /* Alt+Delete */
#define KEY_F1           0x3B00        /* F1 */
#define KEY_F2           0x3C00        /* F2 */
#define KEY_F3           0x3D00        /* F3 */
#define KEY_F4           0x3E00        /* F4 */
#define KEY_F5           0x3F00        /* F5 */
#define KEY_F6           0x4000        /* F6 */
#define KEY_F7           0x4100        /* F7 */
#define KEY_F8           0x4200        /* F8 */
#define KEY_F9           0x4300        /* F9 */
#define KEY_F10          0x4400        /* F10 */
#define KEY_F11          0x8500        /* F11 */
#define KEY_F12          0x8600        /* F12 */
#define KEY_CTRL_F1      0x5E00        /* Ctrl+F1 */
#define KEY_CTRL_F2      0x5F00        /* Ctrl+F2 */
#define KEY_CTRL_F3      0x6000        /* Ctrl+F3 */
#define KEY_CTRL_F4      0x6100        /* Ctrl+F4 */
#define KEY_CTRL_F5      0x6200        /* Ctrl+F5 */
#define KEY_CTRL_F6      0x6300        /* Ctrl+F6 */
#define KEY_CTRL_F7      0x6400        /* Ctrl+F7 */
#define KEY_CTRL_F8      0x6500        /* Ctrl+F8 */
#define KEY_CTRL_F9      0x6600        /* Ctrl+F9 */
#define KEY_CTRL_F10     0x6700        /* Ctrl+F10 */
#define KEY_CTRL_F11     0x8900        /* Ctrl+F11 */
#define KEY_CTRL_F12     0x8A00        /* Ctrl+F12 */
#define KEY_ALT_F1       0x6800        /* Alt+F1 */
#define KEY_ALT_F2       0x6900        /* Alt+F2 */
#define KEY_ALT_F3       0x6A00        /* Alt+F3 */
#define KEY_ALT_F4       0x6B00        /* Alt+F4 */
#define KEY_ALT_F5       0x6C00        /* Alt+F5 */
#define KEY_ALT_F6       0x6D00        /* Alt+F6 */
#define KEY_ALT_F7       0x6E00        /* Alt+F7 */
#define KEY_ALT_F8       0x6F00        /* Alt+F8 */
#define KEY_ALT_F9       0x7000        /* Alt+F9 */
#define KEY_ALT_F10      0x7100        /* Alt+F10 */
#define KEY_ALT_F11      0x8B00        /* Alt+F11 */
#define KEY_ALT_F12      0x8C00        /* Alt+F12 */

/* Navigation keys */
#define KEY_UP           0x4800        /* Up arrow */
#define KEY_DOWN         0x5000        /* Down arrow */
#define KEY_LEFT         0x4B00        /* Left arrow */
#define KEY_RIGHT        0x4D00        /* Right arrow */
#define KEY_CENTER       0x4C00        /* Center (5 key on keypad) */
#define KEY_PGDN         0x5100        /* Page down */
#define KEY_PGUP         0x4900        /* Page up */
#define KEY_HOME         0x4700        /* Home */
#define KEY_END          0x4F00        /* End */

#define KEY_CTRL_PGDN    0x7600        /* Ctrl+Page down */
#define KEY_CTRL_PGUP    0x8400        /* Ctrl+Page up */
#define KEY_CTRL_HOME    0x7700        /* Ctrl+Home */
#define KEY_CTRL_END     0x7500        /* Ctrl+End */

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */

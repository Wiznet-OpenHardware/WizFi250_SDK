#ifndef WX_COMMANDS_MISC_H
#define WX_COMMANDS_MISC_H

UINT8 WXCmd_USET(UINT8 *ptr);
#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function + 130702 ID1092 - ATCmd update (naming, adding comments)
UINT8 WXCmd_MECHO(UINT8 *ptr);
UINT8 WXCmd_MHELP(UINT8 *ptr);
#endif
UINT8 WXCmd_MCERT(UINT8 *ptr);
UINT8 WXCmd_MSPI(UINT8 *ptr);

#endif

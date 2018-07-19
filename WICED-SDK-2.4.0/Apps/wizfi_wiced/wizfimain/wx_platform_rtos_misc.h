#ifndef WX_PLATFORM_RTOS_MISC_H
#define WX_PLATFORM_RTOS_MISC_H

UINT8 WXHal_CharGet(UINT8 *ch);
VOID WXHal_CharNGet(UINT8 *ch,UINT16 dataLen);
VOID WXHal_CharPut(UINT8 ch);
VOID WXHal_CharNPut(const VOID *buf, UINT32 len);

#endif

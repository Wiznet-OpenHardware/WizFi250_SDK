#ifndef WX_GENERAL_PARSE_H
#define WX_GENERAL_PARSE_H

UINT8* WXParse_NextParamGet(UINT8 **temp);
UINT8 WXParse_Int(UINT8 *ptr, UINT32 *val);
UINT8 WXParse_Hex(UINT8 *ptr, UINT32 *val);
UINT8 WXParse_Boolean(UINT8 *ptr, UINT8 *val);
UINT8 WXParse_Ip(UINT8 *p, UINT8 *ip);
UINT8 WXParse_Mac(UINT8 *s, UINT8 *mac_addr);
UINT8 WXParse_Ssid(UINT8 *p, UINT8 *ssid, UINT8 *lenp);
INT32 WXParse_StrnCaseCmp(const INT8 *s1, const INT8 *s2, UINT32 n);
UINT8 WXParse_ToHex(UINT8 c);
UINT8 WXParse_Number(UINT8 *ptr, UINT32 *val, UINT8 hex);

int hex_str_to_int(const char* hex_str);
int str_to_int(const char* str);
uint32_t str_to_ip(char* arg);
wiced_security_t str_to_authtype(char* arg);
char* authtype_to_str(char* buff, wiced_security_t security);
wiced_mac_t str_to_mac(char* arg);
char* mac_to_str(char* buff, wiced_mac_t* mac);
char* uartinfo_to_str(char* des, void* pUart);
char *replaceAll(char* src, char* des, const char *olds, const char *news);
char* process_crlf(char* src, char* des);
char* upstr(char *s);

#endif

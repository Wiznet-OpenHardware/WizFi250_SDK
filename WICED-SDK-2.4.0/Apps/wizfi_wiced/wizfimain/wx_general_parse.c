
#include "wx_defines.h"

UINT8* WXParse_NextParamGet(UINT8 **temp)
{
	INT8 *str = (INT8 *)*temp;
	INT8 prev = '\0';
	INT32 quoted = 0;
	INT32 done = 0;
	INT8 *p;

	if (!*str)
	{
		return NULL;
	}

	p = str;
	while (isspace((int)(*p)))
	{
		p++;
	}

	if (*p == '"')
	{
		quoted = 1;
		str = p + 1;
	}

	for (p = str; *p; p++)
	{
		if (*p == ',' && (!quoted || done))
		{
			break;
		}
		else if (*p == '"' && quoted && prev != '\\')
		{
			if (done)
			{
				/* Misquoted parameter */
				return NULL;
			}

			prev = *p = '\0';
			done = 1;
		}

		prev = *p;
	}

#if 1 //MikeJ 130408 ID1011 - If it is NULL parameter, return NULL.
	if (str == p) {
		*temp = (UINT8 *) (p + 1);
		return NULL;
	}
#endif
	if (*p)
	{
		*p = '\0';
		*temp = (UINT8 *) (p + 1);
	}
	else
	{
		*temp = (UINT8 *) p;
	}

	return (UINT8 *) str;
}

UINT8 WXParse_Int(UINT8 *ptr, UINT32 *val)
{
	return WXParse_Number(ptr, val, 0);
}

UINT8 WXParse_Hex(UINT8 *ptr, UINT32 *val)
{
	return WXParse_Number(ptr, val, 1);
}

UINT8 WXParse_Boolean(UINT8 *ptr, UINT8 *val)
{
	while (isspace(*ptr))
	{
		ptr++;
	}

	if ((*ptr != '0' && *ptr != '1') || *(ptr + 1) != '\0')
	{
		return WXCODE_EINVAL;
	}

	*val = (*ptr == '1');
	return WXCODE_SUCCESS;
}

UINT8 WXParse_Ip(UINT8 *p, UINT8 *ip)
{
	UINT32 val;
	INT32 digits;
	INT32 quad;

	memset(ip, 0, 4);

	while (isspace(*p))
	{
		p++;
	}

	for (quad = 0; quad < 4; quad++)
	{
		val = 0;
		digits = 0;

		while (*p)
		{
			if (!isdigit(*p))
			{
				break;
			}

			val = val * 10 + (*p - '0');

			p++;
			digits++;
		}

		if (!digits || val > 255 || (quad < 3 && *p != '.'))
		{
			return WXCODE_EINVAL;
		}

		ip[quad] = val;

		if (quad < 3)
		{
			p++;
		}
	}

	while (isspace(*p))
	{
		p++;
	}

	if (*p)
	{
		return WXCODE_EINVAL;
	}

	return WXCODE_SUCCESS;
}

UINT8 WXParse_Mac(UINT8 *s, UINT8 *mac_addr)
{
	UINT32 i, val;
	UINT8 dummy[6]={0x00,0x00,0x00,0x00,0x00,0x00}; // 0 mac
	UINT8 dummy1[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; // broadcast mac
	memset(mac_addr, 0, 6);

	for (i = 0; *s != '\0' && i < 7; i++, s++)
	{
		INT32 digits = 0;

		val = 0;
		while (*s != ':' && *s != '\0' && *s != ' ')
		{
			if (!isxdigit(*s))
			{
				return WXCODE_EINVAL;
			}

			val = val * 16 + WXParse_ToHex(*s);
			s++;
			digits++;
		}

		if (!digits || digits > 2)
		{
			return WXCODE_EINVAL;
		}

		if(i<6)
		{
			mac_addr[i] = val;
		}
		if(*s == '\0')
		{
			i++;
			break;
		}
	}
	if((memcmp(mac_addr,dummy,6)== 0) || (memcmp(mac_addr,dummy1,6) == 0))
	{
		return WXCODE_EINVAL;
	}
	return i == 6 ? WXCODE_SUCCESS : WXCODE_EINVAL;
}

UINT8 WXParse_Ssid(UINT8 *p, UINT8 *ssid, UINT8 *lenp)
{
	UINT8 len = 0;

	memset(ssid, 0, WX_MAX_SSID_LEN);

	while (*p)
	{
		if (*p == '\\' && *(p + 1) == '"')
		{
			*ssid = '"';
			p++;
		}
		else
		{
			*ssid = *p;
		}

		p++;
		len++;
		ssid++;

		if (len >= WX_MAX_SSID_LEN)
		{
			break;
		}
	}
	*lenp = len;
	if (*p)
	{
		return WXCODE_EINVAL;
	}

	if (len == 0)
	{
		return WXCODE_EINVAL;
	}

	return WXCODE_SUCCESS;
}


INT32 WXParse_StrnCaseCmp(const INT8 *s1, const INT8 *s2, UINT32 n)
{
	INT32 diff;

	while (n--)
	{
		if (!*s1 && !*s2)
		{
			break;
		}

		//diff = toupper(*s1) - toupper(*s2);
		diff = toupper((UINT8)*s1) - toupper((UINT8)*s2);
		if (diff)
		{
			return diff;
		}
		s1++;
		s2++;
	}

	return 0;
}

UINT8 WXParse_ToHex(UINT8 c)
{
	UINT8 val = 0;

	if (c >= '0' && c <= '9')
	{
		val = c - '0';
	}
	else if (c >= 'A' && c <= 'F')
	{
		val = c - 'A' + 10;
	}
	else if (c >= 'a' && c <= 'f')
	{
		val = c - 'a' + 10;
	}

	return val;
}

UINT8 WXParse_Number(UINT8 *ptr, UINT32 *val, UINT8 hex)
{
	UINT32 temp = 0;

	*val = 0;

	while (isspace(*ptr))
	{
		ptr++;
	}

	if (hex)
	{
		while (*ptr)
		{
			if (!isxdigit(*ptr))
			{
				break;
			}
			temp = temp * 16 + WXParse_ToHex(*ptr);
			ptr++;
		}
	}
	else
	{
		for (; *ptr >= '0' && *ptr <= '9'; ptr++)
		{
			temp = temp * 10 + (*ptr - '0');
		}
	}

	if (*ptr)
	{
		while (isspace(*ptr))
		{
			ptr++;
		}

		if (*ptr)
		{
			return WXCODE_EINVAL;
		}
	}

	*val = temp;
	return WXCODE_SUCCESS;
}

int hex_str_to_int(const char* hex_str)
{
	int n = 0;
	uint32_t value = 0;
	int shift = 7;
	while (hex_str[n] != '\0' && n < 8)
	{
		if ( hex_str[n] > 0x21 && hex_str[n] < 0x40 )
		{
			value |= (hex_str[n] & 0x0f) << (shift << 2);
		}
		else if ( (hex_str[n] >= 'a' && hex_str[n] <= 'f') || (hex_str[n] >= 'A' && hex_str[n] <= 'F') )
		{
			value |= ((hex_str[n] & 0x0f) + 9) << (shift << 2);
		}
		else
		{
			break;
		}
		n++;
		shift--;
	}

	return (value >> ((shift + 1) << 2));
}


int str_to_int(const char* str)
{
	uint32_t val = 0;
	if ( strncmp(str, "0x", 2) == 0 )
	{
		val = hex_str_to_int(str + 2);
	}
	else
	{
		val = atoi(str);
	}
	return val;
}


uint32_t str_to_ip(char* arg)
{
	int temp[4];

	sscanf(arg, "%d.%d.%d.%d", &temp[0], &temp[1], &temp[2], &temp[3]);
	return temp[0] << 24 | temp[1] << 16 | temp[2] << 8 | temp[3];
}


wiced_security_t str_to_authtype(char* arg)
{
	if ( strcmp(arg, "OPEN") == 0 )			return WICED_SECURITY_OPEN;
	else if ( strcmp(arg, "WEP") == 0 )		return WICED_SECURITY_WEP_PSK;
	else if ( strcmp(arg, "WPA2AES") == 0 )	return WICED_SECURITY_WPA2_AES_PSK;
	else if ( strcmp(arg, "WPA2TKIP") == 0) return WICED_SECURITY_WPA2_TKIP_PSK; 		// kaizen 20130729 ID1106 Added WPA2_TKIP because WizFi250 is not connect to AP which set WPA2 TKIP option
	else if ( strcmp(arg, "WPA2") == 0 )	return WICED_SECURITY_WPA2_MIXED_PSK;
	else if ( strcmp(arg, "WPA") == 0 )		return WICED_SECURITY_WPA_TKIP_PSK;
	else if ( strcmp(arg, "WPAAES") == 0 )	return WICED_SECURITY_WPA_AES_PSK;			// kaizen 20130408 Added WPA_AES
	else									return WICED_SECURITY_UNKNOWN;
}

char* authtype_to_str(char* buff, wiced_security_t security)
{
	if ( security == WICED_SECURITY_OPEN )					strcpy(buff,"OPEN");
	else if ( security == WICED_SECURITY_WEP_PSK )			strcpy(buff,"WEP");
	else if ( security == WICED_SECURITY_WPA2_AES_PSK )		strcpy(buff,"WPA2AES");
	else if ( security == WICED_SECURITY_WPA2_TKIP_PSK )	strcpy(buff,"WPA2TKIP");	// kaizen 20130729 ID1106 Added WPA2_TKIP because WizFi250 is not connect to AP which set WPA2 TKIP option
	else if ( security == WICED_SECURITY_WPA2_MIXED_PSK )	strcpy(buff,"WPA2");
	else if ( security == WICED_SECURITY_WPA_TKIP_PSK )		strcpy(buff,"WPA");
	else if ( security == WICED_SECURITY_WPA_AES_PSK )		strcpy(buff,"WPAAES");		// kaizen 20130408 Added WPA_AES
	else													strcpy(buff,"");

	return buff;
}

wiced_mac_t str_to_mac(char* arg)
{
	wiced_mac_t mac =
	{
	{ 0, 0, 0, 0, 0, 0 } };
	char* start = arg;
	char* end;
	int a = 0;
	do
	{
		end = strchr(start, ':');
		if ( end != NULL )
		{
			*end = '\0';
		}
		mac.octet[a] = (uint8_t) hex_str_to_int(start);
		if ( end != NULL )
		{
			*end = ':';
			start = end + 1;
		}
		++a;
	} while (a < 6 && end != NULL);
	return mac;
}

char* mac_to_str(char* buff, wiced_mac_t* mac)
{
	sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", mac->octet[0], mac->octet[1], mac->octet[2], mac->octet[3], mac->octet[4], mac->octet[5]);
	return buff;
}

char* uartinfo_to_str(char* des, void* pUart)
{
	USART_InitTypeDef* pData = (USART_InitTypeDef*) pUart;

	char buffData[10];
	sprintf(des, "%d", (int)pData->USART_BaudRate);

	buffData[0] = 0;
	if ( pData->USART_Parity==USART_Parity_No )			strcpy(buffData, "N");
	else if ( pData->USART_Parity==USART_Parity_Even )	strcpy(buffData, "E");
	else if ( pData->USART_Parity==USART_Parity_Odd )	strcpy(buffData, "O");
	strcat(des, ",");
	strcat(des, buffData);

	buffData[0] = 0;
	if ( pData->USART_WordLength==USART_WordLength_9b )		strcpy(buffData, "8");
	else if ( pData->USART_WordLength==USART_WordLength_8b )
	{
		if ( pData->USART_Parity==USART_Parity_No )			strcpy(buffData, "8");
#if 1 //MikeJ 130410 ID1024 - USART Param condition change
		else												strcpy(buffData, "7");
#else
		else												strcpy(buffData, "9");
#endif
	}
	strcat(des, ",");
	strcat(des, buffData);

	buffData[0] = 0;
	if ( pData->USART_StopBits==USART_StopBits_0_5 )		strcpy(buffData, "0.5");
	else if ( pData->USART_StopBits==USART_StopBits_1 )		strcpy(buffData, "1");
	else if ( pData->USART_StopBits==USART_StopBits_1_5 )	strcpy(buffData, "1.5");
	else if ( pData->USART_StopBits==USART_StopBits_2 )		strcpy(buffData, "2");
	strcat(des, ",");
	strcat(des, buffData);

	buffData[0] = 0;
#if 1 //MikeJ 130410 ID1024 - USART Param condition change
	if ( pData->USART_HardwareFlowControl==USART_HardwareFlowControl_None )			strcpy(buffData, "N");
	else if ( pData->USART_HardwareFlowControl==USART_HardwareFlowControl_RTS_CTS )	strcpy(buffData, "HW");
#else
	if ( pData->USART_HardwareFlowControl==USART_HardwareFlowControl_None )			strcpy(buffData, "N");
	else if ( pData->USART_HardwareFlowControl==USART_HardwareFlowControl_RTS )		strcpy(buffData, "R");
	else if ( pData->USART_HardwareFlowControl==USART_HardwareFlowControl_CTS )		strcpy(buffData, "C");
	else if ( pData->USART_HardwareFlowControl==USART_HardwareFlowControl_RTS_CTS )	strcpy(buffData, "RC");
#endif
	strcat(des, ",");
	strcat(des, buffData);

	return des;
}

char *replaceAll(char* src, char* des, const char *olds, const char *news)
{
	char *ptr;
	int i, count = 0;
	int oldlen = strlen(olds);
	if (oldlen < 1)
		return src;
	int newlen = strlen(news);

	if (newlen != oldlen)
	{
		for (i = 0; src[i] != '\0';)
		{
			if (memcmp(&src[i], olds, oldlen) == 0) count++, i += oldlen;
			else i++;
		}
	}
	else
	{
		i = strlen(src);
	}

	ptr = des;
	while (*src)
	{
		if (memcmp(src, olds, oldlen) == 0)
		{
			memcpy(ptr, news, newlen);
			ptr += newlen;
			src  += oldlen;
		}
		else
		{
			*ptr++ = *src++;
		}
	}
	*ptr = '\0';

	return des;
}

char* process_crlf(char* src, char* des)
{
	char buff1[256];

	replaceAll(src, buff1, "\r", "{0x0D}");
	replaceAll(buff1, des, "\n", "{0x0A}");
	return des;
}

char* upstr(char *s)
{
	char* p = 0;

	for(p = s; *p != '\0'; p++)
	{
		*p = (char) toupper((int) (*p));
	}

	return s;
}

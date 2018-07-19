
#include "wx_defines.h"
#include "wiced_platform.h"


UINT8 WXHal_CharGet(UINT8 *ch)
{
	*ch = getchar();
	/*
	if ( wiced_uart_receive_bytes(WICED_UART_1, ch, 1, WICED_NEVER_TIMEOUT)!=WICED_SUCCESS )
	{
	}
	*/

	return 1;
}

VOID WXHal_CharNGet(UINT8 *ch, UINT16 dataLen)
{
	// sekim 20130318 stdio -> wiced_uart_xxx
	UINT32 ch_count = 0;
	while(1)
	{
		*ch = getchar();
		ch++;
		ch_count++;
		if ( ch_count>=dataLen )	break;
	}
	/*
	if ( wiced_uart_receive_bytes(WICED_UART_1, ch, dataLen, WICED_NEVER_TIMEOUT)!=WICED_SUCCESS )
	{
	}
	*/
}

VOID WXHal_CharPut(UINT8 ch)
{
	// sekim 20130318 stdio -> wiced_uart_xxx
	putchar(ch);
	/*
	if ( wiced_uart_transmit_bytes(WICED_UART_1, &ch, 1)!=WICED_SUCCESS )
	{
	}
	*/

	return;
}

VOID WXHal_CharNPut(const VOID *buf, UINT32 len)
{
	// sekim 20130318 stdio -> wiced_uart_xxx
	/*
	UINT32 i;
	for (i=0; i<len; i++)
	{
		if (isascii(((char*)buf)[i]))	printf("%c", ((char*)buf)[i]);
	}
	*/
	for (; len!=0; --len)
	{
		putchar(*(char*)buf++);
	}
	/*
	if ( wiced_uart_transmit_bytes(WICED_UART_1, buf, len)!=WICED_SUCCESS )
	{
	}
	*/

	return;
}



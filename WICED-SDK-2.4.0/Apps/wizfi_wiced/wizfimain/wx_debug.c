#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "wx_defines.h"

VOID PrintHexLine(UINT8 *payload, INT32 len, INT32 offset)
{
	INT32 i;
	INT32 gap;
	UINT8 *ch;

	W_RSP("%08x   ", (UINT32)(payload));

	ch = payload;
	for(i = 0; i < len; i++)
	{
		W_RSP("%02x ", *ch);

		ch++;
		if ( i == 7 )			W_RSP(" ");
	}
	if ( len < 8 )		W_RSP(" ");

	if ( len < 16 )
	{
		gap = 16 - len;
		for(i = 0; i < gap; i++)
			W_RSP("   ");
	}
	W_RSP("   ");

	ch = payload;
	for(i = 0; i < len; i++)
	{
		if ( isprint(*ch) )		W_RSP("%c", *ch);
		else					W_RSP(".");
		ch++;
	}

	W_RSP("\r\n");
	return;
}

VOID WDUMP(UINT8 *payload, INT32 len)
{
	INT32 len_rem = len;
	INT32 line_width = 16;
	INT32 line_len;
	INT32 offset = 0;
	UINT8 *ch = payload;

	if ( len <= 0 )
	{
		return;
	}

	if ( len <= line_width )
	{
		PrintHexLine(ch, len, offset);
		return;
	}

	for(;;)
	{
		line_len = line_width % len_rem;
		PrintHexLine(ch, line_len, offset);
		len_rem = len_rem - line_len;
		ch = ch + line_len;
		offset = offset + line_width;
		if ( len_rem <= line_width )
		{
			PrintHexLine(ch, len_rem, offset);
			break;
		}
	}
	return;
}

VOID PrintHexLineExt(UINT8 *payload, INT32 len, UINT32 offset, char* pData)
{
	INT32 i;
	INT32 gap;
	UINT8 *ch;
	char szBuff[256];

	sprintf(pData, "%09x   ", (UINT32)(payload));

	ch = payload;
	for(i = 0; i < len; i++)
	{
		sprintf(szBuff, "%02x ", *ch);
		strcat(pData, szBuff);

		ch++;
		if ( i == 7 )
			strcat(pData, " " );
	}
	if ( len < 8 )
		strcat(pData, " ");

	if ( len < 16 )
	{
		gap = 16 - len;
		for(i = 0; i < gap; i++)
			strcat(pData, "   ");
	}
	strcat(pData, "   ");

	ch = payload;
	for(i = 0; i < len; i++)
	{
		if ( isprint(*ch) )
		{
			sprintf(szBuff, "%c", *ch);
			strcat(pData, szBuff);
		}
		else
			strcat(pData, ".");
		ch++;
	}

	strcat(pData, "\r\n");
	return;
}

VOID WDUMPEXT(UINT8 *payload, UINT32 len)
{
	UINT32 len_rem = len;
	UINT32 line_width = 16;
	UINT32 k = 0;
	INT32 line_len;
	UINT32 offset = 0;
	UINT8 *ch = payload;

	char szLine1[100];
	char szPreLine[100];
	UINT8 preData[100];

	INT8 is_pre_print_dot = -1;
	INT8 is_pre_print_data = -1;
	INT8 pre_bRandomPattern = 0;

	if ( len <= 0 )
	{
		return;
	}

	if ( len <= line_width )
	{
		PrintHexLineExt(ch, len, offset, szLine1);
		W_RSP(szLine1);
		return;
	}

	for(;;)
	{
		line_len = line_width % len_rem;
		
		PrintHexLineExt(ch, line_len, offset, szLine1);		
		//printf(szLine1);

		////////////////////////////////////////////////////////////////////////		
		UINT8 option_print_dot = 0;

		if ( is_pre_print_data==-1 )
		{
			W_RSP(szLine1);
			is_pre_print_data = 1;
			is_pre_print_dot = 0;			
		}
		else
		{	
			if ( memcmp(preData, ch, line_len)!=0 )
			{
				UINT8 bRandomPattern = 0;
				for (k=1; k<line_len; k++)
				{
					if ( ch[k-1]!=ch[k] )
					{
						bRandomPattern = 1;
						break;
					}
				}
				
				if ( pre_bRandomPattern==bRandomPattern )
				{
					option_print_dot = 1;
				}

				pre_bRandomPattern = bRandomPattern;
			}
			else
			{
				option_print_dot = 1;
			}

			if ( option_print_dot )
			{
				if ( !is_pre_print_dot )
				{
					W_RSP("................................................................................\r\n");
					W_RSP("................................................................................\r\n");
					is_pre_print_dot = 1;
				}
				is_pre_print_data = 0;
			}
			else
			{
				if ( is_pre_print_data==0 )
					W_RSP(szPreLine);
				W_RSP(szLine1);
				is_pre_print_data = 1;
				is_pre_print_dot = 0;
			}
		}

		strcpy(szPreLine, szLine1);
		memcpy(preData, ch, line_len);
		////////////////////////////////////////////////////////////////////////

		len_rem = len_rem - line_len;
		ch = ch + line_len;
		offset = offset + line_width;
		if ( len_rem <= line_width )
		{
			PrintHexLineExt(ch, len_rem, offset, szLine1);
			W_RSP(szLine1);
		
			break;
		}
	}
	return;
}


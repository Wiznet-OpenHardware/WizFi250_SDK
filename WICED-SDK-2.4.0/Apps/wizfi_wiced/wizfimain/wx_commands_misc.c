
#include "wx_defines.h"
#include "wiced_platform.h"

#if 1 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
extern const struct WX_COMMAND g_WXCmdTable[];
#endif

UINT8 WXCmd_USET(UINT8 *ptr)
{
	UINT8	*p;
	UINT8	status;
	UINT32	baud_rate;
	UINT16  word_length, stop_bits, parity, flow_control;
	UINT32	param;

#if 1 //MikeJ 130410 ID1023 - Add =? function to USET
	if ( strcmp((char*)ptr, "?") == 0 )
	{
		char buffDisplay[30];
		uartinfo_to_str(buffDisplay, &g_wxProfile.usart_init_structure);
		W_RSP( "%s\r\n", buffDisplay );
		return WXCODE_SUCCESS;
	}
#endif

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &param);
	if (status != WXCODE_SUCCESS ) return WXCODE_EINVAL;
	baud_rate = param;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if		( strcmp((char*)p,"n") == 0 || strcmp((char*)p,"N") == 0 )		{ parity = USART_Parity_No;  }
	else if ( strcmp((char*)p,"e") == 0 || strcmp((char*)p,"E") == 0 )		{ parity = USART_Parity_Even;}
	else if	( strcmp((char*)p,"o") == 0 || strcmp((char*)p,"O") == 0 )		{ parity = USART_Parity_Odd; }
	else	return WXCODE_EINVAL;


	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &param);
	if (status != WXCODE_SUCCESS ) return WXCODE_EINVAL;

	if 		( param == 7 )
	{
		/* With this configuration a parity (Even or Odd) should be set */
		word_length = USART_WordLength_8b;
#if 1 //MikeJ 130410 ID1024 - USART Param condition change
		if(parity == USART_Parity_No) return WXCODE_EINVAL;
#endif
	}
	else if ( param == 8 )
	{
		if 		( parity == USART_Parity_No )	{ word_length = USART_WordLength_8b; }
		else	{ word_length = USART_WordLength_9b; }
	}
	else	return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if 		( strcmp((char*)p,"1") == 0 ) 		{ stop_bits = USART_StopBits_1;	 }
	else if ( strcmp((char*)p,"0.5") == 0 )	{ stop_bits = USART_StopBits_0_5;}
	else if ( strcmp((char*)p,"2") == 0 )		{ stop_bits = USART_StopBits_2;  }
	else if	( strcmp((char*)p,"1.5") == 0 )	{ stop_bits = USART_StopBits_1_5; }
	else 	return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
#if 1 //MikeJ 130410 ID1024 - USART Param condition change
	upstr((char*)p);
	if      ( strcmp((char*)p,"N") == 0  ) flow_control = USART_HardwareFlowControl_None;
	else if ( strcmp((char*)p,"HW") == 0 ) flow_control = USART_HardwareFlowControl_RTS_CTS;
	else 	return WXCODE_EINVAL;
#else
	if		( strcmp((char*)p,"n") == 0 || strcmp((char*)p,"N") == 0 )	{ flow_control = USART_HardwareFlowControl_None; }
	else if	( strcmp((char*)p,"r") == 0 || strcmp((char*)p,"R") == 0 )	{ flow_control = USART_HardwareFlowControl_RTS; }
	else if ( strcmp((char*)p,"c") == 0 || strcmp((char*)p,"C") == 0 )	{ flow_control = USART_HardwareFlowControl_CTS; }
	else if ( strcmp((char*)p,"rc") == 0 || strcmp((char*)p,"RC") == 0 )	{ flow_control = USART_HardwareFlowControl_RTS_CTS; }
	else 	return WXCODE_EINVAL;
#endif

	/* Set up USART 1 */
	g_wxProfile.usart_init_structure.USART_BaudRate 			= baud_rate;
	g_wxProfile.usart_init_structure.USART_WordLength          	= word_length;
	g_wxProfile.usart_init_structure.USART_Parity              	= parity;
	g_wxProfile.usart_init_structure.USART_StopBits            	= stop_bits;
	g_wxProfile.usart_init_structure.USART_HardwareFlowControl 	= flow_control;
	g_wxProfile.usart_init_structure.USART_Mode                	= USART_Mode_Rx | USART_Mode_Tx;

#if 1 // kaizen 20131108 ID1136 Bug Fixed which can't save serial information when use this command.
	Save_Profile();

	// kaizen 20131211 // kaizen 20131211 ID1149 Added extra parameter in order to do not restart in WXCmd_USET function.
	p = WXParse_NextParamGet(&ptr);
	if( strcmp((char*)p,"W")!= 0 )
	{
		WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
		WXS2w_SystemReset();
	}
#else
	USART_Cmd	( USART1, DISABLE );
	USART_Init	( USART1, &g_wxProfile.usart_init_structure );
	USART_Cmd	( USART1, ENABLE );
#endif

	return WXCODE_SUCCESS;
}

#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function
#if 0 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
UINT8 WXCmd_Echo(UINT8 *ptr)
#else
UINT8 WXCmd_MECHO(UINT8 *ptr)
#endif
{
	UINT8	*p, status;
	UINT32	param;

	if(strcmp((char*)ptr, "?") == 0)
	{
		W_RSP("%d\r\n", g_wxProfile.echo_mode);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if(!p) return WXCODE_EINVAL;

	status = WXParse_Int(p, &param);
	if (status != WXCODE_SUCCESS || (param != 0 && param != 1))
		return WXCODE_EINVAL;

	if(param == 1) g_wxProfile.echo_mode = TRUE;
	else g_wxProfile.echo_mode = FALSE;
	
	return WXCODE_SUCCESS;
}
#endif

#if 1 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
UINT8 WXCmd_MHELP(UINT8 *ptr)
{
	UINT8 i, tmpbuf[16], cmdidx=0;

	W_RSP("Command        Description & Usage\r\n");

	do {
		if(g_WXCmdTable[cmdidx].description == NULL) continue;

		memset(tmpbuf, 0, 16);	// Clear CMD Name Space
		tmpbuf[0] = 'A';
		tmpbuf[1] = 'T';

		for(i=0; i<13; i++) {		// Copy CMD Name to Buf
			if(g_WXCmdTable[cmdidx].cmd[i] == '=' || 
				g_WXCmdTable[cmdidx].cmd[i] == 0) break;
			tmpbuf[i+2] = g_WXCmdTable[cmdidx].cmd[i];
		}
		if(i >= 15) return WXCODE_FAILURE;

		W_RSP("%-15s%s\r\n", tmpbuf, g_WXCmdTable[cmdidx].description);
		if(g_WXCmdTable[cmdidx].usage != NULL) 
			W_RSP("               %s\r\n", g_WXCmdTable[cmdidx].usage);
	} while(g_WXCmdTable[++cmdidx].cmd != NULL);

	return WXCODE_SUCCESS;
}
#endif

// sekim 20130415 Certificate Management
#include "platform_dct.h"
#include "wiced_dct.h"

// sekim 20130415 Certificate Management
#include "platform_dct.h"
#include "wiced_dct.h"
#if 1	// kaizen 20130520 ID1068 Added Delete Certificate & Key Option
#define	DCT_SECURITY_SIZE 	sizeof(platform_dct_security_t)
UINT8 WXCmd_MCERT(UINT8 *ptr)
{
	UINT8 *p, status;
	UINT8 *p_alloc_input;
	platform_dct_security_t* p_alloc_dct;
	UINT8 write_dct=0, type_key=0;
	UINT32 length;

	platform_dct_security_t const* dct_security = wiced_dct_get_security_section( );

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if      ( strcmp((char*)p,"r") == 0 || strcmp((char*)p,"R") == 0 )	{ write_dct = MCERT_READ;  }
	else if ( strcmp((char*)p,"w") == 0 || strcmp((char*)p,"W") == 0 )	{ write_dct = MCERT_WRITE;  }
	else if ( strcmp((char*)p,"d") == 0 || strcmp((char*)p,"D") == 0 )	{ write_dct = MCERT_DELETE;  }
	else 	return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if      ( strcmp((char*)p,"c") == 0 || strcmp((char*)p,"C") == 0 )	{ type_key = MCERT_CERTIFICATE;  }
	else if ( strcmp((char*)p,"k") == 0 || strcmp((char*)p,"K") == 0 )	{ type_key = MCERT_KEY;  }
	else if ( strcmp((char*)p,"r") == 0 || strcmp((char*)p,"R") == 0 )	{ type_key = MCERT_ROOTCA; }			//kaizen 20140428 ID1171 Added for using ROOT CA
	else 	return WXCODE_EINVAL;

	if ( write_dct == MCERT_READ )
	{
		if ( type_key == MCERT_CERTIFICATE )	printf("%s\r\n", dct_security->certificate);
		else									printf("%s\r\n", dct_security->private_key);
	}
	else if ( write_dct == MCERT_DELETE )
	{
		p_alloc_dct = (platform_dct_security_t*)malloc(DCT_SECURITY_SIZE);

		if ( type_key == MCERT_CERTIFICATE )
		{
			strncpy((char*)p_alloc_dct->certificate, (char*)0xFF, CERTIFICATE_SIZE);
		}
		else if( type_key == MCERT_ROOTCA )																		//kaizen 20140428 ID1171 Added for using ROOT CA
		{
			wiced_tls_deinit_root_ca_certificates();
		}
		else
		{
			strncpy((char*)p_alloc_dct->private_key, (char*)0xFF, PRIVATE_KEY_SIZE);
		}

		if ( wiced_dct_write_security_section(p_alloc_dct)!=WICED_SUCCESS )
		{
			W_DBG("WXCmd_MCERT : wiced_dct_write_security_section error\r\n");
			free(p_alloc_dct);
			return WXCODE_FAILURE;
		}

		free(p_alloc_dct);
	}
	else
	{
		p = WXParse_NextParamGet(&ptr);
		if(!p) return WXCODE_EINVAL;


		status = WXParse_Int(p, &length);
		if ( status!=WXCODE_SUCCESS || length>=CERTIFICATE_SIZE )	return WXCODE_EINVAL;

		p_alloc_input = (UINT8*)malloc(CERTIFICATE_SIZE);
		if ( p_alloc_input==0 )	return WXCODE_EINVAL;
		p_alloc_dct = (platform_dct_security_t*)malloc(DCT_SECURITY_SIZE);
		if ( p_alloc_dct==0 )
		{
			W_DBG("WXCmd_MCERT : malloc error");
			free(p_alloc_input);
			return WXCODE_EINVAL;
		}

		memset(p_alloc_dct, 0, DCT_SECURITY_SIZE);
		memcpy(p_alloc_dct, dct_security, DCT_SECURITY_SIZE);

		memset(p_alloc_input, 0, CERTIFICATE_SIZE);

#if 1	// kaizen 20130516 ID1067 - Modified bug about occurring watchodg event at listening serial input
	wizfi_task_monitor_stop = 1;
	WXHal_CharNGet(p_alloc_input, length);
	wizfi_task_monitor_stop = 0;
	wiced_update_system_monitor(&wizfi_task_monitor_item, MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK);
#else
		WXHal_CharNGet(p_alloc_input, length);
#endif

		if ( type_key == MCERT_CERTIFICATE )
		{
			strcpy((char*)p_alloc_dct->certificate, (char*)p_alloc_input);
		}
		else if( type_key == MCERT_ROOTCA )																		//kaizen 20140428 ID1171 Added for using ROOT CA
		{
			wiced_tls_init_root_ca_certificates( (char*)p_alloc_input);
		}
		else
		{
			strcpy((char*)p_alloc_dct->private_key, (char*)p_alloc_input);
		}

		if ( wiced_dct_write_security_section(p_alloc_dct)!=WICED_SUCCESS )
		{
			W_DBG("WXCmd_MCERT : wiced_dct_write_security_section error\r\n");
			free(p_alloc_input);
			free(p_alloc_dct);
			return WXCODE_FAILURE;
		}

		free(p_alloc_input);
		free(p_alloc_dct);
	}

	return WXCODE_SUCCESS;
}
#else
UINT8 WXCmd_MCERT(UINT8 *ptr)
{
	UINT8 *p, status;
	UINT8 write_dct=0, type_key=0;

	platform_dct_security_t const* dct_security = wiced_dct_get_security_section( );

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if      ( strcmp((char*)p,"d") == 0 || strcmp((char*)p,"D") == 0 )	{ write_dct = 0;  }
	else if ( strcmp((char*)p,"w") == 0 || strcmp((char*)p,"W") == 0 )	{ write_dct = 1;  }
	else 	return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if      ( strcmp((char*)p,"c") == 0 || strcmp((char*)p,"C") == 0 )	{ type_key = 0;  }
	else if ( strcmp((char*)p,"k") == 0 || strcmp((char*)p,"K") == 0 )	{ type_key = 1;  }
	else 	return WXCODE_EINVAL;

	if ( write_dct==0 )
	{
		if ( type_key==0 )	printf("%s\r\n", dct_security->certificate);
		else				printf("%s\r\n", dct_security->private_key);
	}
	else
	{
		UINT32 length;
		p = WXParse_NextParamGet(&ptr);
		if(!p) return WXCODE_EINVAL;

#define TEMP_20130415 	sizeof(platform_dct_security_t)

		status = WXParse_Int(p, &length);
		if ( status!=WXCODE_SUCCESS || length>=CERTIFICATE_SIZE )	return WXCODE_EINVAL;

		UINT8* p_alloc_input = (UINT8*)malloc(CERTIFICATE_SIZE);
		if ( p_alloc_input==0 )	return WXCODE_EINVAL;
		platform_dct_security_t* p_alloc_dct = (platform_dct_security_t*)malloc(TEMP_20130415);
		if ( p_alloc_dct==0 )
		{
			W_DBG("WXCmd_MCERT : malloc error");
			free(p_alloc_input);
			return WXCODE_EINVAL;
		}

		memset(p_alloc_dct, 0, TEMP_20130415);
		memcpy(p_alloc_dct, dct_security, TEMP_20130415);

		memset(p_alloc_input, 0, CERTIFICATE_SIZE);
		WXHal_CharNGet(p_alloc_input, length);

		if ( type_key==0 )
		{
			strcpy((char*)p_alloc_dct->certificate, (char*)p_alloc_input);
		}
		else
		{
			strcpy((char*)p_alloc_dct->private_key, (char*)p_alloc_input);
		}

		char* xxx1 = p_alloc_dct->certificate;
		char* xxx2 = p_alloc_dct->private_key;

		if ( wiced_dct_write_security_section(p_alloc_dct)!=WICED_SUCCESS )
		{
			free(p_alloc_input);
			free(p_alloc_dct);
			return WXCODE_FAILURE;
		}

		free(p_alloc_input);
		free(p_alloc_dct);
	}

	return WXCODE_SUCCESS;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
// sekim 20131125 Add SPI Interface
void quick_change_sdtio_mode(UINT8 mode)
{
	char szCommand[50];
	sprintf(szCommand, "%d,%d", (unsigned int)mode, g_wxProfile.spi_mode);
	WXCmd_MSPI((UINT8*)szCommand);
}

UINT8 WXCmd_MSPI(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff_value1 = 0;
	UINT32 buff_value2 = 0;

	if ( strcmp((char*)ptr, "?")==0 )
	{
		W_RSP("%d,%d\r\n", (unsigned int)g_wxProfile.spi_stdio, g_wxProfile.spi_mode);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);	// 0: ALL, 1: UART, 2:SPI
	if(!p) return WXCODE_EINVAL;
	if( WXParse_Int(p, &buff_value1)!=WXCODE_SUCCESS || (buff_value1!= 0 && buff_value1!= 1 && buff_value1!= 2) )
		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
#if 1	// kaizen 20131202 When using UART mode, SPI Option parameter will be set zero.
	if(buff_value1 != 1)
	{
		if(!p) return WXCODE_EINVAL;
		if( WXParse_Int(p, &buff_value2)!=WXCODE_SUCCESS )
			return WXCODE_EINVAL;
		// (0) 0: Rising Edge,  1: Falling Edge
		// (1) 0: Idle Low,  1: Idle High
		// (3) 0: MSB First,  1: LSB First
		if( buff_value2!=0 && buff_value2!=1 && buff_value2!=2 && buff_value2!=3 && buff_value2!=8 && buff_value2!=9 && buff_value2!=10 && buff_value2!=11 )
			return WXCODE_EINVAL;
	}

#else
	if(!p) return WXCODE_EINVAL;
	if( WXParse_Int(p, &buff_value2)!=WXCODE_SUCCESS )
		return WXCODE_EINVAL;
	// (0) 0: Rising Edge,  1: Falling Edge
	// (1) 0: Idle Low,  1: Idle High
	// (3) 0: MSB First,  1: LSB First
	if( buff_value2!=0 && buff_value2!=1 && buff_value2!=2 && buff_value2!=3 && buff_value2!=8 && buff_value2!=9 && buff_value2!=10 && buff_value2!=11 )
		return WXCODE_EINVAL;
#endif

	g_wxProfile.spi_stdio = buff_value1;
	g_wxProfile.spi_mode = buff_value2;

	// Reset 
	Save_Profile();
	Apply_To_Wifi_Dct();
	WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
	WXS2w_SystemReset();

	return WXCODE_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "wiced.h"
#include "ctype.h"

#include "stm32f2xx.h"

// WizFi250 SPI CONTROL CODE
#define SPI_NULL	(uint8_t) 0xF0
#define SPI_ESC		(uint8_t) 0xF1
#define SPI_F0		(uint8_t) 0x00
#define SPI_F1		(uint8_t) 0x01
#define SPI_SYNC	(uint8_t) 0x02
#define SPI_XON		(uint8_t) 0x03
#define SPI_XOFF	(uint8_t) 0x04
#define SPI_ERR		(uint8_t) 0x05

wiced_spi_device_t g_wizfi_spi_handle = {WICED_SPI_1, WIZFI_SPI_CS, 30000000, SPI_NO_DMA, 8};
wiced_spi_message_segment_t g_wizfi_segments;
uint8_t g_wizfi_spi_tx[1024] = { 0, };
uint8_t g_wizfi_spi_rx[1024] = { 0, };

uint8_t g_print_mode = 3;
uint32_t g_spi_speed = 30000000;
uint32_t g_send_size = 1024;
uint32_t g_send_loopcount = 1;

//uint32_t g_option1 = 70;
//uint32_t g_option2 = 40;
uint32_t g_option1 = 30;
uint32_t g_option2 = 30;
uint32_t g_option3 = 20;

void stm32f2xx_clocks_needed    ( void );
void stm32f2xx_clocks_not_needed( void );
void SPI_I2S_SendData(SPI_TypeDef* SPIx, uint16_t Data);

void process_esc_code(uint8_t* spi_recv, uint8_t* valid_data);

void print_usage()
{
	printf("\r\n\r\n///////////////////////////////////////////////////////////\r\n");
	printf("// 0 : Send \"AT\" and check [OK] \r\n");
	printf("// 1 : <SPI to UART> Bypass mode\r\n");
	printf("// 2 : <Chip Select> for a while\r\n");
	printf("// 4 : Send Test 1 using g_send_size \r\n");
	printf("// 5 : Send Test 2(AT+SSEND) using g_send_loopcount\r\n");
	printf("// (Esc)    : Back to the menu \r\n");
	printf("// (option) : <p>print-mode(%d)(2: all print 3:ignore SPI_NULL(0xF0)) \r\n", g_print_mode);
	printf("//          : <o>Option  1(%lu) 2(%lu) 3(%lu) \r\n", g_option1, g_option2, g_option3);
	printf("//          : <s>SPI-reinitialize(%lu) \r\n", g_spi_speed);
	printf("//          : <t>Toggle send size(%lu) \r\n", g_send_size);
	printf("//          : <c>Toggle send loop count(%lu) \r\n", g_send_loopcount);
	printf("///////////////////////////////////////////////////////////\r\n");
}

////////////////////////////////////////////////////////////////////////////////////

uint32_t g_dwt_timer_start;
uint32_t g_dwt_timer_stop;

#define DEMCR_TRCENA    0x01000000

#define DEMCR           (*((volatile uint32_t *)0xE000EDFC))
#define CYCCNTENA       (1<<0)
#define DWT_CYCCNT      ((volatile uint32_t *)0xE0001004)

#define DWT_CTRL        (*((volatile uint32_t *)0xE0001000))

#define STOPWATCH_START { g_dwt_timer_start = (*DWT_CYCCNT); }
#define STOPWATCH_STOP  { g_dwt_timer_stop = (*DWT_CYCCNT); }

static volatile uint32_t g_dwt_ticks_for_1us = 0;
static volatile uint32_t g_dwt_ticks_for_1ms = 0;

static inline void dwt_reset(void)
{
    // Enable DWT
    DEMCR |= DEMCR_TRCENA;
    *DWT_CYCCNT = 0;

    // Enable CPU cycle counter
    DWT_CTRL |= (1<<0);
}

void dwt_init()
{
	RCC_ClocksTypeDef clocks;
	RCC_GetClocksFreq(&clocks);

	g_dwt_ticks_for_1us = clocks.SYSCLK_Frequency / 1000000;
	g_dwt_ticks_for_1ms = g_dwt_ticks_for_1us * 1000;

	dwt_reset();

	printf("dwt_init : SYSCLK_Frequency(%lu) g_dwt_ticks_for_1us(%lu) \r\n", clocks.SYSCLK_Frequency, g_dwt_ticks_for_1us);
}

static inline void dwt_delay_tick(uint32_t ticks)
{
	uint32_t tick_end = (*DWT_CYCCNT) + ticks;
    while ( (*DWT_CYCCNT)<tick_end );
}

static inline void dwt_delay_microseconds(uint32_t us)
{
	uint32_t tp = (*DWT_CYCCNT) + (us * g_dwt_ticks_for_1us);
	while ( (uint32_t)(*DWT_CYCCNT) < tp );
}

static inline void dwt_delay_milliseconds(uint32_t us)
{
	uint32_t tp = (*DWT_CYCCNT) + (us * g_dwt_ticks_for_1ms);
	while ( (uint32_t)(*DWT_CYCCNT) < tp );
}

void dwt_print_timer_status()
{
	uint32_t dwt_diff = g_dwt_timer_stop - g_dwt_timer_start;
	printf("dwt-start-tick(%lu), dwt-stop-tick(%lu), diff(%lu)\r\n", g_dwt_timer_start, g_dwt_timer_stop, dwt_diff);
	printf("diff : %lu ns \r\n", (uint32_t) ((1000. * dwt_diff)/g_dwt_ticks_for_1us));
}

uint32_t get_integer_from_uart(uint32_t old_value)
{
	uint32_t index_value = 0;
	uint8_t ch_data;
	uint8_t valid_data = 0;;
	uint8_t data_value[256];

	memset(data_value, 0, sizeof(data_value));

	printf("Input new value. (%lu) \r\n", old_value);

    while(1)
    {
    	if ( wiced_uart_receive_bytes(WICED_UART_1, &ch_data, 1, WICED_NEVER_TIMEOUT)!=WICED_SUCCESS )
    	{
    		printf("wiced_uart_receive_bytes error \r\n");
    		return old_value;
    	}

    	if ( ch_data=='\r' || ch_data=='\n' )
		{
    		printf("\r\n");
    		data_value[index_value++] = 0;
    		valid_data = 1;
    		break;
		}
    	else if ( ch_data>='0' || ch_data<='9' )
    	{
    		printf("%c", ch_data);
    		data_value[index_value++] = ch_data;
    	}
    	else if ( (ch_data)==0x1b )
    	{
    		break;
    	}
    }

    if ( valid_data )
    {
    	return (uint32_t)(atoi((char*)data_value));
    }

    return old_value;
}

void dwt_test()
{
	dwt_reset();
	STOPWATCH_START
	wiced_rtos_delay_milliseconds(1000);
	STOPWATCH_STOP
	dwt_print_timer_status();

	dwt_reset();
	STOPWATCH_START
	wiced_rtos_delay_milliseconds(2000);
	STOPWATCH_STOP
	dwt_print_timer_status();

	dwt_reset();
	STOPWATCH_START
	wiced_rtos_delay_milliseconds(3000);
	STOPWATCH_STOP
	dwt_print_timer_status();

	dwt_reset();
	STOPWATCH_START
	dwt_delay_tick(500000000);
	STOPWATCH_STOP
	dwt_print_timer_status();
}

////////////////////////////////////////////////////////////////////////////////////


void wizfi_spi_reinit()
{
	wiced_result_t result;
	result = wiced_spi_deinit(&g_wizfi_spi_handle);
    if ( result!=WICED_SUCCESS )
    {
    	printf("wiced_spi_deinit error (%d)\r\n", result);
    	return;
    }
	g_wizfi_spi_handle.speed = g_spi_speed;
	result = wiced_spi_init(&g_wizfi_spi_handle);
    if ( result!=WICED_SUCCESS )
    {
    	printf("wiced_spi_init error (%d)\r\n", result);
    	return;
    }
}

uint8_t wizfi_spi_send(uint8_t send_data)
{
	uint8_t recv_data;
	uint32_t i;

	wiced_gpio_output_low(g_wizfi_spi_handle.chip_select);
	for (i = 0; i < g_option1; i++) __asm__("nop");

	while ( SPI_I2S_GetFlagStatus( SPI1, SPI_I2S_FLAG_TXE ) == 0 );
	SPI_I2S_SendData(SPI1, send_data);
	while ( SPI_I2S_GetFlagStatus( SPI1, SPI_I2S_FLAG_RXNE ) == 0 );
	recv_data = (uint8_t)SPI_I2S_ReceiveData( SPI1 );

	for (i = 0; i < g_option2; i++) __asm__("nop");
	wiced_gpio_output_high(g_wizfi_spi_handle.chip_select);

	return recv_data;
}

uint8_t wizfi_spi_xsend(uint8_t send_data)
{
	uint8_t recv_data = wizfi_spi_send(send_data);

	if ( g_print_mode==1 )
	{
		printf("{0x%02x}", recv_data);
	}
	else if ( g_print_mode==2 )
	{
		if ( isprint(recv_data) || recv_data==0x0d || recv_data==0x0a )	printf("%c", recv_data);
		else 															printf("{0x%02x}", recv_data);
	}
	else if ( g_print_mode==3 )
	{
		if ( recv_data!=SPI_NULL )
		{
			if (isprint(recv_data) || recv_data==0x0d || recv_data==0x0a )	printf("%c", recv_data);
			else 															printf("{0x%02x}", recv_data);
		}
	}
	return recv_data;
}

// Send SPI_NULL and Wait String
uint8_t wizfi_wait_string(uint32_t wait_count, uint8_t* end_string, uint8_t* check_string)
{
	uint32_t i, recv_result_index = 0;
	uint8_t recv_data;
	uint8_t recv_result_buffer[1024] = {0,};

	// Wait check_string
	uint8_t found_string = 0;
	for (i=0; i<wait_count; i++)
	{
		if ( wiced_gpio_input_get(WIZFI_SPI_REQ)==WICED_TRUE )
		{
			recv_data = wizfi_spi_send(SPI_NULL);
			if (isprint(recv_data) || recv_data==0x0d || recv_data==0x0a )
			{
				recv_result_buffer[recv_result_index++] = recv_data;
				if ( strlen((char*)recv_result_buffer)>(sizeof(recv_result_buffer)-1) )
				{
					//printf((char*)recv_result_buffer);
					printf("wizfi_wait_string buffer overflow 2\r\n");
					return 1;
				}
			}

			if ( strstr((char*)recv_result_buffer, (char*)end_string) )
			{
				found_string = 1;
				break;
			}
		}
	}

	if ( found_string==1 )
	{
		if ( strstr((char*)recv_result_buffer, (char*)check_string) )
		{
			//printf((char*)recv_result_buffer);
			return 0;
		}
		else
		{
			printf("<found end string>");
		}
	}
	else
	{
		printf("<not found end string>\r\n");
	}

	printf((char*)recv_result_buffer);
	//printf("wizfi_wait_string can't find string. \r\n");

	return 1;
}

uint8_t wizfi_send_at_command(uint8_t* command_data, uint32_t wait_count, uint8_t* end_string, uint8_t* check_string)
{
	int i = 0;
	uint8_t recv_result_buffer[256] = {0,};

	memset(recv_result_buffer, 0, sizeof(recv_result_buffer));
	memset(g_wizfi_spi_tx, 0xf0, sizeof(g_wizfi_spi_tx));

	uint32_t command_length = (uint32_t)strlen((char*)command_data);
	memcpy(g_wizfi_spi_tx, command_data, command_length);

	for(i=0; i<command_length; i++)
	{
		//for (i = 0; i < g_option1; i++) __asm__("nop");
		wizfi_spi_xsend(g_wizfi_spi_tx[i]);
	}

	return wizfi_wait_string(wait_count, end_string, check_string);
}


void process_esc_code(uint8_t* spi_recv, uint8_t* valid_data)
{
	static uint8_t esc_mode = 0;
	static uint8_t esc2_mode = 0;

	(*valid_data) = 0;

	if ( (*spi_recv)==SPI_NULL )	{ return; }
	if ( (*spi_recv)==SPI_ESC )		{ esc_mode = 1;	return;	}

	if ( esc_mode==1 )
	{
		esc_mode = 0;
		switch((*spi_recv))
		{
		case SPI_F0:	// Pass 'F0' to upper
			(*valid_data) = 1;
			(*spi_recv) = SPI_NULL;
			break;
		case SPI_F1: // Pass 'F1' to upper
			(*valid_data) = 1;
			(*spi_recv) = SPI_ESC;
			break;
		case SPI_SYNC: // Handle Sync Signal
			g_wizfi_spi_rx[0] = wizfi_spi_xsend((uint8_t)SPI_ESC);
			if ( g_wizfi_spi_rx[0]!=SPI_NULL )	printf("Need SPI_NULL\r\n");
			g_wizfi_spi_rx[0] = wizfi_spi_xsend((uint8_t)SPI_SYNC);
			if ( g_wizfi_spi_rx[0]!=SPI_NULL )	printf("Need SPI_NULL\r\n");
			break;
		case SPI_XON: // Handle Xon Signal
			break;
		case SPI_XOFF: // Handle Xoff Signal
			{
				uint8_t found_xon = 0;
				uint16_t xon_count = 0;
				while(1)
				{
					wiced_rtos_delay_milliseconds(10);
					printf(".");

					xon_count++;
					if ( xon_count>=300 )
					{
						printf("\r\ncan't find xon. timeout. \r\n");
						break;
					}

					//spi2_recv = wizspix_byte(WIZ_SPI2, (uint8_t)SPI_NULL, 1, 0, 0, g_opt_print_mode);
					g_wizfi_spi_rx[0] = wizfi_spi_xsend((uint8_t)SPI_ESC);
					if ( g_wizfi_spi_rx[0]==SPI_NULL )	{ continue; }
					if ( g_wizfi_spi_rx[0]==SPI_ESC )	{ esc2_mode = 1;	return;	}

					if ( esc2_mode==1 )
					{
						esc2_mode = 0;
						switch(g_wizfi_spi_rx[0])
						{
							case SPI_XON: // Handle Xon Signal
								found_xon = 1;
								break;
							default:
								break;
						}
					}

					if ( found_xon==1 )			break;
				}
			}
			break;
		case SPI_ERR: // Handle Error Signal
			break;
		case SPI_ESC: // Lost ESC Data
			esc_mode = 1;	//Just Ignore previous ESC
			break;
		default:
			break;
		}
	}
	else
	{
		(*valid_data) = 1;
	}
}

void fill_dummy_data(uint8_t* ptr_src, uint32_t len)
{
	int i;
	ptr_src[0] = 'A';
	for (i=1; i<len; i++)
	{
		ptr_src[i] = ptr_src[i-1] + 1;
		if ( ptr_src[i]=='z' )  ptr_src[i] = 'A';
	}
}

void wizfi_send_xxx()
{
	int j;
	for (j=0; j<1024; j++)
	{
		wizfi_spi_send(g_wizfi_spi_tx[j]);
	}
}

void wizfi_send_enter()
{
	uint32_t i;
	uint8_t spi_command[256];
	strcpy((char*)spi_command, (char*)"\r");
	wizfi_send_at_command(spi_command, 100, (uint8_t*)"", (uint8_t*)"");
	for ( i=0; i<1024; i++)
	{
		wizfi_spi_send(SPI_NULL);
	}
}


void application_start( )
{
    wiced_result_t result;
    uint32_t i,j,k;
    (void)i;
    (void)j;
    (void)k;

    // turn off buffers, so IO occurs immediately
    setvbuf( stdin, NULL, _IONBF, 0 );
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );

	g_wizfi_spi_handle.mode |= 9;

	printf("\r\nInitializing SPI.....\r\n");
	result = wiced_spi_init(&g_wizfi_spi_handle);
	if ( result!=WICED_SUCCESS )
	{
		printf("wiced_spi_init error (%d) \r\n", result);
		return;
	}
	wiced_gpio_init(g_wizfi_spi_handle.chip_select, OUTPUT_PUSH_PULL);

	dwt_init();

    wiced_gpio_init(WIZFI_SPI_REQ, INPUT_PULL_UP);

    memset(g_wizfi_spi_tx, 0, sizeof(g_wizfi_spi_tx));
    memset(g_wizfi_spi_rx, 0, sizeof(g_wizfi_spi_tx));

    g_wizfi_segments.tx_buffer = g_wizfi_spi_tx;
    g_wizfi_segments.rx_buffer = g_wizfi_spi_rx;

    while(1)
    {
    	print_usage();

    	uint8_t ch_data;
    	uint8_t ch_data2;
    	uint8_t spi_valid_data;
    	uint8_t spi_command[256];
		uint8_t recv_data;

    	if ( wiced_uart_receive_bytes(WICED_UART_1, &ch_data, 1, WICED_NEVER_TIMEOUT)!=WICED_SUCCESS )
    	{
    		printf("wiced_uart_receive_bytes error \r\n");
    		return;
    	}

    	if ( ch_data=='p' )
		{
			g_print_mode++;
			if ( g_print_mode==4 )	g_print_mode = 1;
		}
		else if ( ch_data=='o' )
		{
			while(1)
			{
				printf("Option : 1(%lu) 2(%lu) 3(%lu) \r\n", g_option1, g_option2, g_option3);
				if ( wiced_uart_receive_bytes(WICED_UART_1, &ch_data2, 1, WICED_NEVER_TIMEOUT)!=WICED_SUCCESS )
				{
					printf("wiced_uart_receive_bytes error \r\n");
					return;
				}

				if ( ch_data2=='1' )		g_option1 = get_integer_from_uart(g_option1);
				else if ( ch_data2=='2' )	g_option2 = get_integer_from_uart(g_option2);
				else if ( ch_data2=='3' )	g_option3 = get_integer_from_uart(g_option3);
				else if ( ch_data2==0x1b )	break;
			}
		}
		else if ( ch_data=='s' )
		{
			if ( g_spi_speed==60000000/256 )	g_spi_speed = 30000000;
			else
			{
				g_spi_speed = g_spi_speed/2;
			}
			wizfi_spi_reinit();
		}
		else if ( ch_data=='t' )
		{
			if ( g_send_size==32 )				g_send_size = 1024;
			else if ( g_send_size==1024 )		g_send_size = 1024*10;
			else if ( g_send_size==1024*10 )	g_send_size = 1024*100;
			else if ( g_send_size==1024*100 )	g_send_size = 1024*1024;
			else if ( g_send_size==1024*10 )	g_send_size = 1024*100;
			else if ( g_send_size==1024*1024 )	g_send_size = 32;
		}
		else if ( ch_data=='c' )
		{
			if ( g_send_loopcount==1 )			g_send_loopcount = 10;
			else if ( g_send_loopcount==10 )	g_send_loopcount = 100;
			else if ( g_send_loopcount==100 )	g_send_loopcount = 1000;
			else if ( g_send_loopcount==1000 )	g_send_loopcount = 10000;
			else if ( g_send_loopcount==10000 )	g_send_loopcount = 1;
		}



		else if ( ch_data=='0' )
		{
			strcpy((char*)spi_command, (char*)"AT\r");
			if ( wizfi_send_at_command(spi_command, 1000, (uint8_t*)"]", (uint8_t*)"[OK]")==0 )
			{
				printf("Send AT, Found OK");
			}
		}
		else if ( ch_data=='1' )
    	{
    		printf("UART to SPI bypass mode \r\n");
			while(1)
			{
				if ( wiced_gpio_input_get(WIZFI_SPI_REQ)==WICED_TRUE )
				{
					recv_data = wizfi_spi_xsend((uint8_t)SPI_NULL);
					process_esc_code(&recv_data, &spi_valid_data);
				}

				if ( wiced_uart_receive_bytes(WICED_UART_1, &ch_data, 1, 0)==WICED_SUCCESS )
				{
					if ( (ch_data)==0x1b )		break;
					recv_data = wizfi_spi_xsend((uint8_t)ch_data);
					process_esc_code(&recv_data, &spi_valid_data);
				}

				wiced_rtos_delay_milliseconds(1);
			}
    	}
		else if ( ch_data=='2' )
		{
			uint8_t old_print_mode = g_print_mode;
			g_print_mode = 2;

			for (i=0; i<100; i++)
			{
				wizfi_spi_xsend((uint8_t)SPI_NULL);
			}
			g_print_mode = old_print_mode;
		}
		else if ( ch_data=='4' )
		{
			uint32_t total_sent = 0;
			fill_dummy_data((uint8_t*)(&g_wizfi_spi_tx[0]), 1024);

			i = 0;
			for ( total_sent=0; total_sent<g_send_size; total_sent++)
			{
				wizfi_spi_xsend(g_wizfi_spi_tx[i++]);
				if ( i>=1024 )	i = 0;
			}
		}
		else if ( ch_data=='3' )
		{
			strcpy((char*)spi_command, (char*)"\r");
			wizfi_send_at_command(spi_command, 100, (uint8_t*)"", (uint8_t*)"");
			for ( i=0; i<1024; i++)
			{
				wizfi_spi_send(SPI_NULL);
			}
		}
		else if ( ch_data=='5' )
		{
			uint8_t send_dummy_data[1024];
			//fill_dummy_data((uint8_t*)(&g_wizfi_spi_tx[0]), 1024);
			fill_dummy_data((uint8_t*)send_dummy_data, 1024);

			//for (i=0; i<g_send_loopcount; i++)
			for (i=0; i<g_send_loopcount; )
			{
				printf(".");

				strcpy((char*)spi_command, (char*)"AT\r");
				if ( wizfi_send_at_command(spi_command, 500000, (uint8_t*)"]\r\n", (uint8_t*)"[OK]\r\n")!=0 )
				{
					printf("<Can't found OK. Retry>");
					continue;
				}

				strcpy((char*)spi_command, (char*)"AT+SSEND=0,,,1024,2000\r");
				if ( wizfi_send_at_command(spi_command, 500000, (uint8_t*)"]\r\n", (uint8_t*)"1024,2000]\r\n")!=0 )
				{
					printf("<Can't found data length[1024]. Retry>");
					//i--;
					continue;
				}

				fill_dummy_data((uint8_t*)(&g_wizfi_spi_tx[0]), 1024);
				//fill_dummy_data((uint8_t*)send_dummy_data, 1024);
				//wizfi_send_xxx();
				for (j=0; j<1024; j++)		wizfi_spi_send(g_wizfi_spi_tx[j]);

				if ( wizfi_wait_string(5000000, (uint8_t*)"]\r\n", (uint8_t*)"[OK]")!=0 )
				{
					printf("<Can't found Send-OK. Retry>");

					continue;
				}
				i++;
			}
		}
		else if ( ch_data=='6' )
		{
			uint8_t send_dummy_data[1400];
			//fill_dummy_data((uint8_t*)(&g_wizfi_spi_tx[0]), 1024);
			fill_dummy_data((uint8_t*)send_dummy_data, 1400);

			for (i=0; i<g_send_loopcount; )
			{
				printf(".");

				///*
				strcpy((char*)spi_command, (char*)"AT\r");
				if ( wizfi_send_at_command(spi_command, 500000, (uint8_t*)"]\r\n", (uint8_t*)"[OK]")!=0 )
				{
					printf("<Can't found AT, OK. Retry>\r\n");
					wiced_rtos_delay_milliseconds(100);
					continue;
				}
				//*/

				strcpy((char*)spi_command, (char*)"AT+SSENDXX=0,,,1024\r");
				if ( wizfi_send_at_command(spi_command, 500000, (uint8_t*)"]\r\n", (uint8_t*)"1024]")!=0 )
				{
					printf("<Can't found AT+SSENDXX, data length[1024]. Retry>\r\n");
					wiced_rtos_delay_milliseconds(100);
					continue;
				}

				wiced_gpio_output_low(g_wizfi_spi_handle.chip_select);
				for (k = 0; k < g_option1; k++) __asm__("nop");

				while ( SPI_I2S_GetFlagStatus( SPI1, SPI_I2S_FLAG_TXE ) == 0 );
				SPI_I2S_SendData(SPI1, send_dummy_data[0]);
				while ( SPI_I2S_GetFlagStatus( SPI1, SPI_I2S_FLAG_RXNE ) == 0 );
				recv_data = (uint8_t)SPI_I2S_ReceiveData( SPI1 );

				for (k = 0; k < g_option1; k++) __asm__("nop");

				for (j=1; j<1024; j++)
				{
					for (k = 0; k < g_option3; k++) __asm__("nop");

					while ( SPI_I2S_GetFlagStatus( SPI1, SPI_I2S_FLAG_TXE ) == 0 );
					SPI_I2S_SendData(SPI1, send_dummy_data[j]);
					while ( SPI_I2S_GetFlagStatus( SPI1, SPI_I2S_FLAG_RXNE ) == 0 );
					recv_data = (uint8_t)SPI_I2S_ReceiveData( SPI1 );
				}
				for (k = 0; k < g_option2; k++) __asm__("nop");
				wiced_gpio_output_high(g_wizfi_spi_handle.chip_select);

				if ( wizfi_wait_string(5000000, (uint8_t*)"]\r\n", (uint8_t*)"[OK]")!=0 )
				{
					printf("<Can't found AT+SSENDXX, OK. Retry>\r\n");
					wiced_rtos_delay_milliseconds(100);
					continue;
				}
				i++;

				//wiced_rtos_delay_milliseconds(1);
			}
		}
    }

}


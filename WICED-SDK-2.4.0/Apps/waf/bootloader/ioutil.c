/**
  ******************************************************************************
  * @file    STM32F2xx_IAP/src/common.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    02-May-2011
  * @brief   This file provides all the common functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/** @addtogroup STM32F2xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "ioutil.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void Delay_us(uint8_t time_us)
{
	volatile uint8_t i, j;	//register

	for(i=0; i<time_us; i++) {
		for(j=0; j<5; j++) {	// 25CLK
			asm("nop");	//1CLK         
			asm("nop");	//1CLK         
			asm("nop");	//1CLK         
			asm("nop");	//1CLK         
			asm("nop");	//1CLK                  
		}      
	}					// 25CLK*0.04us=1us
}

void Delay_ms(uint16_t time_ms)
{
	volatile uint16_t i;	//register

	for(i=0; i<time_ms; i++) {
		Delay_us(250);
		Delay_us(250);
		Delay_us(250);
		Delay_us(250);
	}
}

/**
  * @brief  Convert an Integer to a string
  * @param  str: The string
  * @param  base: 10 or 16
  * @param  intnum: The integer to be converted
  * @retval None
  */
void Int2Str(uint8_t* str, uint8_t base, int64_t intnum)
{
	uint32_t Dvs, Dvd;
	uint8_t Unit, Cnt, i, j = 0;

	if(base == 16) {
		Dvs = 0x10000000;
		Unit = 0x10;
		Cnt = 8;
	} else {
		Dvs = 1000000000;
		Unit = 10;
		Cnt = 10;
	}

	if(intnum < 0) {
		str[j++] = '-';
		intnum *= -1;
	}

	if(intnum >= ((int64_t)Dvs * Unit)) {
		str[j++] = '*';
		str[j] = 0;
		return;
	}

	Dvd = (uint32_t)intnum;

	for (i = 0; i < Cnt; i++) {
		str[j++] = (Dvd / Dvs) + 0x30;
		if(str[j-1] > 0x39) str[j-1] += 7;
		Dvd %= Dvs;
		Dvs /= Unit;
		if (j == 1 && str[0] == '0') j = 0;
	}

	if(j==0) str[j++] = 0x30;
	str[j] = 0;
}

/**
  * @brief  Convert a string to an integer
  * @param  inputstr: The string to be converted
  * @param  intnum: The integer value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
	uint32_t i = 0, res = 0;
	uint32_t val = 0;

	if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
	{
		if (inputstr[2] == '\0') return 0;
		for (i = 2; i < 11; i++) {
			if (inputstr[i] == '\0') {
				*intnum = val;
				res = 1;	/* return 1; */
				break;
			}
			if (ISVALIDHEX(inputstr[i])) {
				val = (val << 4) + CONVERTHEX(inputstr[i]);
			} else {
				res = 0;	/* Return 0, Invalid input */
				break;
			}
		}		
		if (i >= 11) res = 0;	/* Over 8 digit hex --invalid */
	}
	else /* max 10-digit decimal input */
	{
		for (i = 0;i < 11;i++) {
			if (inputstr[i] == '\0') {
				*intnum = val;
				res = 1;	/* return 1 */
				break;
			} else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0)) {
				val = val << 10;
				*intnum = val;
				res = 1;
				break;
			} else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0)) {
				val = val << 20;
				*intnum = val;
				res = 1;
				break;
			} else if (ISVALIDDEC(inputstr[i])) {
				val = val * 10 + CONVERTDEC(inputstr[i]);
			} else {
				res = 0;	/* return 0, Invalid input */
				break;
			}
		}
		if (i >= 11) res = 0;	/* Over 10 digit decimal --invalid */
	}

	return res;
}

void Serial_Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	// 20131015 sekim Rinnai Baud Rate to 9600
	USART_InitStructure.USART_BaudRate = 115200;
	//USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

void Serial_Deinit(void)
{
	USART_Cmd(USART1, DISABLE);
	USART_DeInit(USART1);
}

/**
  * @brief  Print a character on the HyperTerminal
  * @param  c: The character to be printed
  * @retval None
  */
void SerialPutChar(uint8_t c)
{
	USART_SendData(USART1, c);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

/**
  * @brief  Print a string on the HyperTerminal
  * @param  s: The string to be printed
  * @retval None
  */
void Serial_PutString(uint8_t *s)
{
	while (*s != '\0') {
		SerialPutChar(*s);
		s++;
	}
}

/**
  * @brief  Test to see if a key has been pressed on the HyperTerminal
  * @param  key: The key pressed
  * @retval 1: Correct
  *         0: Error
  */
uint32_t SerialKeyPressed(uint8_t *key)
{
	if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
		*key = (uint8_t)USART1->DR;
		return 1;
	} else {
		return 0;
	}
}

/**
  * @brief  Get a key from the HyperTerminal
  * @param  None
  * @retval The Key Pressed
  */
uint8_t GetKey(void)
{
	uint8_t key = 0;

	/* Waiting for user input */
	while(1) {
		if(SerialKeyPressed(&key)) break;
	}
	return key;
}

/**
  * @brief  Get Input string from the HyperTerminal
  * @param  buffP: The input string
  * @retval None
  */
void GetInputString (uint8_t *buffP)
{
	uint32_t bytes_read = 0;
	uint8_t c = 0;

	while(1) {
		c = GetKey();
		if(c == '\r') break;
		if(c == '\b') { /* Backspace */
			if(bytes_read > 0) {
				SerialPutString("\b \b");
				bytes_read --;
			}
			continue;
		}
		if(bytes_read >= CMD_STRING_SIZE) {
			SerialPutString("Command string size overflow\r\n");
			bytes_read = 0;
			continue;
		}
		if(c >= 0x20 && c <= 0x7E) {
			buffP[bytes_read++] = c;
			SerialPutChar(c);
		}
	}
	SerialPutString("\r\n");
	buffP[bytes_read] = 0;
}

/**
  * @brief  Get an integer from the HyperTerminal
  * @param  num: The integer
  * @retval 1: Correct
  *         0: Error
  */
uint32_t GetIntegerInput(int32_t *num)
{
	uint8_t inputstr[16];

	while(1) {
		GetInputString(inputstr);
		if (inputstr[0] == '\0') continue;
		if ((inputstr[0] == 'a' || inputstr[0] == 'A') && inputstr[1] == '\0') {
			SerialPutString("User Cancelled \r\n");
			return 0;
		}

		if (Str2Int(inputstr, num) == 0) {
			SerialPutString("Error, Input again: \r\n");
		} else {
			return 1;
		}
	}
}

/**
  * @}
  */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/

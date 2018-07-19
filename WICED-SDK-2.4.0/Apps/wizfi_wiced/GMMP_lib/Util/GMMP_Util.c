/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Util.c
 **/

#include <string.h>
#include "GMMP_Util.h"


int  IsBigEndianSystem(void)
{
	int nVal = 1;
	unsigned char byBuf[4];

//	extern void *memcpy (void *__restrict __dest,
//			     __const void *__restrict __src, size_t __n)

	memcpy(byBuf, &nVal, sizeof(nVal) );
	if (byBuf[3] != 0)
		return 1;
	else
		return 0;
}

short  ltobs(const short nShort)
{
	if(IsBigEndianSystem() == 1)
	{
		return nShort;
	}

	return _btols(nShort);
}

short btols(const short nShort)
{
	if(IsBigEndianSystem() == 1)
	{
		return nShort;
	}

	return _btols(nShort);
}

short _btols(const short nShort)
{
	short nValue = 0;

	nValue = (unsigned short)nShort >> 8;
	nValue |=(unsigned short) nShort << 8;

	return  nValue;
}

int  ltobi(const int nInt)
{
	if(IsBigEndianSystem() == 1 )
	{
		return nInt;
	}

	return _ltobi(nInt);
}

int btoli(const int nInt)
{
	if(IsBigEndianSystem() == 1)
	{
		return nInt;
	}

	return _ltobi(nInt);
}

int _ltobi(const int nInt)
{
	int nVal0, nVal1, nVal2, nVal3;

	nVal0 =  (unsigned int)nInt >> 24;
	nVal1 =  (unsigned int)(nInt & 0x00FF0000) >> 8;
	nVal2 =  (unsigned int)(nInt & 0x0000FF00) << 8;
	nVal3 =  (unsigned int)nInt << 24;

	return (nVal0 | nVal1 | nVal2 | nVal3);
}

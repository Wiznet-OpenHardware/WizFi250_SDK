/** GMMP에 사용되는 Util 함수
 * @file GMMP_Util.h
 * @date 2013/11/21
 * @version 0.0.0.1
 **/
#ifndef GMMP_UTIL_H_
#define GMMP_UTIL_H_
/**
 *@brief 라이브러리를 사용중인 플랫폼 시스템이 big-endian, little-endian인지 확인한다.
 * @return big-endian : 0, little-endian : 1
 */
int		IsBigEndianSystem(void); //big endian system check

/**
 *  @brief short형 변수의 정보를 리틀엔디언 에서 빅엔디언으로 변경
 * @param nShort
 */
short	ltobs(const short nShort); //little endian to big endian for short

/**
 *  @brief short형 변수의 정보를  빅엔디언에서 리틀엔디언으로 변경
 * @param nShort
 */
short btols(const short nShort);

/**
 * @brief short형 변수의 정보 변환 내부 함수
 * @param nShort
 */
short _btols(const short nShort);

/**
 * @brief int형 변수의 정보를 리틀엔디언 에서 빅엔디언으로 변경
 * @param nInt
 * @return
 */
int		ltobi(const int nInt);

/**
 * @brief int형 변수의 정보를  빅엔디언에서 리틀엔디언으로 변경
 * @param nInt
 * @return
 */
int 		btoli(const int nInt);

/**
 * @brief int형 변수의 정보 변환 내부 함수
 * @param nInt
 * @return
 */
int 		_ltobi(const int nInt);

#endif /* GMMP_UTIL_H_ */

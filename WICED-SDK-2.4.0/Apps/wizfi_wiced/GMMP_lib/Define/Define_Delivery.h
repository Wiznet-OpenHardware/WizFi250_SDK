/** GMMP Operation 중 주기보고 명령 리스트 정의
 * @file Define_Delivery.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/
#ifndef DEFINE_DELIVERY_H_
#define DEFINE_DELIVERY_H_

#define DELIVERY_COLLECT_DATA			0x01 ///<OMP에 수집 데이터 보고 한다.
#define DELIVERY_ALARM_DATA				0x02 ///<OMP에 장애 데이터 보고 한다.
#define DELIVERY_EVENT_DATA				0x03 ///<OMP에 이벤트 데이터 보고 한다.
#define DELIVERY_ALARM_CLEAR			0x04 ///<OMP에 장애 해제 데이터 보고 한다.

#endif /* DEFINE_DELIVERY_H_ */

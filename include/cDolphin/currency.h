/*
   File:           currency.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The interface to translate currency number.
*/

#ifndef __CURRENCY_H_
#define __CURRENCY_H_

#ifdef __cplusplus
extern "C" {
#endif

int Int32ToCurrency( unsigned int nValue, char * );
int Int64ToCurrency( unsigned __int64 nValue, char * );
int DecimalToCurrency( double fValue, char * );

char *Int32ToCurrency2( unsigned int nValue );
char *Int64ToCurrency2( unsigned __int64 nValue );
char *DecimalToCurrency2( double fValue );

#ifdef __cplusplus
}
#endif

#endif  /* __CURRENCY_H_ */

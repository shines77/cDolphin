/*
   File:           currency.c

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Some translate to currency routines.
*/

#include <math.h>
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "currency.h"

static char str_buffer[64];

int Int32ToCurrency(unsigned int nValue, char *buffer) {
    int i, len, s;
    char buf[32];
    char *current;
    if (buffer == NULL)
        return 0;
    current = buffer;
    sprintf(buf, "%u", nValue);
    len = strlen(buf);
    for (i = 0; i < len; i++) {
        //sprintf( current++, "%c", buf[i] );
        *current++ = buf[i];
        s = len - i;
        if ((s == 4 || s == 7 || s == 10 ||
            s == 13 || s == 16)) {
            //sprintf( current++, "," );
            *current++ = ',';
        }
    }
    *current = '\0';
    return (current - buffer);
}

int Int64ToCurrency(unsigned __int64 nValue, char *buffer) {
    int i, len, s;
    char buf[32];
    char *current;
    if (buffer == NULL)
        return 0;
    current = buffer;
    sprintf(buf, "%I64u", nValue);
    len = strlen(buf);
    for (i = 0; i < len; i++) {
        //sprintf( current++, "%c", buf[i] );
        *current++ = buf[i];
        s = len - i;
        if ((s == 4 || s == 7 || s == 10 ||
            s == 13 || s == 16)) {
            //sprintf( current++, "," );
            *current++ = ',';
        }
    }
    *current = '\0';
    return (current - buffer);
}

int DecimalToCurrency(double fValue,
                      char *buffer) {
    int i, len, s;
    char buf[64];
    char *current;
    if (buffer == NULL)
        return 0;
    current = buffer;
    //fValue = (fValue >= 0.0f) ? floor( fValue ) : ceil(fValue);
    sprintf(buf, "%.0f", fValue);
    len = strlen(buf);
    for (i = (fValue < 0.0f); i < len; i++) {
        //sprintf( current++, "%c", buf[i] );
        *current++ = buf[i];
        s = len - i;
        if ((s == 4 || s == 7 || s == 10 ||
            s == 13 || s == 16)) {
            //sprintf( current++, "," );
            *current++ = ',';
        }
    }
    *current = '\0';
    return (current - buffer);
}

char *Int32ToCurrency2(unsigned int nValue) {
    int i, len, s;
    char buf[32];
    char *current = str_buffer;
    sprintf(buf, "%u", nValue);
    len = strlen(buf);
    for (i = 0; i < len; i++) {
        //sprintf( current++, "%c", buf[i] );
        *current++ = buf[i];
        s = len - i;
        if ((s == 4 || s == 7 || s == 10 ||
            s == 13 || s == 16)) {
            //sprintf( current++, "," );
            *current++ = ',';
        }
    }
    *current = '\0';
    return str_buffer;
}

char *Int64ToCurrency2(unsigned __int64 nValue) {
    int i, len, s;
    char buf[32];
    char *current = str_buffer;
    sprintf(buf, "%I64u", nValue);
    len = strlen(buf);
    for (i = 0; i < len; i++) {
        //sprintf( current++, "%c", buf[i] );
        *current++ = buf[i];
        s = len - i;
        if ((s == 4 || s == 7 || s == 10 ||
            s == 13 || s == 16)) {
            //sprintf( current++, "," );
            *current++ = ',';
        }
    }
    *current = '\0';
    return str_buffer;
}

char *DecimalToCurrency2(double fValue) {
    int i, len, s;
    char buf[64];
    char *current = str_buffer;
    //fValue = (fValue >= 0.0f) ? floor( fValue ) : ceil(fValue);
    sprintf(buf, "%.0f", fValue);
    len = strlen(buf);
    for (i = (fValue < 0.0f); i < len; i++) {
        //sprintf( current++, "%c", buf[i] );
        *current++ = buf[i];
        s = len - i;
        if ((s == 4 || s == 7 || s == 10 ||
            s == 13 || s == 16)) {
            //sprintf( current++, "," );
            *current++ = ',';
        }
    }
    *current = '\0';
    return str_buffer;
}

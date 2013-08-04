/*
   File:           utils.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Some utility routines.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

INLINE Boolean Eqv(Boolean bArg1, Boolean bArg2) {
	return bArg1 ? bArg2 : !bArg2;
}

INLINE Boolean Xor(Boolean bArg1, Boolean bArg2) {
	return bArg1 ? !bArg2 : bArg2;
}

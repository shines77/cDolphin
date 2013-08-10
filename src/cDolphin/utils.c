/*
   File:           utils.c

   Created:        2007-07-10

   Modified:       2013-08-10

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

/*
int main(int argc, char **argv)
{
	//
	return 0;
}
//*/

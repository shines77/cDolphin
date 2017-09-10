/*
   opname.h

   Automatically created by OSF on Sun Nov 24 16:01:10 2002
*/

#ifndef __OPNAME_H_
#define __OPNAME_H_

#define OPENING_COUNT       76

typedef struct tagOpeningDescriptor {
    const char *name;
    const char *sequence;
    int hash_val1;
    int hash_val2;
    int level;
} OpeningDescriptor;

extern OpeningDescriptor opening_list[OPENING_COUNT];

#endif  /* __OPNAME_H_ */

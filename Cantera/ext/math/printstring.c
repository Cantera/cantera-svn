#include <stdio.h>
#include "config.h"

typedef  int          ftnlen;

#ifdef __cplusplus
extern "C" {
#endif
void printstring_(char* s, ftnlen ls) 
{
    printf("%s",s);
}
#ifdef __cplusplus
}
#endif    

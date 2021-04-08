#ifndef opCode
#define opCode

#include "xc.h"
#include "stdio.h"
#include "stdbool.h"
#include "configBits.h"
#include "I2C.h"
#include "lcd.h"

#include "helpers.h"
#include "lcdCode.h"
void runOp(unsigned char time[3], unsigned char *pTotalSupplied, 
    unsigned char *pNumPoles, poleInfo *infoArr, 
    volatile bool *pInt0P, unsigned char *opTime);

/*
struct poleInfo {
    unsigned char id;
    int pos;
    unsigned char tiresSupp;
    unsigned char tiresPresent;
};
typedef struct poleInfo poleInfo; */

#endif
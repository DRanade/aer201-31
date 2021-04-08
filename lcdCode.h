#ifndef lcdCode
#define lcdCode

#include "I2C.h"
#include "lcd.h"
#include "stdio.h"
#include "helpers.h"

void showTime(unsigned char pTime[7]);
void scrDoneMain(void);
void scrDoneGen(unsigned int time, unsigned char tireCount, 
        unsigned char poles);
void scrDonePoleDet(poleInfo poleData[15],int currPole);
void dispOpProg(void);
void dispProcMode(unsigned char mode);
void detectFeedback(void);
void tooManyTires(void);
void lcdNorm(void);
void dispShowLogs(void);
void dispLogMain(unsigned char log);
void dispLogGen(unsigned char log);
void dispLogPoleDet(unsigned char log, unsigned char currPole);
#endif

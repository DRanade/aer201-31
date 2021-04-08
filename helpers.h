#ifndef helpers
#define helpers

#include "I2C.h"
#include "lcd.h"
#include "stdio.h"
#include "stdbool.h"

struct poleInfo {
    unsigned char id;
    int pos;
    unsigned char tiresSupp;
    unsigned char tiresPresent;
};
typedef struct poleInfo poleInfo;

#include "lcdCode.h"

void setupUART(void);
void fixTime(unsigned char Time[]);
void getTime(unsigned char pTime[3]);
void dispSMrun(char direc, unsigned int motorPos);
void dispSMsrun(char direc1, char direc2);
unsigned char dispStateTrans(unsigned char keypress, unsigned char dispMode, 
        unsigned char startTime[3], unsigned char *pStepDir, 
        unsigned char *pStepDir2, unsigned char* pLogToShow, 
        unsigned char lastWritten);
void runSM(unsigned char direction, unsigned int *pSMposition, 
        volatile bool *int1Pressed);
void runSMA(unsigned char dirA);
void runSMB(unsigned char dirB);
void runSMs(unsigned char dirA, unsigned char dirB);
void runSMsControl(unsigned char dirA, unsigned char dirB, 
        volatile bool *int1Pressed);
void deployLeft(void);
void deployRight(void);
void deployBoth(void);
void tireDeploy(unsigned char amount, unsigned char firstTank);





void write_EEPROM(unsigned char address, unsigned char data);
unsigned char read_EEPROM(unsigned char address);
void shift_EEPROM(void);

void storeLog(unsigned char time, unsigned char numPoles, 
        unsigned char tiresDep, poleInfo* infoArr, unsigned char* pLastLog);

#endif
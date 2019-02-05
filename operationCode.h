#include "xc.h"
#include "stdio.h"
#include "stdbool.h"
#include "configBits.h"
#include "I2C.h"
#include "lcd.h"

#include "getTime.h"
#include "lcdCode.h"
void runOp(unsigned char time[3], unsigned char *pTotalSupplied, unsigned char *pNumPoles, poleInfo infoArr, volatile bool *pInt1P, unsigned char *opTime){
    INT1IE = 0;                     // disable sensor/keypad interrupt
    LATCbits.LATC5 = 1;             // shunt keypad (disable it)
    // start time was already pulled when start button was pressed
    *pTotalSupplied = 0;
    *pNumPoles = 0;
    int i;
    // lcd display
    unsigned int rampPos = 0;
    unsigned int pos = 0;
    unsigned char procMode = 0;     // 0-Moving out     1-Base Detected
                                    // 2-Pole Detected  3-Tires Deployed
                                    // 4-Moving Back
    lcd_home();
    printf("  THE OPERATION ");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("  IS CURRENTLY  ");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("   IN PROGRESS  ");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("                ");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    for (i=0; i<13; i++){            // potential progress bar left in for demo
        __delay_ms(250);
        printf("-");
    }
    while(1){   // poll through machine operation. Emergency stop still works
        if (*pInt1P){       // state transition portion
            *pInt1P = false;        // we know that interrupt happened. 
                                    // time to change state
            if (procMode == 0){     // a base was detected
                INT1IE = 0;         // disable detector interrupt
                procMode = 1;
            } else if (procMode == 1){  // if pole detected, move on to deploy
                INT1IE = 0;
                procMode = 2;
            } 
        }
        if (procMode == 0){         // moving out
            LATEbits.LATE2 = 0;     // sets active detector to be base detector
                                    // by telling detector selector this
            INT1IE = 1;             // enables the base detector
            LATCbits.LATC7 = 1;     // tells coprocessor we want to move
            unsigned char oldPosSig = PORTCbits.RC6;
            while (*pInt1P == false){   // counts current position
                if (oldPosSig == ~PORTCbits.RC6){
                    pos += 1;
                    oldPosSig = oldPosSig ? 0 : 1 ;
                }
            }
        } else if (procMode == 1){  // base detected. moving ramp out
            detectFeedback();       // shows feedback after detecting base
            infoArr[*pNumPoles].id = *pNumPoles + 1;
            infoArr[*pNumPoles].pos = pos;
            LATEbits.LATE2 = 1;     // sets active detector to be pole detector
                                    // through selector
            INT1IE = 1;             // enables the pole detector
            while (*pInt1P == false){   // move out ramp until pole is detected
                // Push the ramp out, keeping track of position
                // Code here. Outline similar to coprocessor running motor
                //
                //
            }
            LATAbits.LATA4 = 0;     // code to stop the motor
        } else if (procMode == 2){ // tire deploy
            unsigned char tireVar = 0;
            if (PORTBbits.RB2){
                tireVar += 1;
            }
            if (PORTBbits.RB3){
                tireVar += 1;
            }
            if (*pNumPoles == 0){       // follow algorithm and calc how many to deploy
                                        // this is first pole case
                tireVar = 2-tireVar;
                infoArr[*pNumPoles].tiresPresent = 2;
            } else if ((pos - infoArr[*pNumPoles-1].pos) < 300){ // within 30cm
                if (tireVar == 2){ tooManyTires(); }
                tireVar = 1-tireVar;
                infoArr[*pNumPoles].tiresPresent = 1;
            } else {
                tireVar = 2-tireVar;
                infoArr[*pNumPoles].tiresPresent = 2;
            }
            tireDeploy(tireVar,(*pTotalSupplied)%2);    // deploy tires and update data variables
            infoArr[*pNumPoles].tiresSupp = tireVar;
            *pTotalSupplied += tireVar;
            detectFeedback();           // send LCD back into "operation in progress"
            while (tireVar != infoArr[*pNumPoles].tiresPresent){    // confirm
                                                    // that tires have arrived
                tireVar = 0;
                if (PORTBbits.RB2){
                    tireVar += 1;
                }
                if (PORTBbits.RB3){
                    tireVar += 1;
                }
            }
            *pNumPoles += 1;    // finally increments number of detected poles
            procMode = 3;
        } else if (procMode == 3){  // move forward while retracting ramp
                                    // and checking for base
            // send move command to coprocessor
            // start ramp motor in opposite direction
            LATEbits.LATE2 = 0;     // sets active detector to be base detector
                                    // by telling detector selector this
            LATCbits.LATC7 = 1;     // tells coprocessor we want to move
            unsigned char distSinceBase = 0;
            unsigned char oldPosSig = PORTCbits.RC6;
            unsigned int clicks = 0;               // confirm we actually need ints for this
            unsigned char read = 0;
            LATAbits.LATA5 = 1; // motor direction
            LATAbits.LATA4 = 1; // set motor enable pin to 'on'
            while (*pInt1P == false){   // counts current position
                if (oldPosSig == ~PORTCbits.RC6){
                    pos += 1;
                    oldPosSig = oldPosSig ? 0 : 1 ;
                    distSinceBase += 1;
                    if (distSinceBase == 10){                                       // potential param to change later
                        INT1IE = 1;             // enables the base detector
                    }
                }
                if (read != PORTEbits.RE0){    // has position of encoder changed?
                    read = PORTEbits.RE0;      // update it
                    // confirm direction by reading PORTCbits.RE1
                    clicks += 1;               // add click count
                    if (clicks >= THRESHOLD){  // threshold tbd (equal to number of clicks in a millimeter)
                        clicks = 0;
                        rampPos -= 1;
                        if (rampPos <= 0){                                                  // potential param to change later
                            procMode == 0;
                            // disable ramp motor
                            LATAbits.LATA4 = 0;
                            break;
                        }
                    }
                }
                
            }            
                // move in ramp by changing direction and following similar
                // outline to coprocessor section
                // poll for ramp position
            // poll for position variable
            // poll for ramp position
            // this is all above
            LATAbits.LATA4 = 0; // disable ramp motor if base detected first
        } else if (procMode == 4){
            LATCbits.LATC7 = 1;     // tells coprocessor we want to move
            unsigned char oldPosSig = PORTCbits.RC6;
            while (pos > 0){   // counts current position
                if (oldPosSig == ~PORTCbits.RC6){
                    pos -= 1;
                    oldPosSig = oldPosSig ? 0 : 1 ;
                }
            }
            // record end time
            unsigned char endTime[3];
            getTime(endTime);
            if (endTime[2] > time[2]){  // checks to see if the hour has rolled over
                                        // it would otherwise blow out of integer size
                endTime[2] -= 1;
                time[2] += 60;
            }
            *opTime = endTime[1]*60+endTime[0] - (time[1]*60-time[0]);
            return;
        }
    }
};

void deployLeft(void){
    // insert stepper motor code (from other file) for RB4-RB7
}

void deployRight(void){
    // insert stepper motor code (from other file) for RA0-RA3
}

void deployBoth(void){
    // insert stepper motor code (from other file) for both RB4-RB7 and RA0-RA3
    // at the same time
}

void tireDeploy(unsigned char amount, unsigned char firstTank){
    if (amount == 1){
        if (firstTank == 1){    // 1 denotes left tank is first to deploy
            deployLeft();
        } else {
            deployRight();
        }
    } else {
        deployBoth();
    }
}

void detectFeedback(void){
    lcd_home();
    printf("!!!!! POLE !!!!!");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("! SUCCESSFULLY !");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("!   DETECTED   !");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("!!!!!!!!!!!!!!!!");
    // make sound through speaker
}

void tooManyTires(void){
    lcd_home();
    printf("!!!!!ERROR!!!!!!");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("!  THIS  POLE  !");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("! HAS TOO MANY !");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("! TIRES ON IT  !");
    // make sound through speaker
}

void lcdNorm(void){
    lcd_home();
    printf("  THE OPERATION ");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("  IS CURRENTLY  ");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("   IN PROGRESS  ");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("                ");
}

/*

// code for coprocessor:
    // set tris appropriately
    while (1){  // loops moving forwards and backwards forever
        unsigned int position = 0;
        unsigned int rClicks = 0;           // confirm we actually need ints for this
        unsigned char rRead = 0;
        unsigned int lClicks = 0;
        unsigned char lRead = 0;
        unsigned char motorDir = 0;
        unsigned char direction = 0;    // moving out
        // set both motor direction pins to 'outwards'
        while (position < 4000){
            if (COM1){                  // find out which pin is COM1
                // set both motor enable pins to 'on'
            } else {
                // set both motor enable pins to 'off'
            }
            // reading position on both encoders
            if (lRead != PORTCbits.RC4){    // has position of encoder changed?
                lRead = PORTCbits.RC4;      // update it
                // confirm direction by reading PORTCbits.RC0
                lClicks += 1;               // add click count
                if (lClicks >= THRESHOLD){  // threshold tbd (equal to number of clicks in a millimeter)
                    lClicks = 0;
                    position += 1;
                    COM2 = ~COM2;           // so main processor knows position has ticked
                }
            }
            if (rRead != PORTCbits.RC5){    // has position of encoder changed?
                rRead = PORTCbits.RC5;      // update it
                rClicks += 1;               // add click count
                if (rClicks >= THRESHOLD){  // threshold tbd (equal to number of clicks in a millimeter)
                    rClicks = 0;
                }
            }
            // compare rClicks with lClicks and use PWM to change the enable outputs
            // so that both motors turn at the same speed
            // 
            // continue polling motor encoders and changing motor speeds actively
        }
        direction = 1;
        while (position >= 0){
            if (COM1){                  // find out which pin is COM1
                // set both motor enable pins to 'on'
            } else {
                // set both motor enable pins to 'off'
            }
            // reading position on both encoders
            if (lRead != PORTCbits.RC4){    // has position of encoder changed?
                lRead = PORTCbits.RC4;      // update it
                // confirm direction by reading PORTCbits.RC0
                lClicks += 1;               // add click count
                if (lClicks >= THRESHOLD){  // threshold tbd (equal to number of clicks in a millimeter)
                    lClicks = 0;
                    position += 1;
                    COM2 = ~COM2;           // so main processor knows position has ticked
                }
            }
            if (rRead != PORTCbits.RC5){    // has position of encoder changed?
                rRead = PORTCbits.RC5;      // update it
                rClicks += 1;               // add click count
                if (rClicks >= THRESHOLD){  // threshold tbd (equal to number of clicks in a millimeter)
                    rClicks = 0;
                }
            }
            // compare rClicks with lClicks and use PWM to change the enable outputs
            // so that both motors turn at the same speed
            // 
            // continue polling motor encoders and changing motor speeds actively
        }
    }

*/

/* struct poleInfo {
    unsigned char id;
    int pos;
    unsigned char tiresSupp;
    unsigned char tiresPresent;
};
typedef struct poleInfo poleInfo; */
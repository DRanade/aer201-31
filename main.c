// include Libraries
#include "xc.h"
#include "stdio.h"
#include "stdbool.h"
#include "configBits.h"
#include "I2C.h"
#include "lcd.h"

// include subfiles
#include "operationCode.h"
#include "helpers.h"

volatile bool int1Pressed = false;          // these variables keep track of
volatile bool int0Pressed = false;          // whether an interrupt happened
unsigned char totalTime = 0;                // total time in seconds
unsigned char tiresSupp = 0;                // total tires supplied by machine
unsigned char poleCount = 0;                // total number of poles
unsigned char currPole = 0;                 // current pole on display LCD
poleInfo poleInfoArr[15];                   // stores pole information. See
                                            // helpers.h for struct definition
unsigned char tick = 0;                     // only updates time every 50 ticks
                                            // otherwise, LCD looks bad
unsigned char lastStored = 3;           // used for access to log (range: 0-3)
unsigned char logToShow = 0;                // used for logs to access EEPROM

void main(void){
    ADCON1 = 0b00001111;            // Set all A/D ports to digital (pg. 222)
    INT1IE = 1;                     // Enable RB1 (keypad data) interrupt
    INT0IE = 0;                     // Disable Base (interrupt) sensor for now
    ei();                           // enable interrupts
    
    TRISA = 0b11000000; TRISB = 0b11111111; // config pins as input or output
    TRISC = 0b11011001; TRISD = 0b00000000; TRISE = 0b00001110;
    
    LATDbits.LATD1 = 0; LATAbits.LATA5 = 0; // stop machine (by telling arduino)
    
    LATAbits.LATA1 = 0; LATAbits.LATA3 = 0; // stop ramp motor
    
    LATCbits.LATC2 = 0;             // turn off indicator LED
    
    LATCbits.LATC5 = 0;             // enable keypad (enable it)
    
    initLCD();                      // initializes LCD screen
    
    I2C_Master_Init(100000);        // used for communication with RTC
    
    unsigned char time[7];          // stores time read from RTC
    
    unsigned char startTimeInt[3];  // stores hours,mins,secs of start of run
    
    unsigned char dispMode = 0;     // see index of display modes below
    // 0-standby        1-runningOp     2-doneOpMain    3-doneGen   
    // 4-donePoleDetail 5-pageTurnLeft  6-pageTurnRight 7-testAndDebug
    // 8-showLogMain    9-showLogsSelect    10-logGeneral
    // 11-logPoleDet    12-logPageTurnLeft  13-logPageTurnRight

    unsigned char direction1 = 1;       // controls stepper motor
    unsigned char direction2 = 1;       // directions in debug 

    unsigned int SMposition = 0;        // records SMposition during debug mode

    if (read_EEPROM(63) < 5){           // address 63 stores last written run
        lastStored = read_EEPROM(63);    
    }
    
    while(1){       // main polling loop which cycles through modes as required
        if(int1Pressed){                // if keypad is pressed, update state
            int1Pressed = false;        // indicates keypress resolved
            dispMode = dispStateTrans(((PORTB & 0xF0) >> 4),dispMode,
                            startTimeInt,&direction1, &direction2, 
                            &logToShow, lastStored);
        }
        if (dispMode == 0){             // show standby screen with time
            if (tick >= 50){            // only update time if 50 ticks passed
                showTime(time);
                tick = 0;
            }
            tick ++;
        } else if (dispMode == 1){      // show 'opertation status' screens
            runOp(startTimeInt,&tiresSupp,&poleCount,poleInfoArr,&int0Pressed,
                    &totalTime);        // runs operation
            TRISB = 0b11111011;         // reconfigure input/output pins
            INT1IE = 1;                 // re-enable keypad interrupt
            LATCbits.LATC5 = 0;         // unshunt keypad (reenable it)
            
            // store most recent operation details
            storeLog(totalTime,poleCount,tiresSupp,poleInfoArr,&lastStored);
            
            dispMode = 2;               // change disp state to 'post-operation'
        } else if (dispMode == 2){      // show 'doneMain'
            scrDoneMain();              // main Done screen
            currPole = 0;               // resets currently displayed pole
        } else if (dispMode == 3){      // show 'doneGeneralInfo'
            scrDoneGen(totalTime,tiresSupp,poleCount);
        } else if (dispMode == 4){      // show pole details of 'currPole'
            scrDonePoleDet(poleInfoArr,currPole);
        } else if (dispMode == 5){      // decrement current pole if possible
            if (currPole > 0){ currPole -= 1; }
            dispMode = 4;               // and then display its details
            scrDonePoleDet(poleInfoArr,currPole);
        } else if (dispMode == 6){      // increment current pole if possible
            if (currPole+1 < poleCount){ currPole += 1; }
            dispMode = 4;               // and then display details
            scrDonePoleDet (poleInfoArr,currPole);
        } else if (dispMode == 7){      // runs SM 1 with select keypad controls
            dispSMrun(direction1, SMposition);  // controls are secret menu
            runSM(direction1,&SMposition,&int1Pressed);
        } else if (dispMode == 8){      // logs menu screen
            dispShowLogs();
        } else if (dispMode == 9){      // 'main' screen for selectedlog
            dispLogMain(logToShow);
            currPole = 0;
        } else if (dispMode == 10){     // general details for selected log
            dispLogGen(logToShow);
        } else if (dispMode == 11){     // pole details for selected log
            dispLogPoleDet(logToShow, currPole);
        } else if (dispMode == 12){     // decrement pole if possible
            if (currPole > 0){ currPole -= 1; }
            dispMode = 11;              // and then display details
            dispLogPoleDet(logToShow, currPole);
        } else if (dispMode == 13){     // increment pole if possible
            if (currPole+1 < read_EEPROM(64*logToShow+1)){ currPole += 1; }
            dispMode = 11;              // and then display details
            dispLogPoleDet(logToShow, currPole);
        }
        __delay_ms(10);    // delay while polling so LCD doesn't go crazy
    }
}

void __interrupt() interruptHandler(void){
    if(INT1IF){     // Interrupt keypad handler
        int1Pressed = true; // update status so program knows keypress happened
        INT1IF = 0;         // clear flag
    }
    else if (INT0IF){       // other interrupt (used during operation as 'base')
        int0Pressed = true; // update status so program knows base detected
        INT0IF = 0;         // clear flag
    }
}
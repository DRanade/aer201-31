// include pic things
#include "xc.h"
#include "stdio.h"
#include "stdbool.h"
#include "configBits.h"
#include "I2C.h"
#include "lcd.h"

// include subfiles
#include "lcdCode.h"
#include "getTime.h"
//#include "operationCode.h"

volatile bool int1Pressed = false;          // interrupt thing
unsigned char totalTime = 0;                // total time in seconds
unsigned char tiresSupp = 0;                // total tires supplied by machine
unsigned char poleCount = 0;                // total number of poles
unsigned char currPole = 0;                 // current pole on display
poleInfo poleInfoArr[15];                   // stores pole info
unsigned int position = 0;

void main(void){
    ADCON1 = 0b00001111;            // Set all A/D ports to digital (pg. 222)
    INT1IE = 1;                     // Enable RB1 (keypad data) interrupt
    INT0IE = 1;                     // Enable Emergency Stop Interrupt
    ei();                           // enable interrupts
    TRISA = 0b11000000;             // config pins as input or output
    TRISB = 0b11111111;                                                         // Remember to change tris B7: B4 back to output after indiv eval.
    TRISC = 0b01011000;
    TRISD = 0b00000000;
    TRISE = 0b00001011;
    initLCD();
    I2C_Master_Init(100000);        // this section shows standby screen
    unsigned char time[7];          // time read from RTC
    unsigned char startTimeInt[3];
    unsigned char dispMode = 0; 
    // 0-standby  1-running   2-doneMain  3-doneGen   
    // 4-donePoleDet  5-pageTurnLeft  6-pageTurnRight
    int ticks = 0;
// testing data
poleInfoArr[0].id = 1;
poleInfoArr[0].pos = 258;
poleInfoArr[0].tiresSupp = 1;
poleInfoArr[0].tiresPresent = 2;
poleInfoArr[1].id = 2;
poleInfoArr[1].pos = 285;
poleInfoArr[1].tiresSupp = 0;
poleInfoArr[1].tiresPresent = 1;
poleInfoArr[2].id = 3;
poleInfoArr[2].pos = 558;
poleInfoArr[2].tiresSupp = 2;
poleInfoArr[2].tiresPresent = 2;
poleInfoArr[3].id = 4;
poleInfoArr[3].pos = 1038;
poleInfoArr[3].tiresSupp = 0;
poleInfoArr[3].tiresPresent = 2;
poleCount = 4;
// end testing data
    while(1){           // polling loop
        // Screen Transition Function
        if(int1Pressed){
            int1Pressed = false; // Clear the flag
            unsigned char keypress = (PORTB & 0xF0) >> 4;   // Read the Keypad
            if (keypress == 0x0D && dispMode == 0) {        // start button pres
                getTime(time);                              // gets start time
                for(unsigned int i = 0; i<3; i++){
                    startTimeInt[i] = time[i];
                }
                dispMode = 1;           // send from 'standby' to 'running'
            } else if (dispMode == 1){  // send from 'running' to 'doneMain'
                dispMode = 2;
            } else if (dispMode == 2){  // send from 'doneMain' 
                if (keypress == 0x00){  // to 'doneGenDetails'
                    dispMode = 3;
                } else if (keypress == 0x01){   // to 'donePoleDetails'
                    dispMode = 4;
                // Add reset (of motor)!
                } else if (keypress == 0x0D){   // to 'standby'
                    dispMode = 0;
                }
            } else if (dispMode == 3){  // send from 'doneGenDetails'
                if (keypress == 0x0D){  // to 'doneMain'
                    dispMode = 2;
                }
            } else if (dispMode == 4){  // send from 'donePoleDetails'
                if (keypress == 0xC){   // to <previousPole>
                    dispMode = 5;
                } else if (keypress == 0x0D){   // to <nextPole>
                    dispMode = 2;
                } else if (keypress == 0x0E){   // to 'doneMain'
                    dispMode = 6;
                }
            } 
        }
        if (dispMode == 0){     // show start screen with time
            if (ticks == 10){
                showTime(time);
                ticks = 0;
            }
            ticks ++;
        } else if (dispMode == 1){  // show 'in progress scene'
            // runOp(startTimeInt,&tiresSupp,&poleCount,poleInfoArr,&int1Pressed,&totalTime);
                            unsigned int i;
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
            dispMode = 2;
        } else if (dispMode == 2){  // show 'doneMain' and reset  current pole
            scrDoneMain();
            currPole = 0;
        } else if (dispMode == 3){  // show 'doneGen'
            scrDoneGen(totalTime,tiresSupp,poleCount);
        } else if (dispMode == 4){  // show pole details
            scrDonePoleDet(poleInfoArr,currPole);
        } else if (dispMode == 5){  // decrement current pole if possible
            if (currPole > 0){ currPole -= 1; } // and then display details
            dispMode = 4;
            scrDonePoleDet(poleInfoArr,currPole);
        } else if (dispMode == 6){  // increment current pole if possible
            if (currPole+1 < poleCount){ currPole += 1; }
            dispMode = 4;
            scrDonePoleDet (poleInfoArr,currPole);  // and then display details
        }
        __delay_ms(100);    // delay while polling
    }
}

void __interrupt() interruptHandler(void){
    if(INT1IF){     // Interrupt keypad handler
        int1Pressed = true;     // update status
        INT1IF = 0;         // clear flag
    }
    else if (INT0IF){    // Emergency Stop
        // assign all motor enable pins to zero
        // set dispMode to 0 (standby)
        // set procMode to 0 100 (emergency stopped)
        // Display LCD message
        unsigned char intTime[7];
            // Reset RTC memory pointer
            I2C_Master_Start(); // Start condition
            I2C_Master_Write(0b11010000); // 7 bit RTC address + Write
            I2C_Master_Write(0x00); // Set memory pointer to seconds
            I2C_Master_Stop(); // Stop condition

            // Read current time
            I2C_Master_Start(); // Start condition
            I2C_Master_Write(0b11010001); // 7 bit RTC address + Read
            for(unsigned char i = 0; i < 6; i++){
                intTime[i] = I2C_Master_Read(ACK); // Read with ACK to continue reading
            }
            intTime[6] = I2C_Master_Read(NACK); // Final Read with NACK
            I2C_Master_Stop(); // Stop condition

            // Print received data on LCD
            lcd_home();
            printf("TEAM 31     TIRE");
            lcd_set_ddram_addr(LCD_LINE2_ADDR);
            printf("    %02x/%02x/%02x    ", intTime[6],intTime[5],intTime[4]); // Print date in YY/MM/DD
            lcd_set_ddram_addr(LCD_LINE3_ADDR);
            printf("    %02x:%02x:%02x    ", intTime[2],intTime[1],intTime[0]); // HH:MM:SS
            lcd_set_ddram_addr(LCD_LINE4_ADDR);
            printf("MACHINE  STOPPED");
    }
}
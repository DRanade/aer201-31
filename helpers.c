#include "helpers.h"

void fixTime(unsigned char Time[]){
    // takes raw data from RTC and makes it readable
    // RTC sends the characters over as hexidecimal numbers of decimal digits
    // this fixes it
    Time[2] = ((Time[2])>>4)*10 + ((Time[2])&0x0F);
    Time[1] = ((Time[1])>>4)*10 + ((Time[1])&0x0F);
    Time[0] = ((Time[0])>>4)*10 + ((Time[0])&0x0F);
}

void getTime(unsigned char pTime[3]){
    // Reset RTC memory pointer
    I2C_Master_Start();             // Start communication
    I2C_Master_Write(0b11010000);   // with device at this address
    I2C_Master_Write(0x00);
    I2C_Master_Stop();

    // Read current time
    I2C_Master_Start();
    I2C_Master_Write(0b11010001);
    for(unsigned char i = 0; i < 2; i++){
        pTime[i] = I2C_Master_Read(ACK);        // Read data
    }
    pTime[2] = I2C_Master_Read(NACK);           // Final Read data
    
    I2C_Master_Stop();                          // Stop communication
}

void dispSMrun(char direc, unsigned int motorPos){
    // shows display screen when running stepper motor in debug mode
    lcd_home();
    printf(" STEPPER  MOTOR ");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf(" RUNNING DIR %s", (direc==1) ? "CCW\0":"CW \0");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("%16x",motorPos);
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("1:SWITCH  D:STOP");
}

void dispSMsrun(char direc1, char direc2){
    // shows display screen when running both stepper motors in debug mode
    lcd_home();
    printf(" STEPPER  MOTOR ");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("A DIR %s       ", (direc1==1) ? "CCW\0":"CW \0");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("B DIR %s       ", (direc2==1) ? "CCW\0":"CW \0");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("1:SWITCH  D:STOP");
}

unsigned char dispStateTrans(unsigned char keypress, unsigned char dispMode, 
        unsigned char startTime[3], unsigned char *pStepDir1, unsigned char 
        *pStepDir2, unsigned char* pLogToShow, unsigned char lastWritten){
    // changes display state based on which key is pressed
    if (dispMode == 0){                 // if currently in standby mode
        
        if (keypress == 0x0D){          // start button press
            
            getTime(startTime);         // reads time of operation start
            fixTime(startTime);
            return 1;
        } else if (keypress == 0x0F){   // 'D' for debug mode pressed
            return 7;
        } else if (keypress == 0x09){   // '8' pressed for logs
            return 8;
        } else if (keypress == 0x03){   // 'A' pressed
            LATAbits.LATA1 = 1; LATAbits.LATA3 = 0;     // brings in ramp motor
            while (PORTCbits.RC0 == 0){                 // until button pressed
                continue;
            }
            LATAbits.LATA1 = 0; LATAbits.LATA3 = 0;     // stops ramp motor
            
            return 0;                   // back to 'standby'
            
        } else if (keypress == 0x07){   //'B' pressed
            LATAbits.LATA1 = 0; LATAbits.LATA3 = 1;     // pushes out ramp a bit
            __delay_ms(50);
            LATAbits.LATA1 = 0; LATAbits.LATA3 = 0;     // stops ramp after 50ms
            
            return 0;                   // back to standby
            
        } else if (keypress == 0x02){   // '3' pressed
            
            runSMA(0);                  // brings SM A back for one tire length
            
        } else if (keypress == 0x06){   // '6' pressed
            
            LATCbits.LATC5 = 1;         // disables keypad
            
            TRISB = 0b10101011;         // requires setting SM pins as output
            
            runSMB(1);                  // brings SM B back for one tire length
            
            TRISB = 0b11111111;         // sets them back to input
            
            LATCbits.LATC5 = 0;         // enable keypad
        }
    } else if (dispMode == 2){          // send from 'doneMain' 
        
        if (keypress == 0x08){          // to 'doneGenDetails'
            return 3;
        } else if (keypress == 0x0A){   // to 'donePoleDetails'
            return 4;
        } else if (keypress == 0x0D){   // to 'standby'
            return 0;
        }
    } else if (dispMode == 3){          // send from 'doneGenDetails'
        
        if (keypress == 0x0D){          // to 'doneMain'
            return 2;
        }
    } else if (dispMode == 4){          // send from 'donePoleDetails'
        
        if (keypress == 0xC){           // to <previousPole>
            return 5;
        } else if (keypress == 0x0D){   // to 'doneMain'
            return 2;
        } else if (keypress == 0x0E){   // to <nextPole>
            return 6;
        }
    } else if (dispMode == 7){          // debug mode
        
        if (keypress == 0x00){          // switch direction of SM when '1' press
            (*pStepDir1) = ((*pStepDir1) == 0) ? 1:0;
            return 7;
        } else if (keypress == 0x01){
            (*pStepDir2) = ((*pStepDir2) == 0) ? 1:0;
            return 7;
        } else {                        // go back to standby if any other key
            return 0;
        }
    } else if (dispMode == 8){          // logs select screen
        
        if (keypress == 0){                         // log 1 (newest)
            *pLogToShow = (lastWritten + 4)%4;
            return 9;
        } else if (keypress == 1){                  // log 2
            *pLogToShow = (lastWritten + 3)%4;
            return 9;
        } else if (keypress == 2){                  // log 3
            *pLogToShow = (lastWritten + 2)%4;
            return 9;
        } else if (keypress == 4){                  // log 4 (oldest)
            *pLogToShow = (lastWritten + 1)%4;  // <pLogToShow tells processor
            return 9;                           // which log needs to be shown
        
        } else if (keypress == 0x0D){               // back to menu
            return 0;
        }
        return dispMode;
    } else if (dispMode == 9){  // send from log 'doneMain' 
        if (keypress == 0x08){  // to log 'doneGenDetails'
            return 10;
        } else if (keypress == 0x0A){   // to log 'donePoleDetails'
            return 11;
        } else if (keypress == 0x0D){   // back to log select menu
            if (read_EEPROM((*pLogToShow)*64+1) > 128){     // if this condition
                                        // is true, there is no log stored there
                return 8;
            }
            return 0;
        }
    } else if (dispMode == 10){         // send from log 'doneGenDetails'
        
        if (keypress == 0x0D){          // to log 'doneMain'
            return 9;
        }
    } else if (dispMode == 11){         // send from log 'donePoleDetails'

        if (keypress == 0xC){           // to log <previousPole>
            return 12;
        } else if (keypress == 0x0D){   // to log 'doneMain'
            return 9;
        } else if (keypress == 0x0E){   // to log <nextPole>
            return 13;
        }
    } else{
        return dispMode;        // invalid keypress ==> dispMode doesn't change
    }
    return dispMode;
}

void runSM(unsigned char direction, unsigned int *pSMposition, 
        volatile bool *int1Pressed){
    // this function runs SM A in debug mode, keeping track of position
    unsigned char ticksA = 0;   // increments every loop (for storing position)
    unsigned char hundreds = 1; // need two variables because screw shaft
                                // is too long and PIC variables are too small
    while(1){
        if (direction == 0){    // direction CW
            ticksA = (ticksA+1)%8;      // rolls over to next position
            hundreds++;
            if (hundreds >= 100){       // this bit increments position
                (*pSMposition)++;
                hundreds = 0;
            }
        } else { 
            ticksA = (ticksA-1)%8;      // same stuff for direction CCW
            hundreds--;
            if (hundreds <= 0){
                (*pSMposition)--;
                hundreds = 100;
            }
        }
        __delay_ms(0.7);                // delay so motor can keep up
        if (ticksA == 0){               // each of 8 different states of motor
            LATA = (LATA & 0xEA) | 0x01;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 1){
            LATA = (LATA & 0xEA) | 0x05;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 2){
            LATA = (LATA & 0xEA) | 0x04;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 3){
            LATA = (LATA & 0xEA) | 0x14;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 4){
            LATA = (LATA & 0xEA) | 0x10;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 5){
            LATA = (LATA & 0xEA) | 0x10;
            LATEbits.LATE0 = 1;    
        } else if (ticksA == 6){
            LATA = (LATA & 0xEA) | 0x00;
            LATEbits.LATE0 = 1;
        } else if (ticksA == 7){
            LATA = (LATA & 0xEA) | 0x01;
            LATEbits.LATE0 = 1;
        }
        if (*int1Pressed){          // if certain keypress, break out of loop
            if ((PORTB & 0xF1) != 0x31){
                break;
            }
        }
    }
    while((PORTB & 0xF1) == 0x31){  // hold here until key released
                                    // this was used to determine how many turns
                                    // per tire length
        continue;
    }
}

void runSMA(unsigned char dirA){    // same logic as above SM function
                                    // but this runs only SM A
                                    // but only runs for one tire length
    unsigned char subCtr = 0;
    unsigned char ctr = 0;
    unsigned char ticksA = 0;
    while(1){
        subCtr++;
        if (subCtr >= 40){
            subCtr = 0;
            ctr++;
        }
        __delay_ms(0.7);
        if (dirA == 0){ 
            ticksA = (ticksA+1)%8; 
        } else { 
            ticksA = (ticksA-1)%8; 
        }
        if (ticksA == 0){
            LATA = (LATA & 0xEA) | 0x01;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 1){
            LATA = (LATA & 0xEA) | 0x05;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 2){
            LATA = (LATA & 0xEA) | 0x04;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 3){
            LATA = (LATA & 0xEA) | 0x14;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 4){
            LATA = (LATA & 0xEA) | 0x10;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 5){
            LATA = (LATA & 0xEA) | 0x10;
            LATEbits.LATE0 = 1;    
        } else if (ticksA == 6){
            LATA = (LATA & 0xEA) | 0x00;
            LATEbits.LATE0 = 1;
        } else if (ticksA == 7){
            LATA = (LATA & 0xEA) | 0x01;
            LATEbits.LATE0 = 1;
        }
        if (ctr >= 197){
            return;
        }
    }
}

void runSMB(unsigned char dirB){    // same logic as above SM function
                                    // but this runs only SM B
                                    // but only runs for one tire length
    unsigned char subCtr = 0;
    unsigned int ctr = 0;
    unsigned char ticksB = 0;
    while(1){
        subCtr++;
        if (subCtr >= 40){
            subCtr = 0;
            ctr++;
        }
        __delay_ms(0.7);
        if (dirB == 0){ 
            ticksB = (ticksB+1)%8; 
        } else { 
            ticksB = (ticksB-1)%8; 
        }
        if (ticksB == 0){
            LATB = (LATB & 0xAB) | 0x04;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 1){
            LATB = (LATB & 0xAB) | 0x14;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 2){
            LATB = (LATB & 0xAB) | 0x10;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 3){
            LATB = (LATB & 0xAB) | 0x50;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 4){
            LATB = (LATB & 0xAB) | 0x40;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 5){
            LATB = (LATB & 0xAB) | 0x40;
            LATDbits.LATD0 = 1;    
        } else if (ticksB == 6){
            LATB = (LATB & 0xAB) | 0x00;
            LATDbits.LATD0 = 1;
        } else if (ticksB == 7){
            LATB = (LATB & 0xAB) | 0x04;
            LATDbits.LATD0 = 1;
        }
        if (ctr >= 197){
            return;
        }
    }
}

void runSMs(unsigned char dirA, unsigned char dirB){    // same logic as above 
                                // but this runs both SMs
                                // SM function but only runs for one tire length
    unsigned char subCtr = 0;
    unsigned int ctr = 0;
    unsigned char ticksA = 0;
    unsigned char ticksB = 0;
    while(1){
        subCtr++;
        if (subCtr >= 40){
            subCtr = 0;
            ctr++;
        }
        __delay_ms(0.7);
        if (dirA == 0){ 
            ticksA = (ticksA+1)%8; 
        } else { 
            ticksA = (ticksA-1)%8; 
        }
        if (dirB == 0){ 
            ticksB = (ticksB+1)%8; 
        } else { 
            ticksB = (ticksB-1)%8; 
        }
        if (ticksA == 0){
            LATA = (LATA & 0xEA) | 0x01;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 1){
            LATA = (LATA & 0xEA) | 0x05;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 2){
            LATA = (LATA & 0xEA) | 0x04;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 3){
            LATA = (LATA & 0xEA) | 0x14;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 4){
            LATA = (LATA & 0xEA) | 0x10;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 5){
            LATA = (LATA & 0xEA) | 0x10;
            LATEbits.LATE0 = 1;    
        } else if (ticksA == 6){
            LATA = (LATA & 0xEA) | 0x00;
            LATEbits.LATE0 = 1;
        } else if (ticksA == 7){
            LATA = (LATA & 0xEA) | 0x01;
            LATEbits.LATE0 = 1;
        }
        if (ticksB == 0){
            LATB = (LATB & 0xAB) | 0x04;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 1){
            LATB = (LATB & 0xAB) | 0x14;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 2){
            LATB = (LATB & 0xAB) | 0x10;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 3){
            LATB = (LATB & 0xAB) | 0x50;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 4){
            LATB = (LATB & 0xAB) | 0x40;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 5){
            LATB = (LATB & 0xAB) | 0x40;
            LATDbits.LATD0 = 1;    
        } else if (ticksB == 6){
            LATB = (LATB & 0xAB) | 0x00;
            LATDbits.LATD0 = 1;
        } else if (ticksB == 7){
            LATB = (LATB & 0xAB) | 0x04;
            LATDbits.LATD0 = 1;
        }
        if (ctr >= 197){
            return;
        }
    }
}

void runSMsControl(unsigned char dirA, unsigned char dirB, 
        volatile bool *int1Pressed){
    // same logic as above SM functions, but this one runs both of them
    // and allows changing direction as reqd
    unsigned char ticksA;
    unsigned char ticksB;
    TRISB = 0b00001011;
    while(1){
        __delay_ms(0.7);
        if (dirA == 0){ 
            ticksA = (ticksA+1)%8; 
        } else { 
            ticksA = (ticksA-1)%8; 
        }
        if (dirB == 0){ 
            ticksB = (ticksB+1)%8; 
        } else { 
            ticksB = (ticksB-1)%8; 
        }
        if (ticksA == 0){
            LATA = (LATA & 0xEA) | 0x01;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 1){
            LATA = (LATA & 0xEA) | 0x05;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 2){
            LATA = (LATA & 0xEA) | 0x04;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 3){
            LATA = (LATA & 0xEA) | 0x14;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 4){
            LATA = (LATA & 0xEA) | 0x10;
            LATEbits.LATE0 = 0;
        } else if (ticksA == 5){
            LATA = (LATA & 0xEA) | 0x10;
            LATEbits.LATE0 = 1;    
        } else if (ticksA == 6){
            LATA = (LATA & 0xEA) | 0x00;
            LATEbits.LATE0 = 1;
        } else if (ticksA == 7){
            LATA = (LATA & 0xEA) | 0x01;
            LATEbits.LATE0 = 1;
        }
        if (ticksB == 0){
            LATB = (LATB & 0xAB) | 0x04;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 1){
            LATB = (LATB & 0xAB) | 0x14;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 2){
            LATB = (LATB & 0xAB) | 0x10;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 3){
            LATB = (LATB & 0xAB) | 0x50;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 4){
            LATB = (LATB & 0xAB) | 0x40;
            LATDbits.LATD0 = 0;
        } else if (ticksB == 5){
            LATB = (LATB & 0xAB) | 0x40;
            LATDbits.LATD0 = 1;    
        } else if (ticksB == 6){
            LATB = (LATB & 0xAB) | 0x00;
            LATDbits.LATD0 = 1;
        } else if (ticksB == 7){
            LATB = (LATB & 0xAB) | 0x04;
            LATDbits.LATD0 = 1;
        }
        if (*int1Pressed){
            TRISB = 0b11111011;
            break;
        }
    }
}

void deployLeft(void){                  // deploy only from tank A
    runSMA(1);
}

void deployRight(void){                 // deploy only from tank B
    runSMB(0);
}

void deployBoth(void){                  // deploy from both tanks
    runSMs(1,0);
}

void tireDeploy(unsigned char amount, unsigned char firstTank){
    if (amount == 0){                   // tire deployment function
                                        // does nothing if none to deploy
        return;
    }
    else if (amount == 1){              // if one to deploy, chooses which
                                        // tank which has not most recently
                                        // deployed
        if (firstTank == 0){
            deployLeft();
        } else {
            deployRight();
        }
    } else {
        deployBoth();
    }
}

void write_EEPROM(unsigned char address, unsigned char data){
    //checking if not busy with an earlier write.
    while( EECON1bits.WR  ){continue;} 

    EECON1bits.WREN=1;              // Enable writing to EEPROM 
    EEADR=address;                  // load address 
    EEDATA=data;                    // load data
    EECON1bits.EEPGD=0;             // access EEPROM memory
    EECON1bits.CFGS=0;              // avoid access configuration registers
    INTCONbits.GIE=0;               // disable interrupts for critical 
                                    // EEPROM write sequence
                                    // required sequence start
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
                                    // required sequence end
    INTCONbits.GIE = 1;             // enable interrupts, critical sequence
                                    // complete
    while (EECON1bits.WR==1);       // wait for write to complete
    EECON1bits.WREN=0;              // do not allow EEPROM writes
}

unsigned char read_EEPROM(unsigned char address){
    while( EECON1bits.WR  ){continue;}  //checking if busy
    
    EEADR = address;                // load address 
    EECON1bits.EEPGD = 0;           // access EEPROM memory
    EECON1bits.CFGS  = 0;           // avoid access configuration registers
    EECON1bits.RD    = 1;           // read 
    return( EEDATA );
}

void storeLog(unsigned char time, unsigned char numPoles, 
        unsigned char tiresDep, poleInfo* infoArr, unsigned char* pLastLog){
    // stores all relevent information for operation
    unsigned char currAdr = 0;
    unsigned char i=0;
    unsigned char currLog = (*pLastLog+1)%4;
    *pLastLog = currLog;            // changes which log was written last
    write_EEPROM(63,*pLastLog);     // stores which log was written last
    currAdr = currLog*64;           // goes to start of space for correct log
    
    // organization is as follows: Each run takes 63 byte of space as below:
    // byte 0: total time for operation
    // byte 1: number of poles detected during operation
    // byte 2: number of tires deployed by machine during operation
    // the next 60 bits store details about each of (up to) 15 poles
    // taking four bytes per pole
    // for each pole:
        // byte 0: most significant bits of position variable (in millimeters)
        // byte 1: least significant bits of position variable
        // byte 2: number of tires supplied to pole
        // byte 3: number of tires present on pole
    
    write_EEPROM(currAdr,time);                         currAdr++;
    write_EEPROM(currAdr,numPoles);                     currAdr++;
    write_EEPROM(currAdr,tiresDep);                     currAdr++;
    while (1) {
        if ((i >= numPoles)&& (i < 15)) { break; }
        write_EEPROM(currAdr,((infoArr[i]).pos)>>8);    currAdr++;
        write_EEPROM(currAdr,((infoArr[i]).pos)&0xff);  currAdr++;
        write_EEPROM(currAdr,(infoArr[i]).tiresSupp);   currAdr++;
        write_EEPROM(currAdr,(infoArr[i]).tiresPresent);currAdr++;
        i++;
    }
}
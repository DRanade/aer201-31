#include "operationCode.h"          // includes function declarations

void runOp(unsigned char time[3], unsigned char *pTotalSupplied, 
        unsigned char *pNumPoles, poleInfo *infoArr, 
        volatile bool *pInt0P, unsigned char *opTime){
    
    INT0IE = 0;                     // disable the base interrupt during setup
    
    LATCbits.LATC5 = 1;             // shunt keypad (disable it) for operation
    
    TRISB = 0b10101011;             // set stepper motor pins to outputs
    
    *pTotalSupplied = 0;            // sets the number of supplied tires to 0
    
    *pNumPoles = 0;                 // sets number of poles found to 0
    
    unsigned char pos = 0;          // position tick counter (from 0-5)
    
    unsigned char posTurns = -5;    // position turn counter
                                    // counts number of times pos has hit 5.
    
                                    // it starts at -9 because, while the 
                                    // wheels start at the start line, the 
                                    // actual base sensor starts behind the 
                                    // start line by 9 units
    
    unsigned char procMode = 0;     // this variable stores the current mode
                                    // 0-Moving out     1-Base Detected
                                    // 2-Pole Detected  3-Tires Deployed
                                    // 4-Moving Back
    
    unsigned char distBaseDet = 0;  // after deploying tires, this measures how
                                    // far robot travelled. after it has
                                    // traveled half the diameter of base,
                                    // then it goes from mode 3 to mode 0
    
    dispProcMode(procMode);         // displays current progress on LCD
    
    INT0IE = 1;     *pInt0P = false;// enables interrupts and tells processor
                                    // that no interrupt has happened yet
    
    while(1){   // poll through main operation. Emergency stop still works
        if (procMode == 0 || procMode == 3){        // mode transition portion
                                    // this part changes processor mode if
                                    // interrupt has happened (base detect)
            
            if (*pInt0P){           // if base detected, move on to mode 1
                
                (*pInt0P) = false;  // indicate flag has been dealt with
                
                INT0IE = 0;         // re-enables interrupt
                
                procMode = 1;       // switches processor mode
                                                                                /*
                                                                                LATAbits.LATA5 = 0;
                                                                                LATDbits.LATD1 = 0;
                                                                                lcd_home();
                                                                                printf("%05u      %05u",posTurns,pos);
                                                                                __delay_ms(5000);
                                                                                */
                
                distBaseDet = 0;    // declares that the machine hasn't moved
                                    // forward since finding the base
                
                dispProcMode(procMode);     // show the new LCD mode info
            } 
        }
        if (procMode == 0){         // robot is moving out
            
            // tell coprocessor to move outwords
            LATAbits.LATA5 = 0; LATDbits.LATD1 = 1;
            
            LATCbits.LATC2 = 0;     // turn off LED indicator
            
            // start recording distance traveled
            unsigned char oldPosSig = PORTEbits.RE1;
            
            INT0IE = 1;             // enables the base detector interrupt
            
            while (*pInt0P == false){       // until base is detected
                
                if (oldPosSig != PORTEbits.RE1){    // if encoder has changed
                    
                    __delay_us(1000);       // debounce sensor in case the
                                            // reading was an error
                    
                    if (oldPosSig != PORTEbits.RE1){// if it was valid
                        
                        if (pos >= 5){      // if position is max
                            pos = 0;        
                            posTurns += 1;  // update posTurns
                                                                                /*
                                                                                if ((posTurns == 58) && (pos = 4)) {
                                                                                    LATAbits.LATA5 = 0;
                                                                                    LATDbits.LATD1 = 0;
                                                                                    __delay_ms(5000);
                                                                                    lcd_home();
                                                                                    printf("I have gone forward 1 meter");
                                                                                    return;
                                                                                }
                                                                                */
                                                                                
                            
                            if (posTurns == 90){   // this means robot
                                procMode = 4;       // has travelled 4m
                                
                                dispProcMode(procMode);     // show mode on LCD
                                
                                break;      // stop checking for base
                            }
                        }
                                                                                
                                                                                lcd_home();
                                                                                printf("%05u      %05u",posTurns,pos);
                                                                                
                        pos += 1;   // increment pos variable
                        
                        oldPosSig = oldPosSig ? 0 : 1 ; // if oldPosSig flipped,
                                                        // then update variable
                    }
                }
            }
        } else if (procMode == 1){      // base detected. nudging forward
                                        // while looking for pole

            LATAbits.LATA5 = 0; LATDbits.LATD1 = 0; // tell coprocessor 'stop'
            LATCbits.LATC2 = 1;         // turn on the LED since we found a base

            dispProcMode(procMode);     // show processor mode on LCD
            
            if (PORTCbits.RC0 == 0){    // if ramp not in, put it in
                LATAbits.LATA1 = 1; LATAbits.LATA3 = 0;     // brings arm in
            }
            while (PORTCbits.RC0 == 0){ // while arm coming in, do nothing
                continue;
            }
            LATAbits.LATA1 = 0;     LATAbits.LATA3 = 0;     // stop ramp motor
            
            __delay_ms(10);             // delay to allow ramp to come to stop
            
            LATAbits.LATA5 = 1; LATDbits.LATD1 = 1;         // tell coprocessor
                                                            // to nudge forward
            
            unsigned char oldPosSig = PORTEbits.RE1;        // record position
                                                            // through encoder
// see section in processor mode 0 for commented encoder distance measurement
            
            // origDetLoc is the position when we found the base. If we nudge 
            // 10 cm without finding a pole, just continue forward and
            // assume a ghost pole with one tire on top of it
            unsigned char origDetLoc = posTurns;
            
            while (1){                  // nudge until nudge sensor detects pole
                
                if (PORTBbits.RB5 == 0){// if nudge sensor detects pole
                    
                    __delay_ms(50);     // debounce it
                    
                    if (PORTBbits.RB5 == 0){    // if valid reading
                        
                        break;          // break out of nudging loop
                    }
                }
                
                if (oldPosSig != PORTEbits.RE1){    // see above in procMode 0
                    __delay_us(1000);               // section for comments here
                    if (oldPosSig != PORTEbits.RE1){
                        if (pos >= 5){
                            pos = 0;
                            posTurns += 1;
                            if (posTurns >= (origDetLoc + 3)) { break; }
                            if (posTurns == 90){
                                procMode = 4;
                                dispProcMode(procMode);
                                break;
                            }
                        }
                        pos += 1;
                        oldPosSig = oldPosSig ? 0 : 1 ;
                    }
                }
            }
            // end of nudging
            LATAbits.LATA5 = 0; LATDbits.LATD1 = 0; // tell coprocessor 'stop'
            
            if (PORTBbits.RB5 == 0){    // if nudge sensor is on (there is a 
                                        // pole detected), debounce signal
                __delay_ms(200);
                
                if (PORTBbits.RB5 == 0){// nudge forward until pole sensor reads
                                        // HIGH or nudge sensor reads LOW
                    
                    LATAbits.LATA5 = 1; LATDbits.LATD1 = 1; // tell coprocessor
                                                            // to nudge forward
                    
                    while (1){          // keep nudging during this while loop
                                if (PORTBbits.RB3 == 0){    // if pole sensor is
                                    __delay_ms(10);         // on, debounce and
                                    if (PORTBbits.RB3 == 0){// break out of loop
                                        break;
                                    }
                                }
                                if (PORTBbits.RB5 == 0){    // if nudge sensor
                                    __delay_ms(100);        // is on, debounce
                                    if (PORTBbits.RB5 == 0){// and  recheck pole
                                        continue;           // sensor
                                    }
                                }
                        
                        unsigned char oldPosSig = PORTEbits.RE1;// measure dist-
                        unsigned char distPoleDet = 0;  // ance while watching
                        
                        while (distPoleDet <= 1){   // keep nudging during the
                            if (PORTBbits.RB3 == 0){// distance encoded delay
                                __delay_ms(10);
                                if (PORTBbits.RB3 == 0){// but stop if pole sen-
                                    break;              // sor comes on
                                }
                            }
                            if (oldPosSig != PORTEbits.RE1){    // encoder stuff
                                __delay_us(1000);               // again
                                if (oldPosSig != PORTEbits.RE1){
                                    if (pos == 5){
                                        pos = 0;
                                        posTurns+=1;
                                    }
                                    pos += 1;
                                    distPoleDet += 1;
                                    oldPosSig = oldPosSig ? 0 : 1 ;
                                }
                            }
                        }
                        break;      // this is just so that breaking out of the
                                    // distance encoded delay loop also breaks
                                    // out of the nudging loop
                        
                        
                    }
                    LATAbits.LATA5 = 0; LATDbits.LATD1 = 0; // tell coprocessor
                                                            // to 'stop'
                }
            }
            LATAbits.LATA1 = 0; LATAbits.LATA3 = 1;         // send ramp out
            
            // below two variables exist to make sure ramp doesn't go too far
            // and fall off of the stand
            unsigned char temp = 0;
            unsigned int i = 0;
            unsigned char found = 0;    // found will be the number of cycles
                                        // of pushing the ramp out then nudging 
                                        // forward if pole not found.
                                        // if a pole is found, found is set
                                        // to 100 to break out of loop
            
            while (found < 4){  // <--this is the nudge/ramp cycle loop
                
                found += 1;             // increment iterative variable (we are
                                        // doing a new cycle)
                
                temp = 0;       // these variables keep track of how much the
                i = 0;          // ramp has travelled out
                
                LATAbits.LATA1 = 1; LATAbits.LATA3 = 0; // start moving ramp in
                                                        // in case it is out
                while (PORTCbits.RC0 == 0){     // wait until button is pressed
                    continue;                   // meaning the arm is fully in
                }
                LATAbits.LATA5 = 1; LATDbits.LATD1 = 1; // tell coprocessor
                                                        // to start nudging
                
                __delay_ms(400);    // nudge two times per cycle. this delay 
                                    // will allow for two nudges
                
                posTurns++;         // increment distance travelled during nudge
                
                LATAbits.LATA5 = 0; LATDbits.LATD1 = 0; // tell coprocessor
                                                        // to stop
                
                LATAbits.LATA1 = 0; LATAbits.LATA3 = 1; // after nudge, try to
                                                        // push ramp out while
                                                        // looking for pole
                while (1){      // ramp pushes out during this while loop
                    
                    if (PORTBbits.RB3 == 0){    // if pole sensor comes on,
                        __delay_ms(50);         // debounce
                        if (PORTBbits.RB3 == 0){
                            found = 100;        // indicates pole found and
                            break;              // breaks so ramp can stop
                        }
                    }
                    temp++;                     // counts ramp distance traveled
                    if (temp >= 0xff){
                        temp = 0;
                        i++;
                        if ((i >= 0x360) && (temp >= 0x00)){
                            break;      // if no pole found after this distance,
                                        // stop moving arm out
                        }
                    }
                }
            }
            LATAbits.LATA1 = 0; LATAbits.LATA3 = 0;             // stops arm
            
            infoArr[*pNumPoles].id = *pNumPoles + 1;    // updates data storage
                                                        // pole ID and
            infoArr[*pNumPoles].pos = posTurns*44 + pos*9;     // pole position
            
            if (found < 80){    // found being less than 80 means we completed
                                // 4 cycles and have not found pole. we will
                                // assume a pole exists but was too far away
                
                LATAbits.LATA1 = 1; LATAbits.LATA3 = 0; // brings ramp in if
                                                        // it is out
                
                while (PORTCbits.RC0 == 0){ // wait as long as ramp is not in
                    continue;
                }
                LATAbits.LATA1 = 0; LATAbits.LATA3 = 0; // stop the ramp now
                
                infoArr[*pNumPoles].tiresPresent = 1;   // assume ghost pole
                                                        // with one tire on it
                infoArr[*pNumPoles].tiresSupp = 0;  // we didn't deploy any tire
                *pNumPoles += 1;                    // increment number of poles
                procMode = 3;           // change to processor mode 3 (so no
                                        // tire deployment)
                
                continue;               // continue in main loop
            }
            
            // if this part runs, we know that a pole has been found
            while(1){
                if (PORTBbits.RB3 == 0){        // if a pole was found, debounce
                    __delay_ms(10);
                    if (PORTBbits.RB3 == 0){    // if valid reading
                        
                        procMode = 2;           // changes into tire deploy mode
                        
                        dispProcMode(procMode); // LCD update
                        break;
                    }
                }
            }
        } else if (procMode == 2){      // tire deployment processor mode
            LATAbits.LATA5 = 0; LATDbits.LATD1 = 0; // tell coprocessor 'stop'
            LATCbits.LATC2 = 1;         // turn on LED indicator
            unsigned char tireVar = 0;  // at this point, tireVar will the
                                        // number of tires detected by sensors
            
            if (PORTCbits.RC6 == 0){    // if top sensor detects HIGH (after
                __delay_ms(500);        // debounce), set tireVar to two tires
                if (PORTCbits.RC6 == 0){
                    tireVar = 2;
                }
            } else if (PORTBbits.RB1 == 1){     // otherwise, if bottom sensor
                __delay_ms(500);                // reads HIGH (after debounce),
                if (PORTBbits.RB1 == 1){        // set tireVar to one tire
                    tireVar = 1;
                }
            }
            
            // we will now change tireVar to be the number of tires required 
            // to deploy by the robot (based on the RFP))
                    
            if (*pNumPoles == 0){       // if there have been no poles before,
                tireVar = 2-tireVar;    // we will fill the pole to 2 tires
                
                infoArr[*pNumPoles].tiresPresent = 2;   // when the robot is
                                        // done, the pole will have 2 tires
                
                // otherwise, check to see if the most recent pole was within
                // the past 30 cm
                // note: each tick of posTurns is 75 mm in actual distance
                // so we calculate (posTurns - oldPosition/15)
            } else if ((posTurns - (infoArr[*pNumPoles-1].pos)/44) <= 7){
                tireVar = 1-tireVar;    // if <30cm, fill to 1 tire
                
                infoArr[*pNumPoles].tiresPresent = 1;   // when robot is done,
                                                        // pole will have 1 tire
            } else {
                tireVar = 2-tireVar;    // if distance is greater than this,
                                        // then the robot will fill to 2 tires
                
                infoArr[*pNumPoles].tiresPresent = 2;   // when robot is done,
                                                        // pole will have 2 tire
            }
            
            // tireVar is now the number of tires robot needs to deploy
            
            tireDeploy(tireVar,(*pTotalSupplied)%2);    // this function does
                                                        // the actual deployment
            
            infoArr[*pNumPoles].tiresSupp = tireVar;    // updates the number of
                                                        // tires supplied
            
            *pTotalSupplied += tireVar; // update total number of tires supplied
            
            dispProcMode(procMode);     // show LCD status

            if (tireVar != 0){          // if any tires have been deployed,
                __delay_ms(3000);       // wait 3 seconds for tires to arrive
            }

            *pNumPoles += 1;    // finally increments number of detected poles
            
            procMode = 3;       // change to 'moving forward + ramp in' mode
            dispProcMode(procMode);     // show this progress on LCD
            
        } else if (procMode == 3){  // move forward after retracting ramp
            
                                                                                /*if (*pNumPoles >= 6){
                                                                                    procMode = 4;
                                                                                    continue;
                                                                                }*/
            
            unsigned char distSinceBase = 0;    // base interrupt will be
                                                // disabled until the robot
                                                // travels 7 cm since last base.
                                                // this will ensure that the
                                                // same base isn't accidentally
                                                // again
            
            LATCbits.LATC2 = 0;     // turn LED off again since robot is moving
            
            LATAbits.LATA1 = 1; LATAbits.LATA3 = 0; // bring ramp in first thing
            
            while(PORTCbits.RC0 == 0){          // wait until ramp is in
                continue;
            }
            
            LATAbits.LATA1 = 0; LATAbits.LATA3 = 0; // turn off ramp motor
            
            LATAbits.LATA5 = 0; LATDbits.LATD1 = 1; // tell coprocessor to move
                                                    // 'outwards'
            
            unsigned char oldPosSig = PORTEbits.RE1;// we will record position
                                                    // since we're moving out
            
            INT0IE = 0;             // disable base interrupt until clear of
                                    // current base
            
            while (1){              // during this loop, robot drives forward
                                    // until it clears the current base
                
                if (oldPosSig != PORTEbits.RE1){    // same encoder code as
                    __delay_us(1000);               // before to record distance
                    if (oldPosSig != PORTEbits.RE1){
                        if (pos == 5){
                            pos = 0;
                            posTurns += 1; 
                            distSinceBase += 1;     // also increment distance
                                                    // traveled since base
                            
                            if (distSinceBase == 2){// if traveled far enough

                                INT0IE = 1;         // re-enable base interrupt
                                INT1IF = 0;         // clear interrupt flag(s)
                                *pInt0P = false;    // just in case
                                
                                // tell coprocessor to stop
                                LATAbits.LATA5 = 0; LATDbits.LATD1 = 0;
                                
                                procMode = 0;       // change processor mode
                                                    // to 'moving out'
                                
                                dispProcMode(procMode); // update LCD progress
                                
                                break;              // exit this loop so we
                                                    // can go back to procMode 0
                            }
                        }
                        pos += 1;               // regular distance encoder code
                        oldPosSig = oldPosSig ? 0 : 1 ;
                    }
                }
            }
        } else if (procMode == 4){                  // coming back to start
            
            LATAbits.LATA5 = 1; LATDbits.LATD1 = 0; // tell coprocessor to come
                                                    // back to start
            
            LATCbits.LATC2 = 0;                     // LED indicator off again
                                                    // since robot is moving
            
            posTurns += 55;
                                // inconsistent driving backwards
                                // this was calibrated during testing
            
            unsigned char oldPosSig = PORTEbits.RE1;// standard encoder code
                                                    // for driving backwards
            while (posTurns > 0){
                if (oldPosSig != PORTEbits.RE1){
                    __delay_us(1000);
                    if (oldPosSig != PORTEbits.RE1){
                        if (pos <= 00){
                            posTurns-=1;
                            pos = 5;
                        }
                        pos -= 1;
                        oldPosSig = oldPosSig ? 0 : 1 ;
                    }
                }
            }
            LATAbits.LATA5 = 0; LATDbits.LATD1 = 0; // stop robot since it is
                                                    // now back to start line
            
            unsigned char endTime[3];               // record end time
            
            getTime(endTime);                       // read time from RTC
            
            fixTime(endTime);                       // change the RTC
                                                    // representation to 
                                                    // normal representation
            
            if (endTime[2] > time[2]){              // checks to see if the hour
                                                    // variable has changed.
                                                    // if so, the regular
                                                    // calculation would be too
                                                    // big for the allocated
                                                    // variable
                
                endTime[2] -= 1;                    // this is the fix for it
                time[2] += 60;
            }
            // below writes the time taken for the operation
            *opTime = 60*endTime[1]+endTime[0] - 60*time[1] - time[0];
            
            // ends the operation function
            return;
        }
    }
}

/*
// i just copied the data storage struct definition for ease of access
struct poleInfo {
    unsigned char id;
    int pos;
    unsigned char tiresSupp;
    unsigned char tiresPresent;
};
typedef struct poleInfo poleInfo; */
/* 
 * File:   eeprom.c
 * Author: Devansh Ranade
 *
 * Created on March 9, 2019, 12:58 AM
 */
#include "configBits.h"
#include "eeprom_h.h"


void write_EEPROM(unsigned char address, unsigned char data){
    while( EECON1bits.WR  ){continue;} //checking if not busy with an earlier write.

    EECON1bits.WREN=1; // Enable writing to EEPROM 
    EEADR=address; // load address 
    EEDATA=data; // load data
    EECON1bits.EEPGD=0; // access EEPROM memory
    EECON1bits.CFGS=0; // avoid access configuration registers
    INTCONbits.GIE=0; // disable interrupts for critical EEPROM write sequence
    // required sequence start
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    // required sequence end
    INTCONbits.GIE = 1; // enable interrupts, critical sequence complete
    while (EECON1bits.WR==1); // wait for write to complete
    EECON1bits.WREN=0;  // do not allow EEPROM writes
}

unsigned char read_EEPROM(unsigned char address){
    while( EECON1bits.WR  ){continue;} //checking if busy
    
    EEADR = address; // load address 
    EECON1bits.EEPGD = 0; // access EEPROM memory
    EECON1bits.CFGS  = 0; // avoid access configuration registers
    EECON1bits.RD    = 1; // read 
    return( EEDATA );
}

void shift_EEPROM(void){
    short int i;
    short int val = 0;
    
    for (i = 204; i >= 0; i--){ // shifting all values by 41 addresses in EEPROM 
        val = read_EEPROM(i);
        write_EEPROM(i+41, val);
    }
}

void storeLog(unsigned char time, unsigned char numPoles, unsigned char tiresDep, poleInfo* infoArr, unsigned char* pLastLog){
    unsigned char currAdr = 0;
    unsigned char i=0;
    unsigned char currLog = (*pLastLog+1)%4;
    *pLastLog = currLog;
    currAdr = currLog*64;
    write_EEPROM(currAdr,time);                         currAdr++;
    write_EEPROM(currAdr,numPoles);                     currAdr++;
    write_EEPROM(currAdr,tiresDep);                     currAdr++;
    while (1) {
        if ((i >= numPoles)&& (i < 15)) { break; }
        write_EEPROM(currAdr,((infoArr[i]).pos)>>8)     currAdr++;
        write_EEPROM(currAdr,((infoArr[i]).pos)&0xff)   currAdr++;
        write_EEPROM(currAdr,(infoArr[i]).tiresSupp)    currAdr++;
        write_EEPROM(currAdr,(infoArr[i]).tiresPresent) currAdr++;
        i++;
    }
}

/*
struct poleInfo {
    unsigned char id;
    int pos;
    unsigned char tiresSupp;
    unsigned char tiresPresent;
};
*/
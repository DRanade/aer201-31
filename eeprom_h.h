/* 
 * File:   eeprom_h.h
 * Author: Devansh Ranade
 *
 * Created on March 9, 2019, 1:00 AM
 */

#ifndef EEPROM_H_H
#define	EEPROM_H_H

/********************************** Includes **********************************/
#include <xc.h>
#include <stdio.h>
#include "helpers.h"

/****************************** EEPROM Functions *******************************/
void write_EEPROM(unsigned char address, unsigned char data);
unsigned char read_EEPROM(unsigned char address);
void shift_EEPROM(void);

void storeLog(unsigned char time, unsigned char numPoles, unsigned char tiresDep, poleInfo* infoArr, unsigned char* pLastLog);

#endif	/* EEPROM_H_H */


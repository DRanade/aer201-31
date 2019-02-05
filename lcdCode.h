/* 
 * File:   standby.h
 * Author: Devansh Ranade
 *
 * Created on January 29, 2019, 2:44 PM
 */

#include "I2C.h"
#include "lcd.h"
#include "stdio.h"
void showTime(unsigned char pTime[7]){
    // Reset RTC memory pointer
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010000); // 7 bit RTC address + Write
    I2C_Master_Write(0x00); // Set memory pointer to seconds
    I2C_Master_Stop(); // Stop condition

    // Read current time
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010001); // 7 bit RTC address + Read
    for(unsigned char i = 0; i < 6; i++){
        pTime[i] = I2C_Master_Read(ACK); // Read with ACK to continue reading
    }
    pTime[6] = I2C_Master_Read(NACK); // Final Read with NACK
    I2C_Master_Stop(); // Stop condition

    // Print received data on LCD
    lcd_home();
    printf("TEAM 31     TIRE");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("    %02x/%02x/%02x    ", pTime[6],pTime[5],pTime[4]); // Print date in YY/MM/DD
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("    %02x:%02x:%02x    ", pTime[2],pTime[1],pTime[0]); // HH:MM:SS
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("----0:START-----");
};

void scrDoneMain(void){
    lcd_home();
    printf("1: GENERAL INFO ");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("2: POLE DETAILS ");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("                ");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("-----0:Done-----");
}

void scrDoneGen(unsigned int time, unsigned char tireCount, 
    unsigned char poles){
    lcd_home();
    printf("TOTAL TIME: %03xs",time);
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("TIRES SUPP:  %02x ",tireCount);
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("POLE COUNT:  %02x ",poles);
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("-----0:MENU-----");
}

struct poleInfo {
    unsigned char id;
    int pos;
    unsigned char tiresSupp;
    unsigned char tiresPresent;
};
typedef struct poleInfo poleInfo;

void scrDonePoleDet(poleInfo poleData[15],int currPole){
    lcd_home();
    printf("POLE%01x POS: %01d.%02dm",
    poleData[currPole].id,poleData[currPole].pos/1000,(poleData[currPole].pos % 1000)/10);
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("TIRES SUPP:   %01d ",poleData[currPole].tiresSupp);
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("TIRES ON POLE:%01d ",poleData[currPole].tiresPresent);
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("<*   0:MENU   #>");
};

// no modulo: poleData[currPole].pos-poleData[currPole].pos/1000
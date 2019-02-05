#include "I2C.h"
#include "lcd.h"
#include <stdio.h>
void showTime(unsigned char pTime[7]);
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
    printf("Team 31     TIRE");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("    %02x/%02x/%02x    ", pTime[6],pTime[5],pTime[4]); // Print date in YY/MM/DD
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("    %02x:%02x:%02x    ", pTime[2],pTime[1],pTime[0]); // HH:MM:SS
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("----0:START-----");
    __delay_ms(100);
}

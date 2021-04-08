#include "lcdCode.h"

void showTime(unsigned char pTime[7]){
    // first bit gets time from RTC. See getTime() function in helpers.c
    I2C_Master_Start();
    I2C_Master_Write(0b11010000);
    I2C_Master_Write(0x00);
    I2C_Master_Stop();

    // read current time
    I2C_Master_Start();
    I2C_Master_Write(0b11010001);
    for(unsigned char i = 0; i < 6; i++){
        pTime[i] = I2C_Master_Read(ACK);
    }
    pTime[6] = I2C_Master_Read(NACK);
    I2C_Master_Stop();

    // Print received data on LCD
    lcd_home();
    printf("TEAM 31     TIRE");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    // Print date in YY/MM/DD
    printf("    %02x/%02x/%02x    ", pTime[6],pTime[5],pTime[4]);
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    // Print time in // HH:MM:SS
    printf("    %02x:%02x:%02x    ", pTime[2],pTime[1],pTime[0]);
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("0:START   8:LOGS");
}

void scrDoneMain(void){     // show 'run complete' screen
    lcd_home();
    printf("--RUN COMPLETE--");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("7: GENERAL INFO ");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("9: POLE DETAILS ");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("-----0:DONE-----");
}

void scrDoneGen(unsigned int time, unsigned char tireCount, 
        unsigned char poles){
    // show 'general details' screen
    lcd_home();
    printf("TOTAL TIME: %3ds",time);
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("TIRES SUPP:  %2d ",tireCount);
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("POLE COUNT:  %2d ",poles);
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("-----0:MENU-----");
}

void scrDonePoleDet(poleInfo poleData[15],int currPole){
    // show 'pole details' screen
    lcd_home();
    printf("POLE%1x POS: %1d.%02dm",
    poleData[currPole].id,poleData[currPole].pos/1000,
            (poleData[currPole].pos % 1000)/10);
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("TIRES SUPP:   %1d ",poleData[currPole].tiresSupp);
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("TIRES ON POLE:%1d ",poleData[currPole].tiresPresent);
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("<*   0:MENU   #>");
}

void dispOpProg(void){      // show 'operation in progress' screen
    unsigned int i;
    lcd_home();
    printf("  THE OPERATION ");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("  IS CURRENTLY  ");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("   IN PROGRESS  ");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("                ");
}

void dispProcMode(unsigned char mode){      // shows current progress through
                                    // operation depending on processor mode
    lcd_home();
    printf("                ");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("                ");
    if (mode == 0){                 // see the print statements for what
        lcd_set_ddram_addr(LCD_LINE3_ADDR);     // each processor mode means
        printf("                ");
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf(" I AM GOING OUT ");
    } else if (mode == 1){
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf("   BASE FOUND   ");
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("LOOKING FOR POLE");
    } else if (mode == 2){
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf("   POLE FOUND   ");
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("DEPLOYING  TIRES");
    } else if (mode == 3){
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf(" TIRES  ARRIVED ");
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("MOVING + RAMP IN");
    } else {
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf("  MAX DISTANCE  ");
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("I AM COMING HOME");
    }
}

void detectFeedback(void){          // screen to flash when base is found
    lcd_home();
    printf("!!!!! POLE !!!!!");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("! SUCCESSFULLY !");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("!   DETECTED   !");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("!!!!!!!!!!!!!!!!");
}

void tooManyTires(void){            // screen to show when illegal number of
                                    // tires detected
    lcd_home();
    printf("!!!!!ERROR!!!!!!");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("!  THIS  POLE  !");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("! HAS TOO MANY !");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("! TIRES ON IT  !");
}

void lcdNorm(void){                 // normal screen during operation
    lcd_home();
    printf("  THE OPERATION ");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("  IS CURRENTLY  ");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("   IN PROGRESS  ");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("                ");
}

void dispShowLogs(void){            // log select screen
    lcd_home();
    printf("CHOOSE WHICH LOG");
    lcd_set_ddram_addr(LCD_LINE2_ADDR);
    printf("TO SEE ( 1 - 4 )");
    lcd_set_ddram_addr(LCD_LINE3_ADDR);
    printf("1:NEW      4:OLD");
    lcd_set_ddram_addr(LCD_LINE4_ADDR);
    printf("-----0:DONE-----");
}

void dispLogMain(unsigned char log){// log main screen
    
    if (read_EEPROM(log*64+1) > 128){       // if no data stored
        lcd_home();
        printf("      ERROR      ");
        lcd_set_ddram_addr(LCD_LINE2_ADDR);
        printf(" NO  INFORMATION ");
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf("      STORED     ",read_EEPROM(log*64+1));
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("-----0:MENU-----");
    } else {                                // if data stored
        lcd_home();
        printf("--SHOWING  LOG--");
        lcd_set_ddram_addr(LCD_LINE2_ADDR);
        printf("7: GENERAL INFO ");
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf("9: POLE DETAILS ");
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("-----0:DONE-----");
    }
}

void dispLogGen(unsigned char log){ // log general details screen
    if (read_EEPROM(log*64+1) > 128){           // if no data stored
        lcd_home();
        printf("      ERROR      ");
        lcd_set_ddram_addr(LCD_LINE2_ADDR);
        printf(" NO  INFORMATION ");
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf("      STORED     ",read_EEPROM(log*64+1));
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("-----0:MENU-----");
    } else {                                    // if data stored
        lcd_home();
        printf("TOTAL TIME: %3ds", read_EEPROM(log*64));
        lcd_set_ddram_addr(LCD_LINE2_ADDR);
        printf("TIRES SUPP:  %2d ",read_EEPROM(log*64+2));
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf("POLE COUNT:  %2d ",read_EEPROM(log*64+1));
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("-----0:MENU-----");
    }
}

void dispLogPoleDet(unsigned char log, unsigned char currPole){
                                    // log pole details screen
    if (read_EEPROM(log*64+1) > 128){           // if no data stored
        lcd_home();
        printf("      ERROR      ");
        lcd_set_ddram_addr(LCD_LINE2_ADDR);
        printf(" NO  INFORMATION ");
        lcd_set_ddram_addr(LCD_LINE3_ADDR);
        printf("      STORED     ",read_EEPROM(log*64+1));
        lcd_set_ddram_addr(LCD_LINE4_ADDR);
        printf("-----0:MENU-----");
    } else {
        if (read_EEPROM(log*64+1) == 0){        // if no poles detected in run
            lcd_home();
            printf("  THIS RUN HAD  ");
            lcd_set_ddram_addr(LCD_LINE2_ADDR);
            printf("    NO POLES    ");
            lcd_set_ddram_addr(LCD_LINE3_ADDR);
            printf("    DETECTED    ");
            lcd_set_ddram_addr(LCD_LINE4_ADDR);
            printf("-----0:MENU-----");
        } else {                                // if data valid
            int pos = ((int)read_EEPROM(log*64+3+4*currPole))<<8 | 
                    (int)read_EEPROM(log*64+4+4*currPole);
            lcd_home();
            printf("POLE%1x POS: %1d.%02dm",
            currPole+1,(pos)/1000, (pos % 1000)/10);
            lcd_set_ddram_addr(LCD_LINE2_ADDR);
            printf("TIRES SUPP:   %1d ",read_EEPROM(log*64+5+4*currPole));
            lcd_set_ddram_addr(LCD_LINE3_ADDR);
            printf("TIRES ON POLE:%1d ",read_EEPROM(log*64+6+4*currPole));
            lcd_set_ddram_addr(LCD_LINE4_ADDR);
            printf("<*   0:MENU   #>");
        }
    }
}
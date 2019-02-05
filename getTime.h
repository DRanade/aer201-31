/* 
 * File:   getTime.h
 * Author: Devansh Ranade
 *
 * Created on January 29, 2019, 3:50 PM
 */
void getTime(unsigned char pTime[3]){
    // Reset RTC memory pointer
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010000); // 7 bit RTC address + Write
    I2C_Master_Write(0x00); // Set memory pointer to seconds
    I2C_Master_Stop(); // Stop condition

    // Read current time
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010001); // 7 bit RTC address + Read
    for(unsigned char i = 0; i < 2; i++){
        pTime[i] = I2C_Master_Read(ACK); // Read with ACK to continue reading
    }
    pTime[2] = I2C_Master_Read(NACK); // Final Read with NACK
    I2C_Master_Stop(); // Stop condition
};
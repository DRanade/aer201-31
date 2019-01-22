/*
 * File:   main.c
 * Author: Devansh Ranade
 * Created on January 22, 2019, 1328h
 */

#include "config.c"

void main(void) {
    /*
    // Set TRIS to be output    TRISA = 0x0;
    // Set BITS to zero         LATA = 0x0;
    // TRISBbits.RB1 = 1;
    // INTCON = 0b10000000;
    // INTCON3 = 0b1000;
    */
    
// START OF CODE
// WAITING FOR START:
    //Poll until button press
// Operation start
    // we are leaving now
    // enable interrupt to check for base INT_BASE
    //while(xPos < 4000){
        // move forward;
    //}
    // disable interrupts. Coming back
    //while (xPos > 0){
        // move backwards;
    //}
// MACHINE OPERATION FINISHED
    // Display / IO and Memory storage
}

void __interrupt() intHandle(void) {
    int test = 1;
    if (test == 1){test = 0;}
    //if (intPin1 == 1){
    //    if (poleDetected == 0){
            // move last little bit as necessary
            // start moving the motor out
            // change type of interrupt
            // detect number of tires
    //    }
    //}
    
    
    
    // check if interrupt is base detected
    //
    //bit poleDetected = 1;
    // reset flag and change this interrupt to pole detection
    //
    //
    // Enable pole detection interrupt
    // start moving arm out
    // count tires
    //if (INTCON3 & 0b10 == 0b10){ //alternatively, check the condition INT1IE && INT1IF
    //    INT1IF = 0;
    //    LATCbits.LATC0 = ~LATCbits.LATC0;
    //}
    
    // check if interrupt is pole detection
}
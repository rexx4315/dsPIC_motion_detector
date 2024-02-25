/**********************************************************************
*
* Author: Brady Mayfield
*
* FileName:        main.c
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       dsPIC33FJ256GP506
* Compiler:        MPLAB® C30 v3.00 or higher
*
* Description: this is the main.c file for an motion detector developed
*              using the dsPIC DSC Starter Kit. It contains the state
*              machine logic for cycling between the different states of
*              the system and for making decisions.
*
************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "p33FJ256GP506.h"
#include "WM8510CODEC.h"
#include "timer.h"
#include "dscInit.h"
#include "dma.h"

#define RX_SIG_SIZE 1200
#define THRESHOLD_VAL 20

// State machine custom type
enum STATE {S_INIT, S_ECHO, S_PROC, S_IDLE};
enum STATE c_state = S_INIT; // Current state
enum STATE n_state = S_INIT; // Next state

// Declare the vector funcions as external.
extern int vectorMAC(int *x, int *y, unsigned int loop_count,unsigned int scale);
extern int squareMAC_Y(int *x, unsigned int loop_count,unsigned int scale);
extern int squareMAC_X(int *x, unsigned int loop_count,unsigned int scale);
extern int vectorShift(int *vector, int vectorLength);

// External signals - DMA
extern unsigned int rxFlag;
extern signed int txSignal[];
extern signed int rxSignal[];

// Processing signals
signed short int sigma;
signed short int sigmaPrev;

// The signal vector is placed in the x memory space.
//The coefficents are placed in the y memory space.
//This allows single cycle MAC operations.
signed int rxSignal_xmem[RX_SIG_SIZE] __attribute__((space(xmemory), aligned,));
signed int rxSignal_ymem[RX_SIG_SIZE]__attribute__ ((space(ymemory), aligned));
signed int rx_switch = 0;
unsigned int rx_count = 0;

// Timer signals
unsigned short int timerCount;
extern unsigned int timerFlag;

extern unsigned int dataReady;



signed short int refEnergy;
signed short int sigEnergy;
signed short int nrmEnergy;

signed long int runningSumCor;
signed long int runningSumEnergy;



int main(void)
{
	short unsigned int n; // Working variable.
	short signed int temp;

    // Always start in the init state
    c_state = S_INIT;

	// Main processing loop.
	// Quit if switch 1 is pressed.
	while(SWITCH_S1) {

        // State machine logic
        switch (c_state) {

            // In the init state, we run through all the necessary intialization
            // functions before transitioning to S_ECHO
            case (S_INIT):

                // Intialize the board and the drivers
                DSCInit();

                // Initalize the CODEC.
                WM8510Init();

                // Start Audio input and output function
                WM8510Start();

                // Configure the CODEC for 8K operation
                WM8510SampleRate8KConfig();

                // Init the timer interrupt
                // Use timer 1 as a real time clock.
                // 1 tick = 15,625 * 256 clock cycles = 100ms.
                timer1Init(15625);

                // Init / enable the DMA
                dmaInit();
                
                // Generate TX waveform
                txSigGen(1000);

                // Set next state, break
                n_state = S_ECHO;
                break;

            // In the echo state, we do not have to do anything - everything is handled
            // by the DMA. We wait for the RX flag to go high, pause the DMA, then transition
            // to process
            case (S_ECHO):

                if (rxFlag == 1) {

                    // Clear RX flag
                    rxFlag = 0;

                    // Where we place the data depends on rx_switch
                    // If rx_switch is 0, place in X
                    // If rx_switch is 1, place in Y
                    if (rx_switch == 0) {
                        rxSignal_xmem[rx_count] = rxSignal[0];
                    }
                    else {
                        rxSignal_ymem[rx_count] = rxSignal[0];
                    }

                    // Check rx count
                    if (rx_count < RX_SIG_SIZE) {
                        rx_count++;
                    }
                    else {

                        // Done, reset count
                        rx_count = 0;

                        // Disable DMA
                        dmaDisable();

                        // Done with echo, process
                        n_state = S_PROC;

                    }

                }

                break;

            // Process
            case (S_PROC):

                // Turn the yellow LED on to indicate when we are processing / waiting
                YELLOW_LED = SASK_LED_ON;

                // Calculate dot product between two most recent received signals
                sigma = vectorMAC(&(rxSignal_xmem[0]), &(rxSignal_ymem[0]), RX_SIG_SIZE, 4);

                // Determine if the dot product difference is above our threshold
                if (abs(sigma - sigmaPrev) > (unsigned short int)THRESHOLD_VAL) {
                    GREEN_LED = SASK_LED_ON;
                }
                else {
                    GREEN_LED = SASK_LED_OFF;
                }

                // Save this result for comparison next time
                sigmaPrev = sigma;

                // Reset time flag, set next state, break
                timerFlag = 0;
                n_state = S_IDLE;
                break;

            // Wait ~1s
            case (S_IDLE):

                if (timerFlag == 1) {

                    // Pull low
                    timerFlag = 0;

                    // Determine if we are done waiting or now
                    if (timerCount < 10) {
                        timerCount += 1;
                    }

                    // Done
                    else {

                        timerCount = 0;

                        // Pull yellow LED low (indicates we are done waiting / processing)
                        YELLOW_LED = SASK_LED_OFF;

                        // Re-enable DMA
                        dmaEnable();

                        // Set next state
                        n_state = S_ECHO;

                        // Toggle rx_switch
                        if (rx_switch == 0) rx_switch = 1;
                        else rx_switch = 0;

                    }

                }

                break;

        }

        // Update current state
        c_state = n_state;

    }

return(0); // Go home.
}
#include <xc.h>
#include <math.h>
#include "dma.h"

// Declare the flags.
unsigned int rxFlag = 0;
unsigned int txFlag = 0;

// Transmit and receive buffers.
// The attribute forces the buffers to be defined in the DMA space.
signed int rxSignal[RX_FRAME_SIZE] __attribute__((space(dma)));
signed int txSignal[TX_FRAME_SIZE] __attribute__((space(dma)));

// Pointers to the buffers.
signed int *pSignalRx;
signed int *pSignalTx;

unsigned int n;

// Interrupt on frame transmitted. This is not used in this example code,
// but is here as a place holder if needed.
void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void)
{
	txFlag = 1;
    IFS0bits.DMA0IF = 0;		//Clear the DMA0 Interrupt Flag
}

// Interrupt on frame received. This is not used in this example code,
// but is here as a place holder if needed.
void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void)
{
	rxFlag = 1;
    IFS0bits.DMA1IF = 0;		//Clear the DMA1 Interrupt Flag
}

void dmaInit()
{
	unsigned int n;

	// Clear the buffer.
	for(n=0; n < TX_FRAME_SIZE; n++)
		{
		txSignal[n] = 0;
		}

    for(n=0; n < RX_FRAME_SIZE; n++)
		{
		rxSignal[n] = 0;
		}

	// Get the address of the signal to send.
	// Notice that this is the DMA memory space!
	DMA0STA = __builtin_dmaoffset(txSignal);
	DMA1STA = __builtin_dmaoffset(rxSignal);

	// DCI TXBUF0 address (0x0298)
	DMA0PAD = (volatile unsigned int) &TXBUF0;
	// DCI RXBUF0 address (0x0290)
	DMA1PAD = (volatile unsigned int) &RXBUF0;
	// Number of words to transfer.
	DMA0CNT = TX_FRAME_SIZE - 1;
	DMA1CNT = RX_FRAME_SIZE - 1;
	// DMA Interrupt Request on DCI Transfer Done.
	DMA0REQ = 0x003C;
	DMA1REQ = 0x003C;

	//DMA channel 0 direction is write.
	DMA0CONbits.DIR = 1;
	// Continous transfer, no ping pong.
	DMA0CONbits.MODE = 0;
	// Addresing mode is set to post-increment.
	DMA0CONbits.AMODE = 0;

	//DMA channel 1 direction is read.
	DMA1CONbits.DIR = 0;
	// Continous transfer, no ping pong.
	DMA1CONbits.MODE = 0;
	// Addresing mode is set to post-increment.
	DMA1CONbits.AMODE = 0;

	IFS0bits.DMA0IF  = 0;			// Clear DMA 0 interrupt
	IEC0bits.DMA0IE  = 1;			// Enable DMA 0 interrupt
	DMA0CONbits.CHEN = 1;			// Enable DMA 0 Channel

	IFS0bits.DMA1IF  = 0;			// Clear DMA 1 interrupt
	IEC0bits.DMA1IE  = 1;			// Enable DMA 1 interrupt
	DMA1CONbits.CHEN = 1;			// Enable DMA 1 Channel
}

void dmaDisable()
{
    // Disable DMA
    DMA0CONbits.CHEN = 0;
    DMA1CONbits.CHEN = 0;
}

void dmaEnable()
{
    // Enable DMA
    DMA0CONbits.CHEN = 1;
    DMA1CONbits.CHEN = 1;
}

void txSigGen(int freq_hz)
{

    // To generate the signal, we need to know a few things:
    // Length of our vector, the desired frequency, and the sample rate
    // number of cycles in vector = (sample period) * (vector length) / (signal period)

    // Definitions for building the coefficents.
    float sig_period = (float)1 / (float)freq_hz;
    float sample_period = (float)1 / (float)8000;
    float numCycles = (float)TX_FRAME_SIZE * sample_period / sig_period;
	float T = (float)1/(float)TX_FRAME_SIZE;
	float PI =  3.14159265;
    int n = 0;

	// Generate the desired waveform.
	for (n =0; n < TX_FRAME_SIZE; n++)
	{
		txSignal[n] = sin(2 * PI * numCycles *(float)n * T) * 0x7FFF;
	}

}

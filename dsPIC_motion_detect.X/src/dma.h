// FRAME_SIZE - Size of each audio frame

// TX_FRAME_SIZE
// 1kHz period = 1ms
// 8kHz period = 1/8ms
// To represent two periods of 1kHz signal, we would need
// a TX Frame Size of 16
#define TX_FRAME_SIZE 				16

// RX_FRAME_SIZE
#define RX_FRAME_SIZE               1

void dmaInit();
void dmaDisable();
void dmaEnable();
void txSigGen();

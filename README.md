# dsPIC_motion_detector
This project utilizes the dsPIC board to implement a simple motion detection system.

## Code Overview
The project utilizes a simple state machine with 4 states:
  1. Init
  2. Echo
  3. Process
  4. Idle

The init state is only executed once after powerup / reset, and sets up the DMA, the CODEC,
the timer interrupt and anything else that needs to be initialized.

The echo state captures the receive signals one by one to create a receive vector that has
1200 total samples. Once 1200 samples are collected, the DMA is disabled and the code goes
to the Process state. The process state calculates the dot product of the two most recent
signals. If the difference between the most recent dot product result and the previous
dot product result is greater than the specified threshold, the green LED is asserted.

## Demo
The idle state just sits for approximately one second, then returns to the echo states where
the entire process is repeated.

In the demo video, you can see me stepping in front of the device and the green LED turns on
briefly. Then I step away, and the green LED turns on briefly again.

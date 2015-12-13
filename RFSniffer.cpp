/*
    RFSniffer.cpp
    https://github.com/n8henrie/rf_pi
 
    Copied with minor (if any) modifications by @n8henrie from:
 
    - https://github.com/ninjablocks/433Utils/blob/master/RPi_utils/RFSniffer.cpp
 
    Attach an RF receiver to your Pi and set `PIN` below.
 
    ## Usage:
        - `make`
        - `./RFSniffer`
        - Click the buttons on your RF remote
        - Output should look something like:
```
Decimal: 7508342
Bit length: 24
Pulse length: 192
Binary: 100l01000111110110110001
```
        - Write it down, profit

    Please also reference instructions in the README and send.cpp.
*/

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>

RCSwitch mySwitch;

int main(int argc, char *argv[]) {

    // Use BCM number, see: https://wiringpi.com/pins
    int PIN = 27;

    printf("Setting up GPIO...\n");
    setenv("WIRINGPI_GPIOMEM", "1", 0);
    if(wiringPiSetupGpio() == -1)
        return 0;

    mySwitch = RCSwitch();
    mySwitch.enableReceive(PIN);  // Receiver on inerrupt 0 => that is pin #2

    printf("RFSniffer started, click a button.\n\n");
    while(1) {

        if (mySwitch.available()) {
            int value = mySwitch.getReceivedValue();

            if (value == 0) {
                printf("Unknown encoding\n");
            } else {    

                int bLength = mySwitch.getReceivedBitlength();
                int pLength = mySwitch.getReceivedDelay();
                char* bValue = mySwitch.dec2binWzerofill(value, bLength);

                printf("Decimal: %i\n", value );
                printf("Bit length: %i\n", bLength);
                printf("Pulse length: %i\n", pLength);
                printf("Binary: %s\n\n", bValue);

            }
            mySwitch.resetAvailable();
        }
    }
    exit(0);
}

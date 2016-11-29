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
#include <iostream>

RCSwitch mySwitch;

int main(int argc, char *argv[]) {

    // Use BCM number, see: https://wiringpi.com/pins
    int PIN = 27;

    std::cout << "Setting up GPIO..." << std::endl;
    setenv("WIRINGPI_GPIOMEM", "1", 0);
    if(wiringPiSetupGpio() == -1)
        return 0;

    mySwitch = RCSwitch();
    mySwitch.enableReceive(PIN);  // Receiver on inerrupt 0 => that is pin #2

    std::cout << "RFSniffer started, click a button." << std::endl << std::endl;
    while(1) {

        if (mySwitch.available()) {
            unsigned long long value = mySwitch.getReceivedValue();

            if (value == 0) {
                printf("Unknown encoding\n");
            } else {    

                int bLength = mySwitch.getReceivedBitlength();
                int pLength = mySwitch.getReceivedDelay();
                char* bValue = mySwitch.dec2binWzerofill(value, bLength);

                std::cerr << "Decimal: " << value  << std::endl;
                std::cerr << "Bit length: " << bLength << std::endl;
                std::cerr << "Pulse length: " << pLength << std::endl;
                std::cerr << "Binary: " << bValue << std::endl;

            }
            mySwitch.resetAvailable();
        }
    }
    exit(0);
}

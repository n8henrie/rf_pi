/*
    send.cpp by @n8henrie
    https://github.com/n8henrie/rf_pi


    - Modified from https://github.com/ninjablocks/433Utils/blob/master/RPi_utils/codesend.cpp
    - which was modified from https://github.com/ninjablocks/433Utils/blob/master/Arduino_sketches/Send_Demo/Send_Demo.ino
    - which was modified from https://github.com/r10r/rcswitch-pi/blob/master/send.cpp

    ## Usage:

    - Set `PIN`, `PULSELENGTH`, and `BITLENGTH` if different than the defaults
    - `./send 12345` where 12345 is your *decimal* RF code (not binary or Tri-State)

    ## Important changes:

    - Externalizes the `send` function to make it available to other languages as a
      shared library
    - Jessie branch uses wiringPiSetupGpio() and sets WIRINGPI_GPIOMEM to use
      /dev/gpiomem instead of wiringPiSetup() to avoid need for root privileges
    - Wheezy branch uses wiringPiSetupSys() (and depends on pre-exported GPIO pins)
      instead of wiringPiSetup() to avoid need for root privileges
        - Wheezy branch **requires** that gpio pin have been exported by wiringPi2
          gpio function beforehand due to use of wiringPiSetupSys, e.g. `sudo -u
          $gpio_user /usr/local/bin/gpio export $pin out`
    - If run as sudo / root *or* set to have `setcap cap_sys_nice+ep`, both
      branches will set high priority scheduling for more reliable RF transmission,
      but still runs otherwise
    - See README.md for other details 
*/

#include "RCSwitch.h"
#include <stdlib.h>
#include <sched.h>
#include <string>
#include <cstring>

extern "C" int send(unsigned long long switches[], int num_switches, int iterations = 3,
                    int pin = 17, int pulseLength = 190, int bitLength = 24){
    // Have to use the BCM pin instead of wiringPi pin (e.g. 17 instead of 0)
    // if using wiringPiSetupGpio() or wiringPiSetupSys(). See:
    // http://wiringpi.com/pins/
    
    // Set the env variable to use /dev/gpiomem for easier rootless access.
    // Will *not* overwrite the existing value, so if you know you don't want
    // this, `export WIRINGPI_GPIOMEM=0` beforehand
    // http://wiringpi.com/wiringpi-update-to-2-29/
    setenv("WIRINGPI_GPIOMEM", "1", 0);

    if ((strcmp(getenv("WIRINGPI_GPIOMEM"), "0") == 0) || (wiringPiSetupGpio() == -1)){
    // Env var is `0` or can't set up `/dev/gpiomem`, so try wiringPiSetupSys

      // Make sure runnning Jessie and check udev rule and /dev/gpiomem
      // permissions. Fallback to `wiringPiSetupSys`, which depends on exported
      // GPIO pin, either done previously through the `gpio` utility or through
      // a `system()` call earlier in the script, e.g.: system("gpio export 17
      // out");
            
      if (wiringPiSetupSys() == -1) return 1;
    }

	RCSwitch mySwitch = RCSwitch();
	mySwitch.enableTransmit(pin);
    mySwitch.setPulseLength(pulseLength);

    // Make iteractions the outer loop and rotate through the switches inside
    // to try to maximize reliability; i.e. if you're tring to toggle 3 switches
    // with codes 1, 2, and 3, and iterations = 2, send `1 2 3 1 2 3` instead of 
    // `1 1 2 2 3 3`, which may have a higher risk of a brief timing glitch screwing
    // up a single code's transmission entirely.
    for (int i=0; i<iterations; i++) {
        for (int n=0; n<num_switches; n++) {
           
            // Uncomment to print to stdout. Note: will print each one $iterations times
            // printf("sending code: %i\n", switches[n]);
            
            mySwitch.send(switches[n], bitLength);
        }
    }

    return 0;
}

// Convenience function for debugging sched when called directly in c++
// http://yumichan.net/programming/obtain-a-list-of-process-scheduling-policy-and-priority/
void debug_sched(const char* msg = "") {
    int ret;
    struct sched_param stSched;
   
    printf("%s\n", msg);
    ret = sched_getscheduler(0); // get the policy
    if (ret < 0) {
        printf("Error: sched_getscheduler\n");
    }

    printf("pid 0 Policy: %d ", ret);

    switch (ret) {
        case SCHED_OTHER:
            printf("SCHED_OTHER\t");
            break;
        case SCHED_FIFO:
            printf("SCHED_FIFO\t");
            break;
        case SCHED_RR:
            printf("SCHED_RR\t");
            break;
        default:
            printf("No matched schedule policy!\n");
            break;
    };

    ret = sched_getparam(0, &stSched); // get the priority
    if (ret != 0) {
        printf("Error: sched_getparam\n");
    }
    printf("Priority: %d\n\n", stSched.sched_priority);
}

int main(int argc, char *argv[]) {
   
    // argv includes executable as argv[0], so we'll need to strip off argv[0]
    // and decreased argc by 1 to make sure send() can receive switches from
    // either the command line via main() or ctypes from python and expect
    // the same number of args
    int num_switches = argc - 1;
    unsigned long long switches[num_switches];
    for (int i=0; i<num_switches; i++) {
        switches[i] = std::stoll(argv[i+1]);
    }

    // Set scheduling if program is run directly from command line. Returning
    // to the original policy doesn't affect much when called in this simple
    // script, but is included in case this is a template for use in another
    // long-running process, in which case only the actual RF transmission
    // should have the max priority.

    // debug_sched("Pre setting priority");
    struct sched_param orig_sched;
    int orig_policy = sched_getscheduler(0);
    std::memset(&orig_sched, 0, sizeof(orig_sched));
    sched_getparam(0, &orig_sched);

    struct sched_param sched;
    std::memset (&sched, 0, sizeof(sched));
    sched.sched_priority = sched_get_priority_max (SCHED_RR);
    sched_setscheduler (0, SCHED_RR, &sched);
    // debug_sched("Post setting priority");
    
    send(switches, num_switches);

    sched_setscheduler (0, orig_policy, &orig_sched);
    // debug_sched("Post returning priority");

    return 0;
}

"""rf_send.py

https://github.com/n8henrie/rf_pi

Example script for sending RF codes using Python and ctypes on a
Raspberry Pi. Includes example decorator to make a function run as high
priority, which is important for reliable timing of the RF transmission.

Prerequisites:
    - Requires python >= 3.3 for optimal reliability, but runs on prior
      versions including 2.7 if needed
    - `sudo apt-get install libcap2-bin` to get the `setcap` command
    - `sudo setcap cap_sys_nice+ep /path/to/python3` to allow python to set
      scheduling priority, but see disclaimer in README for possible security
      ramifications
    - Must use the actual path to the binary, `setcap` doesn't work with a
      symlink
    - Verify permissions were set with `getcap /path/to/python3.5`, should
      return `/path/to/python3.5 = cap_sys_nice+ep`
Usage:
    - Toggle codes on and off multiple times (useful for testing):
        - `python3 rf_send.py test`
    - Send a code (or list of codes) once:
        - `python3 rf_send.py decimal_code [ decimal_code2... ]`

If set to test indefinitely, prints the number of toggles sent after killed,
making it easy to count the number observed to work and calculate reliability.
"""

import ctypes
import os.path
import sys
import ctypes.util
import time
import os
from functools import wraps
import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S',
    )

logger_name = str(__file__) + " :: " + str(__name__)
logger = logging.getLogger(logger_name)

# Replace these with default codes to use with `rf_send.py test`
DEFAULT_TEST_ONCODE = 12345
DEFAULT_TEST_OFFCODE = 54321


def hi_priority(func):
    """Decorator to set close to realtime scheduling policy for things that
    benefit from near-realtime execution (like RF signal transmission)."""

    @wraps(func)
    def wrapper(*args, **kwargs):
        did_sched = False
        try:
            pri = os.sched_get_priority_max(os.SCHED_RR)
            sched = os.sched_param(pri)
            os.sched_setscheduler(0, os.SCHED_RR, sched)
            did_sched = True

        except PermissionError as e:
            msg = ("Looks like you haven't set scheduling capabilities "
                   "for your python executable, so you can't run with "
                   "high priority. To disable this message, either consider "
                   "running `sudo setcap cap_sys_nice+ep /path/to/python3` or "
                   "comment out the `@hi_priority` decorator above the "
                   "rf_send function, since it's not working anyway, or just "
                   "increase the logging threadshold higher than `info`. NB: "
                   "`setcap` will *not* work on symlinks, you must provide "
                   "the path to the actual python3 executable.")
            logger.info(e)
            logger.info(msg)
        func(*args, **kwargs)

        if did_sched:
            os.sched_yield()
    return wrapper if sys.version_info >= (3, 3) else func


@hi_priority
def rf_send(codes, iterations=3, pin=17, pLength=190, bLength=24):
    """Send a list of RF codes using the RCSwitch library (by way of
    send.so)."""
    codes = [int(code) for code in codes]

    dirname = os.path.dirname(os.path.abspath(__file__))
    rf_lib = os.path.join(dirname, 'send.so')
    codesend = ctypes.CDLL(rf_lib)

    codes_arr = (ctypes.c_int * len(codes))(*codes)

    # Usage: codesend.send(length, iterations, codes)
    codesend.send(codes_arr, len(codes), iterations, pin, pLength, bLength)


def toggle(oncodes, offcodes, sleep_time=0.5):

    time.sleep(sleep_time)
    print("Turning on...")
    rf_send(oncodes)

    time.sleep(sleep_time)
    print("Turning off...")
    rf_send(offcodes)

if __name__ == '__main__':

    if sys.version_info < (3, 3):
        msg = ("Looks like you may be running a python version less than "
               "3.3, which means you can't use the os.sched stuff required "
               "for the @hi_priority decorator. `rf_send.py` may still work, "
               "probably just with less reliability than it would with python "
               ">= 3.3. To disable this message, comment out the "
               "`@hi_priority` decorator above the `rf_send` function, since "
               "it's not working anyway, or upgrade python to >= 3.3, or "
               "increase the logging threshold higher than `info`.")
        logger.info(msg)
    if sys.argv[1] == 'test':
        oncodes_str = input("What are the on codes? (blank for default) ")
        if oncodes_str == '':
            oncodes = [DEFAULT_TEST_ONCODE]
            offcodes = [DEFAULT_TEST_OFFCODE]
        else:
            offcodes_str = input("What are the off codes? ")
            split_codes = lambda codes: [int(code) for code in codes.split()]
            oncodes = split_codes(oncodes_str)
            offcodes = split_codes(offcodes_str)
        times = input("How many times to toggle on and off? (Default: "
                      "indefinitely) ")
        counter = 0
        if times == '':
            while True:
                try:
                    counter += 1
                    toggle(oncodes, offcodes, sleep_time=0.5)
                except KeyboardInterrupt:
                    print("\nThat ran {} times.".format(counter))
                    break
        elif times.isdigit():
            try:
                for _ in range(times):
                    toggle(oncodes, offcodes, sleep_time=0.5)
            except KeyboardInterrupt:
                    print("\nThat ran {} times.".format(counter))

    elif all(arg.isdigit() for arg in sys.argv[1:]):
        codes = [int(arg) for arg in sys.argv[1:]]
        rf_send(codes)
    else:
        print("All arguments should be codes unless called with 'test'")

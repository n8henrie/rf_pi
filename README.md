# [rf_pi](https://github.com/n8henrie/rf_pi)

<http://n8henrie.com/2015/12/rf_pi>

Includes various scripts for configuring and testing 433 MHz RF transmission
from a Raspberry Pi *without using sudo / root*. A number of the scripts were
copied with little if any modification from those found at:

- <https://github.com/r10r/rcswitch-pi>
- <http://code.google.com/p/rc-switch>
- <https://github.com/ninjablocks/433Utils>
- <https://projects.drogon.net/raspberry-pi/wiringpi>

## Background / Introduction

Numerous similar projects and tutorials exist, but nearly all depend on running
the script as sudo or root user, which can have major security implications. My
major goal with this project that makes it slightly different than others is to
make everything functional for a non-root user, and I seem to have everything
working pretty well.

I left the main code RCSwitch code almost entirely unchanged but have modified
`send.cpp` to:

- use the decimal RF code (thanks to the folks from 433utils)
- externalize code as a shared library for use in other scripts or
  languages
- add optional scheduling priority for greatly improved transmission
  reliability
- use [wiringPiSetupGpio() or
  wiringPiSetupSys()](http://wiringpi.com/reference/setup) for use *without
  root privileges*, more on this below

I've also added an example python3 script using
[`ctypes`](https://docs.python.org/3.3/library/ctypes.html) to show how the
shared library can be imported and used in other language.

NB: For those exclusively interested in using Python to interact with their 433
switches, I *highly* recommend you check out <https://github.com/milaq/rpi-rf>,
which simplifies things by porting *all* of the logic to Python instead of
using shared libraries and C++ as I've done here.

## Disclaimer

In my experience, the scheduling stuff below is necessary for optimal
reliability of RF transmission from the Pi, since it doesn't operate in
realtime (like an Arduino for example). However, the scheduling capabilities
either require the program to be run as root -- which has major security
implications, especially in an automated script -- or to have scheduling
capabilities given to the executable with `sudo setcap cap_sys_nice+ep`.

While this works for a standaone compiled C++ binary, e.g. `sudo setcap
cap_sys_nice+ep send`, unfortunately it does *not* seem to work for a *script*
called by an executable. In other words, you have to `sudo setcap
cap_sys_nice+ep /path/to/python3.4` using the `python3` that will run
`rf_send.py`.  Even if you add a shebang, `chmod +x rf_send.py`, and then
`setcap` on `rf_send.py`, it won't work -- you have to set the permissions on
`python3.4` and not on `rf_send.py`.

The issue here is that a malicious process or coding error could make a script
that would run indefinitely at very high priority, hogging all system
resources, and it may be able to run it as an unprivileged user and lock up the
system (whereas it would normally require root privileges to run a script that
sets scheduling priority). If such a script were set to run automatically at
boot, it might be really difficult to recover the system.

On the other hand, if you want to be able to reliably trigger RF transmissions
from another process like a webapp, you'd otherwise have to run your *entire
webapp* as `root`. This seems like a bigger risk to me, but it's your call.

I'm just an amateur / hobbyist, so if any of this is outright wrong or grossly
misleading, feel free to message me or make a PR. For testing purposes, I
*have* tried to make a python script "bomb" that set itself to maximum priority
and looped indefinitely (`while True: pass`) and ran it in the background;
while it made things *very* slow, it's didn't entirely freeze the system, I was
still able to kill the process without issue, so maybe I'm overblowing the
security ramifications here.

## Installation and setup

Tested and working on a Raspberry Pi B+ running Jessie. Was working on Wheezy
before, but I've made a few modifications since then -- would love if someone
can confirm and let me know, you'll probably need to change to gcc 4.7 in
`Makefile`.

You'll need your **decimal** RF code, which you can find with RFSniffer.

1. Make sure you have a `gpio` group, and whatever users will run the scripts
   (e.g. `pi`, `n8henrie`, `gpio`, `www-data`, etc.) are `gpio` members
   - Add group: `groupadd -f --system gpio`
   - Check members: `grep gpio /etc/group`
   - Add user `fake` to `gpio` group: `sudo usermod -a -G gpio fake`
1. Install
   [wiringPi](https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/)
1. Identify [your gpio pin(s)](http://wiringpi.com/pins)
1. Recommended: to take advanced of scheduling priority, `sudo apt-get install
   libcap2-bin`
1. Clone the repo, edit files as needed, and `make`:

```bash
git clone https://github.com/n8henrie/rf_pi.git
cd rf_pi

# Only if on wheezy, stay on master for jessie:
# git checkout wheezy

# Edit files with your pin values, etc.
vim RFSniffer.cpp # edit to change your pin number if needed
vim send.cpp # edit to change your pin number / pulse length if needed

# Will ask for sudo privileges for the `setcap` step and will give an (ignored)
# error if you didn't install libcap2-bin:
make
```

### Raspbian Jessie

- By default takes advantage of new `/dev/gpiomem` interface via wiringPi
- Details: <http://wiringpi.com/wiringpi-update-to-2-29>
- Requires `bcm2835_gpiomem` kernel module
    - Check if loaded: `cat /proc/modules | grep gpio`
- May require the `udev` rule described at the link above
    - In the most recent Jessie releases, `/etc/udev/rules.d/99-com.rules`
      is built-in and seems to suffice
    - If needed, I've copied the wiringPi recommended `udev` rule to the
      `extras` folder for convenience; copy to `/etc/udev/rules.d/`
- Use the wheezy setup below if you don't want to use `/dev/gpiomem`

### Raspbian Wheezy

- Check out the `wheezy` branch after cloning: `git checkout wheezy`
- Requires that the gpio pins be exported with the wiringPi `gpio` utility
  *prior* to calling the `send` or `RFSniffer` scripts, e.g.  `gpio export 17
  out` or `gpio export 27 in`
    - Verify what pins are exported with `gpio exports`
- Optional: I've included a sample shell script to export a list of pins:
  `extras/rf_setup.sh`
- Optional: I've included a sample init script (for Wheezy, `extras/rf_setup`)
  and systemd service file (for Jessie, `extras/rf_setup.service`) to automate
  running `rf_setup.sh` at boot, making your pins automatically available to
  members of the `gpio` group

## Usage

1. Wire up your 433 MHz RF transmitter and / or receiver
1. Note your **BCM** pin number(s): <https://wiringpi.com/pins>
1. To find your codes: `./RFSniffer`
1. To send a code: `./send 12345 [23456] [34567]...` where 12345 is a
   **decimal** RF code

### Usage notes

- If set up as described above does *not* require root privileges
- If called directly from the command line (e.g. `./send`) it will try to take
  advantage of scheduling priority capabilities (check the output of `getcap
  send`) if possible, but runs okay without it
- Exports the `send` function in the shared library `send.so`, which does *not*
  attempt to set scheduling priority. If needed, scheduling priority should be
  done in whatever program is calling the function
- `send` accepts as many decimal RF codes as you want to give it (as subsequent
  arguments) and cycles through calling each multiple times (as configured in
  send.cpp, default 3 times). This is to facilitate toggling "groups" of
  switches with maximum reliability. If you put a single switch's on and off
  code as arguments, it will just flicker that switch on and off.

## `rf_send.py`

I wanted to learn how to use c++ programs from Python, and so `rf_send.py` is
my first experiment with
[`ctypes`](https://docs.python.org/3.3/library/ctypes.html). It uses the shared
library `send.so` (created automatically when you `make`) to call the `send`
function from `send.cpp`. I originally had named it `send.py`, but if you try
to `import send` from another script this gives you a naming conflict (with
`send.so`), so I renamed it. `rf_send.py` requires python >= 3.3 to take
advantage of setting process priority, but will also run on python2.7 and
python3.2 -- just with worse reliability as the lower process priority makes
accurate RF transmission less likely. Just FYI, I don't intend to devote any
time to maintaining python2 compatilibity in any future updates.

To use `rf_send.py` reliably, I found that the scheduling priority was critical,
so I wrote decorator `@hi_priority` that can decorate timing-critical
functions. When run by a non-root user, `rf_send.py` will raise an exception if
your `python3` does not have `setcap cap_sys_nice+ep`. Feel free to comment out
`@hi_priority` above `def rf_send` if you don't believe me or want to
experiment. To set this capability, as per above use `sudo setcap
cap_sys_nice+ep /path/to/python3.4`. As explained above, `setcap` will **not**
work on a symlink, it has to be the *actual binary*, so if you're having issues
here, double check:

```bash
$ file /usr/bin/python3
/usr/bin/python3: symbolic link to python3.4
$ file /usr/bin/python3.4
/usr/bin/python3.4: ELF 32-bit LSB executable...
```

Verify it's set with `getcap /path/to/python3.4` -- should return
`/path/to/python3.4 = cap_sys_nice+ep`.

`rf_send.py` can either send the decimal RF code(s) given as argments, (just
like `./send`), or you can run it in "test" mode, which

### `rf_send.py` usage:

- Send a code: `python3 rf_send.py 12345`
- Use `test` mode: `python3 rf_send.py test`

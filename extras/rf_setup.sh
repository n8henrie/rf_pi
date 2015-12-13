#! /bin/bash
# rf_setup.sh
# Run at boot with the init script, systemd service, or rc.local. to export GPIO pins
# Don't forget to make it executable: `sudo chmod +x rf_setup.sh`

name=`basename $0`
stdout_log="/var/log/$name.log"
stderr_log="/var/log/$name.err"
user="pi" # replace with gpio user as needed
gpio_group="gpio"
gpio_tool="/usr/local/bin/gpio" # replace with path to `gpio` as appropriate


case "$1" in
  start)
    echo "Starting rf_setup.sh"

    # Uses wiringPi2 to export appropriate permissions for $gpio_tool pins
    out_pins="17" # space separated list, e.g. "15 17 19"
    in_pins="27" # space separated list, e.g. "15 17 19"

    if [ -n "$out_pins" ]; then
      for out_pin in $out_pins; do
          sudo -u $user -g "$gpio_group" "$gpio_tool" export $out_pin \
          out  >> "$stdout_log" 2>> "$stderr_log"
          chmod 664 $(readlink -f /sys/class/gpio/gpio$out_pin)/{edge,value}
      done
    fi
    if [ -n "$in_pins" ]; then
      for in_pin in $in_pins; do
          sudo -u $user -g "$gpio_group" "$gpio_tool" export $in_pin in \
          >> "$stdout_log" 2>> "$stderr_log"
          chmod 664 $(readlink -f /sys/class/gpio/gpio$in_pin)/{edge,value}
      done
    fi
  ;;
  stop)
    sudo $gpio_tool unexportall
  ;;
esac

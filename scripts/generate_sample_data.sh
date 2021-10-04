#!/usr/bin/bash

main() {
  DEVICE_NAME=${1:-PenTablet}
  TIME_PERIOD_MS=${2:-1000}

  if $(lsusb | grep -q "$DEVICE_NAME"); then
    dev_addr=$(lsusb | grep $DEVICE_NAME | tr -d ':' | awk '{ printf($2":"$4) }')
    echo "Found $DEVICE_NAME at USB address $dev_addr" >> /dev/stderr
    echo "Querying signal using usbhid-dump for $TIME_PERIOD_MS ms" >> /dev/stderr


    usbhid-dump -t $TIME_PERIOD_MS -es -a $dev_addr | grep -v "^$" | grep -v ".*STREAM.*"
  else
    echo "Device $DEVICE_NAME not found."
  fi
}

if [[ "$@" =~ .*\-h\.* ]]; then
  echo    "Usage: ./generate_sample_data.sh [<device_name>] [<time_in_ms>]"
  echo -e "Records <time_in_ms> seconds of usb input data for <device_name>\n"
  echo    "  <device_name>      Device Name, optional, default PenTablet"
  echo    "  <time_in_ms>       Recording Time, optional, default 1000"
else
  main $@
fi

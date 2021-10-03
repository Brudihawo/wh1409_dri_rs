# Driver for Huion WH1409

My plan for this is to write a driver for the Huion WH1409 graphic tablet in rust by
understanding the usb signal sent by the device and mapping that to controls.
This is largely an exercise for fun and might not produce usable driver.

THIS IS A WORK IN PROGRESS AND BY NO MEANS DONE.
CURRENT STAGE IS FIGURING OUT HOW THE SIGNAL WORKS.


# Plan of Action
1. [x] Acquire sample data using usbhid-dump (This is a script now.)
    1. Find device adress using `lsusb`
    2. Use `usbhid-dump -es -t 1000 -a <address>` to generate 1s of Signal data
2. Create function in Rust for reading and parsing that data
3. Find out how to directly read from usb
4. Find out how to set mouse position etc.
5. Find out how interaction of drivers with drawing programs like GIMP works

## TODOs
- [ ] Figure out endianness of signal and processing
- [ ] Figure out signed-ness of signal (current assumption = unsigned maybe wrong)
- [ ] Better plotting for signal analysis
- [ ] Figure out if sent signal is an offset of some sort

# Dependencies
mostly for debugging / signal generation purposes

| Program       | Use                     |
|---------------|-------------------------|
| lsusb         | find device name        |
| usbhid-dump   | generate sample signal  |
| grep, tr, awk | filtering sample signal |

# How-To of Scripts found in ./scripts
## Generating Sample Data
`generate_sample_data.sh` is a small script to generate a stream of bytes (as hex
string) mostly for debugging purposes (e.g. to have sample data to use repeatedly).
Intended use is like this:
```bash
generate_sample_data.sh > sample_signal.txt
```
Intention is to then be able to decode that using a decoding function

## Ploting processed Outputs
`plot_output.py` can be used to plot the processed signals. Expects values to be
in format: (vertical, horizontal, pressure), one datapoint per line, separated by a
single space.

## Processing multiple signal files
After generating sample signals, you might want to process all of them as a batch.
Place them in `./data/in/` and run `./scripts/process_all_in_signals.sh`. This will
process all of them and place output files in `./data/out/`.


# Signal Structure

Each Stream Signal is Composed of 8 Bytes:
1. Two Bytes for the Status:
    1. `07 80` if Pen is lifted and in range of tablet (-> move mouse)
    2. `07 81` if Pen is touching the tablet (-> click / draw)
2. 4 Bytes (little endian) encoding Position (Vertical, Horizontal)
3. 2 Bytes encoding Pressure (little endian)

## Position Encoding
Something fishy is going on. Plotting the Signals for (almost) purely vertical movement
yields this:
![Vertical Movement](./img/vertical_movement_plot.png)

Signal duration for each time was around 1-2 Seconds and the motion performed were 2 up
and down motions. However, this is not represented in the signal as clearly as one would expect.

## Pressure Encoding
Limited at high end to 0x07FF, probably due to sensor resolution. Low threshold might
be limited through calibration in device firmware.
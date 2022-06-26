# pillScope
Oscilloscope based around the STM32F103 Blue Pill and an OLED screen\
_Take a look at [pillScope Plus](https://github.com/tvlad1234/pillScopePlus), with a better screen and more sample memory_
## Features
-3.3V to 3.3V input range (can be increased if using attenuator probes)\
Approximatively 1MOhm input impedance\
Timebase goes down to 20uS/div\
On screen measurements: min/max voltage, peak-to-peak voltage, frequency\
Captured waveforms can be sent over USB or UART in TekScope-compatible CSV format.
## Required parts
### Base parts:
STM32F103C8 Blue Pill development board\
128x64 SSD1306-based OLED display\
4 pushbuttons
### Analog frontend:
LM358 dual op-amp (rail-to-rail opamps should work better in this context, but this is what I had on hand)\
2x 68kOhm resistors (to create a 1.65V offset voltage)\
2x 500kOhm resistors (to create the input attenuator)\

### 2x probes:
just a 1MOhm resistor, in series with the input

## Schematics
The OLED display is connected to I2C1 (PB7-SDA and PB6-SCL). The buttons are connected as follows:\
PB12: Menu\
PB13: Select\
PB8: Down\
PB9: Up

PA9 is the UART TX, PA10 is RX

The output of the analog frontend is connected to ADC1_IN0, which corresponds to PA0.

The analog frontend consists of:\
a 1.65V voltage reference, which serves as a virtual ground point for the input,\
a 2x voltage divider at the input,\
an LM358 dual op-amp, which buffers the reference voltage and the output of the input attenuator

The schematic of the analog frontend can be found in the frontend.pdf file.

## Using the oscilloscope
### The UI
The menu buton cycles between menus on the right side of the screen, while the select button changes the current selection in the menu.

### Measuring things
The frontend makes use of a virtual ground point which is 1.65V above the real ground. Because of this, the oscilloscope and the device under test must not be sharing the same ground reference. If you need to send data to the computer while measuring a device which shares ground with the scope, you should connect the computer via the UART port, with an opto-isolated adapter, while powering the oscilloscope from an external source.

## Saving wavevorms
The captured waveforms can be sent to a computer over USB or UART. Sending `s` or `S` to either port will tell the scope to output the captured waveform in CSV format. This data can then be imported into the Tektronix TekScope app for further analysis.
![Waveform in TekScope](https://user-images.githubusercontent.com/60291077/174594659-d71b9acf-26f0-4e4b-9766-6355c0acc5a1.png)
![Waveform on pillScope](https://user-images.githubusercontent.com/60291077/175494191-82f16835-0c3a-488d-b7e0-959405abd570.jpg)


## Code
The code can be compiled with `make`. Be sure to recursively clone this repository, as the display driver is included as a submodule. The actual oscilloscope code of this project is located in `Core\Src`, the `scope.c`, `ui.c`, `wave.c` files. Feel free to take a look, as they're commented for ease of understanding.

## Pictures
![20220619_165356](https://user-images.githubusercontent.com/60291077/174484756-e336c5bb-27e9-40c6-923a-6aa228a2cb00.jpg)
![20220619_165433](https://user-images.githubusercontent.com/60291077/174484767-cb0bdf95-f4b4-4de8-8a6a-038e26494a6b.jpg)


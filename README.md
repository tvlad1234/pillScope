# pillScope
Oscilloscope based around the STM32F103 Blue Pill and an OLED screen
## Specifications
-3.3V to 3.3V input range (can be increased if using attenuator probes)\
Approximatively 1MOhm input impedance\
Timebase goes down to 20uS/div\
Captured waveforms can be sent over USB in TekScope-compatible CSV format.
## Required parts
### Base parts:
STM32F103C8 Blue Pill development board\
128x64 SSD1306-based OLED display\
4 pushbuttons
### Analog frontend:
LM358 dual op-amp (rail-to-rail opamps should work better in this context, but this is what I had on hand)\
2x 68kOhm resistors (to create a 1.65V offset voltage)\
2x 500kOhm resistors (to create the input attenuator)
### 2x probes:
just a 1MOhm resistor, in series with the input

## Schematics
The OLED display is connected to I2C1 (PB7-SDA and PB6-SCL). The buttons are connected as follows:\
PB12: Menu\
PB13: Select\
PB8: Down\
PB9: Up

The output of the analog frontend is connected to ADC1_IN0, which corresponds to PA0.

The analog frontend consists of:\
a 1.65V voltage reference, which serves as a virtual ground point for the input\
a 2x voltage divider at the input\
an LM358 dual op-amp, which buffers the reference voltage and the output of the input attenuator

The schematic of the analog frontend can be found in the frontend.pdf file.

## Saving wavevorms
The captured waveforms can be sent to a computer over USB. Sending `s` or `S` to the USB serial port will tell the scope to output the captured waveform in CSV format. This data can then be imported into the Tektronix TekScope app for further analysis.
![Waveform in TekScope](https://user-images.githubusercontent.com/60291077/174594659-d71b9acf-26f0-4e4b-9766-6355c0acc5a1.png)
![Waveform as seen on pillScope](https://user-images.githubusercontent.com/60291077/174594986-da637a78-c6e8-41b8-afad-d20e912a3005.jpg)


## Pictures
![20220619_165356](https://user-images.githubusercontent.com/60291077/174484756-e336c5bb-27e9-40c6-923a-6aa228a2cb00.jpg)
![20220619_165433](https://user-images.githubusercontent.com/60291077/174484767-cb0bdf95-f4b4-4de8-8a6a-038e26494a6b.jpg)
![20220619_165454](https://user-images.githubusercontent.com/60291077/174484772-ff6349dc-41c2-4f28-a758-074f50fc7af4.jpg)




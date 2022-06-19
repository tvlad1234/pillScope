# pillScope
Oscilloscope based around the STM32F103 Blue Pill and an OLED screen
## Specifications
-3.3V to 3.3V input range (can be increased if using attenuator probes)\
Approximatively 1MOhm input impedance\
Timebase goes down to 50uS/div
## Required parts:
### Base parts:
STM32F103C8 Blue Pill development board\
128x64 SSD1306-based OLED display\
4 pushbuttons
### Analog frontend:
LM358 dual op-amp (rail-to-rail opamps should work better in this context, but this is what I had on hand)\
2x 68kOhm resistors (to create a 1.65V offset voltage)\
2x 500kOhm resistors (to create the input attenuator)
### 2x probes
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



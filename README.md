# avr-netino
Automatically exported from code.google.com/p/avr-netino

This project summarizes some patches to the arduino core and bootloader to get the AVR-Net-IO board programmable by the arduino IDE. Additional included are some patched libraries to support the io functions of the board (network,LCD,RFM12,...)

The philosophy behind my changes is to support many avr's in the core files and to concentrate the board specific details in one file. So porting to other hardware should be very easy.


>EtherCard is updated to Ver. of Aug. 2013 from https://github.com/jcw/ethercard

>LiquidCrystal_I2C is updated to Ver. 2.0 from http://hmario.home.xs4all.nl/arduino/LiquidCrystal_I2C/

>RM12 is replaced by JeeLib of Oct. 2013 from https://github.com/jcw/jeelib

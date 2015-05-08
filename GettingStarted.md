# Introduction #

Here you find instructions to get avr-netino working.
Avr-Netino is a port of the [arduino](http://arduino.cc) _avr_ software to
program the
AVR-Net-IO board (Art.Nr. 810 058 or 810 073) from [Pollin](http://www.pollin.de/) with the arduino ide. The hardware of the optional Add-on for AVR-Net-IO (Art.Nr. 810 112) is supported by additional libraries.

# Contents #
  * What you need
  * Installation
  * burning bootloader
  * running sketches

# What you need #
  * An AVR-Net-IO board with ATmega32/ATmega644(p) and _auto reset_ feature.
See AddAutoReset for   instructions to add this to your board.
  * an AVR isp programmer, or an other arduino board
  * The Arduino software version 0022 or later.
  * the [zip-file](http://code.google.com/p/avr-netino/downloads/list) with avr-netino
  * optional: Add-on for AVR-Net-IO

# Installation #
  1. Install arduino software.
  1. Unpack the avr-netino-`*`.zip file in the arduino installation directory. This will add avrnetio under hardware and patch SD lib in libraries. Libraries for the Add-on hardware are installed too.
  1. If you install from the mercurial repository, you have to compile the bootloader and to create the boards.txt file in hardware/avrnetio/.

# burning bootloader #
  1. connect your board with your isp programmer
  1. start arduino ide
  1. select avrnetio w/ _your ATmega_ from **Tools->Board**
  1. select **Tools->Programmer->_your programmer_**
  1. select **Tools->Burn Bootloader**

# running sketches #
  1. connect your board via RS232 (a USB to RS232 cable will be ok)
  1. start arduino ide
  1. write a sketch or load an example - look at WhatYouGet for differences to arduino boards
  1. select avrnetio w/ _your ATmega_ from **Tools->Board**
  1. hit upload button
> Have fun.
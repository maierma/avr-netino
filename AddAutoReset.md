# Introduction #

The auto reset feature from arduino boards enables the PC to reset the board by toggling the DTR line of the serial connection.

You can add this to your AVR-Net-IO board by soldering a capacitor between _DTR_ and _Reset_. Or alternatively (see comments below) between _RTS_ (through MAX232) and _Reset_.
Optional, but suggested, add a diode from _Reset_ to _Vcc_.


# Details #

The suggested circuit looks like:
```
DTR:  J5-4
   or    o-----||-----+-----|>|-----o Vcc 
RTS: IC4-9   100nF    |    1N4148   IC5-10
                      |
                Reset o IC5-9
```

When avrdude starts a download it will take down the _DTR_ line and thus giving a short pulse over the capacitor to reset the ATmega.

I've observed, that sometimes after finishing the download, when _DTR_ goes high again, the ATmega hangs. I've figured out, that this is caused by the high pulse on _Reset_. I think this confuses the ATmega, because a higher voltage on _Reset_ will enter High-Voltage-Programming-mode. With the additional diode I haven't this problem any more.

# Instruction #

I've soldered the capacitor (**100nF**)on the bottom of the PCB. From the serial connector **(J5) pin 4** to the ATmega **(IC5) pin 9**.

The diode, a _1N4148_ I've put on top side, **on-top of R 12**, which is the pull-up resistor between _Reset_ and _Vcc_.
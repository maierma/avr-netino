# What you get #

This side records the differences to the arduino boards.
Mostly they are caused by the different ATmega and,
of course, the boards have other IO features.


I changed the pin numbers to a more friendly way for the
AVR-Net-IO. They are also different to the sanguino, which
uses the same ATmega.

# Pins #

| **Pin Nr** | **Port** | **PWM** | **Names** |
|:-----------|:---------|:--------|:----------|
| 0 | C0 | - | pins\_port\_C0, pins\_SCL, pins\_J3\_2 |
| 1 | C1 | - | pins\_port\_C1, pins\_SDA, pins\_J3\_3 |
| 2 | C2 | - | pins\_port\_C2, pins\_TCK, pins\_J3\_4 |
| 3 | C3 | - | pins\_port\_C3, pins\_TMS, pins\_J3\_5 |
| 4 | C4 | - | pins\_port\_C4, pins\_TDO, pins\_J3\_6 |
| 5 | C5 | - | pins\_port\_C5, pins\_TDI, pins\_J3\_7 |
| 6 | C6 | - | pins\_port\_C6, pins\_TOSC1, pins\_J3\_8 |
| 7 | C7 | - | pins\_port\_C7, pins\_TOSC2, pins\_J3\_9 |
| 8 | A0 | - | pins\_port\_A0, pins\_ADC0, pins\_A0, A0 |
| 9 | A1 | - | pins\_port\_A1, pins\_ADC1, pins\_A1, A1 |
| 10 | A2 | - | pins\_port\_A2, pins\_ADC2, pins\_A2, A2 |
| 11 | A3 | - | pins\_port\_A3, pins\_ADC3, pins\_A3, A3 |
| 12 | A4 | - | pins\_port\_A4, pins\_ADC4, pins\_ADC\_1, A4 |
| 13 | A5 | - | pins\_port\_A5, pins\_ADC5, pins\_ADC\_2, A5 |
| 14 | A6 | - | pins\_port\_A6, pins\_ADC6, pins\_ADC\_3, A6 |
| 15 | A7 | - | pins\_port\_A7, pins\_ADC7, pins\_ADC\_4, A7 |
| 16 | D0 | - | pins\_port\_D0, pins\_RXD, pins\_RS232\_RxD |
| 17 | D1 | - | pins\_port\_D1, pins\_TXD, pins\_RS232\_TxD |
| 18 | D2 | - | pins\_port\_D2, pins\_INT0, pins\_LED\_1 |
| 19 | D3 | - | pins\_port\_D3, pins\_INT1, pins\_RFM12\_IRQ |
| 20 | D4 | + | pins\_port\_D4, pins\_OC1B, pins\_LED\_2 |
| 21 | D5 | + | pins\_port\_D5, pins\_OC1A, pins\_RFM12\_CS |
| 22 | D6 | + | pins\_port\_D6, pins\_OC2B, pins\_LED\_3 |
| 23 | D7 | + | pins\_port\_D7, pins\_OC2A, pins\_SDcard\_INS |
| 24 | B0 | - | pins\_port\_B0, pins\_T0, pins\_IR\_Rx |
| 25 | B1 | - | pins\_port\_B1, pins\_T1, pins\_JUMP\_PROG |
| 26 | B2 | - | pins\_port\_B2, pins\_INT2, pins\_ENC28J60\_IRQ |
| 27 | B3 | + | pins\_port\_B3, pins\_OC0, pins\_SDcard\_CS |
| 28 | B4 | - | pins\_port\_B4, pins\_SS, pins\_ENC28J60\_CS |
| 29 | B5 | - | pins\_port\_B5, pins\_MOSI, pins\_SPI\_MOSI |
| 30 | B6 | - | pins\_port\_B6, pins\_MISO, pins\_SPI\_MISO |
| 31 | B7 | - | pins\_port\_B7, pins\_SCK, pins\_SPI\_SCK |

## Explanation ##
  * Pins **0..11** are on Sub-D25 (**J3**)
  * Pins **12..15** are the screw connectors labelled with **ADC 1** to **ADC 4**
  * Pins **18..24,26** are on **EXT** connector
  * to make programming (and porting) easier, I've defined constants for all pins reflecting their usage (including the AddOn board).
  * constants beginning with pins`_` are declared in **pins\_arduino.h**, which is included by Arduino.h. So you don't have to include this file if used.

# Libraries #
The modified libraries are change in a compatible way, that means they will work with standard arduino boards as well as with the AVR-Net-IO board.

# Details #
This section describes the technical aspects of my mods.

## Constants ##
There is only two pre-processor constant:
```
#define AVR_NET_IO	0x20131003	/* Date 2013-10-03 */
```
which indicates that we compile for AVR-Net-IO board, and
```
#define AVR_Netino	0x20131003	/* Date 2012-10-03 */
```
to indicate that we us the avrnetio core files.
They are used to activate my extensions in the modified libraries.
If defined, pin numbers are read from the constants in the table above.
The value of the `AVR_Netino` constant is the date of the avrnetio core files.

I've defined some more constants to describe the AVR-Net-IO hardware:

| **Constant** | **Value** | **Comment** |
|:-------------|:----------|:------------|
| LED\_BUILTIN | pins\_LED\_1 |  |
| I2C\_LCD\_ADR | 0x20 | PCF8574=0x20 PCF8574A=0x38 (7Bit adr) |
| I2C\_LCD\_NBL | 0 | 4Bit-Bus at P0-P3 |
| I2C\_LCD\_RS | 0x10 | P4 D/I |
| I2C\_LCD\_RW | 0x20 | P5 R/W |
| I2C\_LCD\_E | 0x40 | P6 Enable |
| I2C\_LCD\_BL | 0x80 | P7 Back Light |
| ENC28J60\_INR | 2 | Int Nr of ENC28J60 |
| RFM12\_INR | 1 | Int Nr of RFM12 |

The constants in the upper table beginning with `pins_` are declared in **pins\_arduino.h** via board.def file.
They are all `enum` members, which are better constants than `static const int`

## The Xmacro Trick ##
The pin definitions and board specific constants are all concentrated in the board.def file.
For every pin there is a line like
```
pinDef( A, 4 , NOT_ON_TIMER,ADC4,ADC_1,4)
```
`pinDef` is a Xmacro, that means it will be redefined for different usage.
The argument are:
  1. the **port** as single upper letter, so it can be concatenated to `PIN`, `PORT` and`DDR`register names.
  1. the **bit** number in the port
  1. the **timer** used for pwm or `NOT_ON_TIMER` if this pin has no pwm capabilities
  1. the additional pin **function** from the ATmega datasheet
  1. the pin **usage** at the board, that is a pin name at a connector, or a label of a pin, or a specific funktion like RS232\_RxD or a chip select.
  1. the pin-change-interrupt number
  1. `...`, the macro definition should end with `...`, so it can be extended in the future.

Let's look at an example:

For arduino we need three arrays to define the pin functions.
  1. `const uint8_t PROGMEM digital_pin_to_port_PGM[] = {PD,PD,PD,...PC,...};`
  1. `const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {_BV(0),_BV(1),_BV(2),...};`
  1. `const uint8_t PROGMEM digital_pin_to_timer_PGM[] = { NOT_ON_TIMER, NOT_ON_TIMER,...};`
These are extracted from **board.def** with the Xmacro `pinDef` by these defines:
  1. `#define pinDef(p,B,T,...)	(P##p),`
  1. `#define pinDef(P,B,T,...)	_BV(B),`
  1. `#define pinDef(P,B,T,...)	(T),`

In **pins\_arduino.h** the `enum` with the `pins_` constants is filled with
```
#define pinDef(P,B,T,F,...) pins_##F,
```
The prefix `pins_` is needed, because some of the pin names like `INT0` are defined as macros in the AVR header files, so I couldn't use them as `enum` members.

## The big advantage ##
Putting all board specific things in one file makes it very easy to port the arduino IDE to other boards.
For this to work, the files in the core must support all ATmega in use.
With **arduino-0022** a lot of the '#ifdef AVR\_ATmegaXX' are replaced with `#ifdef `_Register_

I continued this strategy to support the 3. ext. Interrupt of the ATmega32/644(p).
Second, it is very easy to change pin numbers, simply by changing the
order of the `pinDef` lines in **board.def**.
And, last but not least, I introduced a lot of constants with pins for special functions,
which makes programming in a portable manner more clear.
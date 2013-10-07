PIC 16F627A Programmer
======================

**TLDR: Setup the circuit and run `python program.py program.asm`.**

Provides an Arduino sketch and python script to program a PIC 16F627A from an asm file. May work with other chips that implement the same programming interface (628A/648A).

Only tested with linux, and likely to need at least minor tweaks to work with other platforms.

The arduino code's heavily based upon Soranne/Ronan Gaillard's PIC programming project, available at http://forum.arduino.cc/index.php?topic=92929.0. Note: the serial interface is NOT compatible with Soranne's client-side windows program, although it shouldn't be too hard to modify either to be so though.

See `lights.asm` for an example assemly file.

`gpasm` and `gplink` are required, and are available in the `gputils` on debian and variants.

Circuit
-------
See `circuit.svg` and `circuit.jpg` for a circuit diagram and photograph(the LEDs are not required for programming).
![Circuit Layout](https://raw.github.com/hrickards/pic_programmer/master/circuit.svg)

Connections:
  - Pin 6 on arduino to RB6 on pic
  - Pin 5 on arduino to RB7 on pic
  - Pin 8 on arduino to VDD on pic
  - Pin 7 on arduino to RB4 on pic
  - Pin 3 on arduino to VPP on pic through a diode (allowing current from arduino -> pic, but not other way round)
  - Pin 2 on arduino connected to gate of a n-channel MOSFET. Source connected to ground. Drain connected to 12V supply through a 1k resistor, and also VPP on pic through a diode (allowing current from arduino -> pic, but not other way round).
  - Ground on arduino to VSS on PIC and ground of 12V supply.

Usage
-----
Run

    python program.py <filename> <serial port>
 
where `<filename>` is the name of the ASM/HEx file and `<serial port>` is the optional name of the serial port the Arduino is connected to.

Serial Interface
----------------
The following commands are available through the serial interface
  - b = Erase all program memory on chip (bulk erase)
  - c = Move program counter to config memory (R to reset back to program memory)
  - w = Write next word from serial to current program location
  - r = Read word at current location and print over serial
  - x = Put PIC into execution mode & run current program (R to reset back to programming mode)
  - i = Increment program counter
  - R = Reset PIC back to program memory, PC=0 & programming mode (same as on initial startup

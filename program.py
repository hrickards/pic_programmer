# Author: Harry Rickards
#
# Usage: python program.py <filename> <serial port>
# <filename> = filename of the asm or hex file to write to the chip
# <serial port> = optional location of the serial port to communicate with the arduino on
#
# Writes an asm/hex file to a PIC 16F627A chip using an arduino.

from subprocess import call
import sys, glob

from pic import Pic
from hexfile import HexFile

filename = sys.argv[1]
extension = filename.split(".")[-1].lower()
# If the file is an ASM file, compile it first
if extension == "asm":
    # Remove the extension from filename
    base = ".".join(filename.split(".")[:-1])
    # Create the hex filename
    hfilename = base + ".hex"

    # Run gpasm & gplink to compile the ASM into HEX
    call(["gpasm", "-c", filename])
    call(["gplink", "-q", base + ".o", "-o", hfilename])
# We don't need to compile if it's already a HEX file
elif extension == "hex":
    # The hex filename is just the passed filename
    hfilename = filename
# If it's not HEX or ASM, what is it?
else:
    raise Exception("Unknown file extension")

# If a serial port has not been passed, use the first available one
if len(sys.argv) < 3: sport = glob.glob('/dev/ttyUSB*')[0]
# Otherwise used the passed one
else: sport = sys.argv[2]

# Init new HexFile and PIC instances
hf = HexFile(hfilename)

prog = Pic(sport, 9600, 0x03FF)

# Write and verify the hex file to the pic
prog.write_file(hf)
prog.verify_file(hf)

# Write and verify the config word from the hex file to the pic
prog.write_verify_config_file(hf)

# Run the program on the pic
prog.execute()

# Close the connection with the pic
prog.close()

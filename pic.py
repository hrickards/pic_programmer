from time import sleep
from progressbar import ProgressBar
import serial

# Represents the PIC chip
class Pic:
    # port = string containing serial port of arduino (e.g. /dev/ttyUSB0)
    # baud_rate = baud rate to communicate with arduino on
    # max_location = the last memory location in programming memory
    def __init__(self, port, baud_rate, max_location):
        self.port = port
        self.baud_rate = baud_rate
        self.max_location = max_location

        # Init program counter to 0
        self.pc = 0

        # Init the device
        self.init_device()

    # Get a serial response
    def get_response(self):
        return self.ser.readline().strip()

    # Close the serial connection
    def close(self):
        self.ser.close()

    # Get a response, and check it's the same as wanted
    def assert_resp(self, wanted):
        resp = self.get_response()
        assert resp == wanted

    # Send the command, and check a blank response is returned
    def assert_d(self, ins):
        self.send(ins)
        self.assert_resp("")

    # Init the device
    def init_device(self):
        # Setup a serial connection
        self.ser = serial.Serial(self.port, self.baud_rate)

        # Reset the PIC
        self.reset()
        
        # Erase the PIC (required before programming)
        self.bulk_erase()

    # Send a serial command
    def send(self, com):
        self.ser.write(com)
        sleep(0.01)

    # Erase the PIC
    def bulk_erase(self):
        self.assert_d("b")

    # Read the current word from the PIC
    def read(self):
        self.send("r")
        # Parse the binary response
        return int(self.get_response(), 2)

    # Write a word to the current locationat the PIC
    def write(self, val):
        self.send("w")
        # Send the numer 0-padded to a 14-bit binary number
        self.assert_d("{0:014b}".format(val))

    # Increment the program counter
    def increment(self):
        # Actually increment the program counter
        self.assert_d("i")
        # Keep track of the increment
        self.pc += 1

    # Resset the PIC
    def reset(self):
        # Reset the program counter
        self.pc = 0
        # Reset the PIC
        self.assert_d("R")

    # Move from programming memory into config memory
    def enter_config(self):
        self.pc = 0x2000
        self.assert_d("c")

    # Go to the config word (0x2007)
    def goto_config_word(self):
        # Go into config memory
        self.enter_config()

        # Increment the PC self times
        for i in range(7): self.increment()
        assert self.pc == 0x2007

    # Write a hex file to the pic
    def write_file(self, hf):
        # Check we're at the start of the memory
        assert self.pc == 0

        pbar = ProgressBar()
        # For each location in the hex file, up to the maximum
        # location
        for i in pbar(range(0, hf.last_loc(self.max_location) + 1)):
            # Get the value for this location from the hex file
            val = hf.get(i)
            if val == 0xffff:
                # If the value is the default (from bulk reset),
                # save time by incrementing
                self.increment()
                continue

            # Write the value to memory
            self.write(val)
            # Increment the program counter
            self.increment()

        # Reset the PIC ready to write/verify/etc from PC=0
        self.reset()

    # Verify the program memory has the same contents as a hex file
    def verify_file(self, hf):
        # Check we're at the start of the memory
        assert self.pc == 0

        pbar = ProgressBar()
        # For each location in the hex file, up to the maximum
        # location
        for i in pbar(range(0, hf.last_loc(self.max_location) + 1)):
            # Get the value that should be at this location
            val = hf.get(i)
            # Providing the value isn't the default, check it's there
            if val != 0xffff: assert self.read() == val
            # Increment the program counter
            self.increment()

        # Reset the PIC ready to write/verify/etc from PC=0
        self.reset()

    # Write and verify the configuration word from a hex file
    def write_verify_config_file(self, hf):
        # Get the configuration word
        cword = hf.config_word()
        
        # Move the pic PC to the config word
        self.goto_config_word()
       
        # Write the config word, and verify it's been written
        self.write(cword)
        assert self.read() == cword

        # Reset the PIC back to PC=0
        self.reset()

    # Execute the program on the PIC
    def execute(self):
        self.assert_d("x")

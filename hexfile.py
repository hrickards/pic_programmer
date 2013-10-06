from intelhex import IntelHex16bit

# Represents a  Microchip-style hex file
class HexFile:
    def __init__(self, filename):
        self.ih = IntelHex16bit()
        self.ih.fromfile(filename, format='hex')

    # Return the last location less than default
    def last_loc(self, default):
        return max(filter(lambda x: x <= default, self.ih.todict().keys()))

    # Get the word at location i
    def get(self, i):
        return self.ih[i]

    # Get the config word
    def config_word(self):
        # Config word at 0x2007
        return self.ih[0x2007]

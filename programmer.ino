/*
 * PIC 16F627A Programmer
 * Author: Harry Rickards
 *
 * Provides a serial interface to program a PIC 16F627A microcontroller.
 * May work with other chips that implement the same programming interface (628A/648A).
 * Created because I couldn't get Soranne's project to work (I think because of the timings)
 * Currently only High Voltage Programming is supported
 *
 * Heavily based upon Soranne's project (http://forum.arduino.cc/index.php?topic=92929.0)
 * but the serial interface is NOT compatible with Soranne's client-side program. Shouldn't be
 * too hard to modify it to be so though.
 *
 * b = Erase all program memory on chip (bulk erase)
 * c = Move program counter to config memory (R to reset back to program memory)
 * w = Write next word from serial to current program location
 * r = Read word at current location and print over serial
 * x = Put PIC into execution mode & run current program (R to reset back to programming mode)
 * i = Increment program counter
 * R = Reset PIC back to program memory, PC=0 & programming mode (same as on initial startup)
 */

// Connected to RB6 on the microcontroller
#define CLOCK 6
// Connected to RB7
#define DATA 5
// Connected to VDD
#define VDD 8
// Connected to the MOSFET switching the 12V supply
#define TWELVEV 2
// Connected to MCLR/VPP through a diode (because of the 12V going into VDD as well)
#define FIVEV 3
// Connected to RB4/PGM
#define PGM 7

void setup() {
  // Init a serial connection
  Serial.begin(9600);
  
  // Get the pic ready for programming
  InitPic();
}

// Get the pic ready for programming
void InitPic() {
  // Put all pins into output mode
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(VDD, OUTPUT);
  pinMode(PGM, OUTPUT);
  pinMode(FIVEV, OUTPUT);
  pinMode(TWELVEV, OUTPUT);
  
  // Go into HVP mode
  digitalWrite(DATA, LOW);
  digitalWrite(CLOCK, LOW);
  digitalWrite(VDD, HIGH);
  digitalWrite(PGM, LOW);
  SwitchPower(12);
  
  //THlLD0=5
  delayMicroseconds(5);
}

// Turn the pic off
void StopPic() {
  // Put all pins into output mode
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(VDD, OUTPUT);
  pinMode(PGM, OUTPUT);
  pinMode(FIVEV, OUTPUT);
  pinMode(TWELVEV, OUTPUT);
  
  // Switch everything low, and wait 50ms to be sure
  digitalWrite(DATA, LOW);
  digitalWrite(CLOCK, LOW);
  digitalWrite(VDD, LOW);
  digitalWrite(PGM, LOW);
  SwitchPower(0);
  delay(50);
}

void loop() {
  char command = '\0';
  
  if (Serial.available() > 0) {
    // Read the command from serial
    command = Serial.read();
    
    switch(command) {
      case 'b':
        // Erase all program memory on the chip
        BulkErase();
        break;
        
      case 'c':
        // Go to config memory (cf program memory)
        EnterConfig();
        break;
        
      case 'w':
        // Write the next 14 serial chars to the current location
        WriteSerialWord();
        break;
        
      case 'r':
        // Read the current word and print it over serial
        ReadAndPrint();
        break;
        
      case 'x':
        // Run the program on the pic
        Execute();
        break;
        
      case 'i':
        // Increment the program counter
        IncrementAddress();
        break;
        
      case 'R':
        // Reset the PIC
        StopPic();
        InitPic();
        Serial.println();
        break;
    }
  }
}

// Run the program on the PIC
void Execute() {
  // Put pins into input mode --- we're not actually using them for input, but
  // the pic may be outputting things on them
  pinMode(CLOCK, INPUT);
  pinMode(DATA, INPUT);
  pinMode(VDD, OUTPUT);
  pinMode(PGM, INPUT);
  pinMode(FIVEV, OUTPUT);
  pinMode(TWELVEV, OUTPUT);
  
  SwitchPower(5);
  digitalWrite(VDD, HIGH);
  Serial.println();
}

// Switch the power going into MCLR/VPP to the passed voltage
// Supports 0/5/12V
void SwitchPower(int voltage) {
  if (voltage == 12) {
    // 12V
    // Don't provide 5V
    // Provide 12V (low on TWELVEV switches MOSFET on)
    digitalWrite(FIVEV, LOW);
    digitalWrite(TWELVEV, LOW);
  } else if (voltage == 0) {
    // 0V
    // Don't provide 5V
    // Don't provide 12V (high on TWELVEV switches MOSFET off)
    digitalWrite(FIVEV, LOW);
    digitalWrite(TWELVEV, HIGH);
  } else {
    // 5V
    // Provide 5V
    // Don't provide 12V (high on TWELVEV switches MOSFET off)
    digitalWrite(FIVEV, HIGH);
    digitalWrite(TWELVEV, HIGH);
  }   
}

// Move to config memory (cf program memory)
void EnterConfig() {
  WriteBit('0');
  WriteBit('0');
  WriteBit('0');
  WriteBit('0');
  WriteBit('0');
  WriteBit('0');
  
  
  // TDLY2
  delayMicroseconds(1);
  
  WriteBit('0');
  for(int i=13;i>=0;i--) {
    WriteBit('0');
  }
  WriteBit('0');
  
  // TDLY2
  delayMicroseconds(1);
  Serial.println();
}

// Write the next 14 serial chars to the current location
void WriteSerialWord() {
  char data[14];
  
  // Get the word to write
  for(int i=0; i<14; i++) {
    while(Serial.available() == 0) { }  
    data[i] = Serial.read();
  }
  
  // Write the word to memory
  WriteWord(data);
  Serial.println();
}

// Increment the program counter
void IncrementAddress() {
  WriteBit('0');
  WriteBit('1');
  WriteBit('1');
  WriteBit('0');
  WriteBit('0');
  WriteBit('1');
  
  Serial.println();
  
  // TDLY2
  delayMicroseconds(1);
}

// Read the word at the current location and print it out over serial
void ReadAndPrint() {
  char valuer[16];
  
  // Read data
  ReadData(valuer);
  for(int i=0; i<14; i++) {
    Serial.print(valuer[i]);
  }
  Serial.println(); 
  
  // TDLY2
  delayMicroseconds(1);
}

// Erase all program memory
void BulkErase() {
  // Load 11111111111111
  char allones[] = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};
  LoadData(allones);
  // TDLY2
  delayMicroseconds(1);
  
  // Run the bulk erase command
  BulkEraseCommand();
  // TDLY2
  delayMicroseconds(1);
  
  Serial.println();
}  

// Write the passed word into memory
void WriteWord(char wword[]) {
  // Load the word
  LoadData(wword);
  // TDLY2
  delayMicroseconds(1);
  
  // Write the loaded word to memory
  WriteData();
  // TDLY2
  delayMicroseconds(1);
}

// Write the loaded word (LoadData) to memory
void WriteData() {
  WriteBit('0');
  WriteBit('0');
  WriteBit('0');
  WriteBit('1');
  WriteBit('0');
  WriteBit('0');
  
  //TPROG
  delay(4);
}
  

// Bulk erase the program memory
// According the programming interface, requires a word of all 1s to
// be loaded previously. Whether it erases the memory to a different value
// if a different word is loaded, I don't know.
void BulkEraseCommand() {
  WriteBit('1');
  WriteBit('0');
  WriteBit('0');
  WriteBit('1');
  WriteBit('0');
  WriteBit('0');
  
  // TERA
  delay(6);
}

// Load the passed data
void LoadData(char valuer[]) {
  // Load instruction
  WriteBit('0');
  WriteBit('1');
  WriteBit('0');
  WriteBit('0');
  WriteBit('0');
  WriteBit('0');
  
  // TDLY2
  delayMicroseconds(1);
  
  // Pre bit of 0
  WriteBit('0');
  // Actual data, from LSB
  for(int i=13;i>=0;i--) {
    WriteBit(valuer[i]);
  }
  // Post bit of 0
  WriteBit('0');
}

// Read a word from the current location
void ReadData(char valuer[]) {
  // Read instruction
  WriteBit('0');
  WriteBit('0');
  WriteBit('1');
  WriteBit('0');
  WriteBit('0');
  WriteBit('1');
  
  
  // TDLY2
  delayMicroseconds(1);
  
  // Read pre bit of 0
  ReadBit();
  // Data comes through on DATA pin
  pinMode(DATA, INPUT);
  
  // Read the actual word
  for(int i=13; i >=0; i--) {
    valuer[i] = ReadBit();
  }
  
  // Put DATA pin ack to output
  pinMode(DATA, OUTPUT);
  // Read post bit of 0
  ReadBit();
}

// Write a bit
void WriteBit(char b) {
  // Clock: 3us high, 3us low (various <1us minimum times)
  
  // Clock high
  digitalWrite(CLOCK, HIGH);
  // Data high if the bit is a 1
  if (b != '0') { digitalWrite(DATA, HIGH); }
  delayMicroseconds(3);
  
  // Clock low
  digitalWrite(CLOCK, LOW);
  // Data the same
  delayMicroseconds(3);
  // Data low for next bit
  digitalWrite(DATA, LOW);
}


// Read a bit
byte ReadBit() {
  // Read the bit into here
  byte valuer = 0;
  
  // Clock: 3us high, 3us low. Longer than 80ns TDLY3 before reading.
  
  // Clock high
  digitalWrite(CLOCK, HIGH);
  delayMicroseconds(3);
  
  // Just before the falling edge of the clock, read the data from DATA
  if (digitalRead(DATA) == HIGH) {
    valuer = '1';
  } else {
    valuer = '0';
  }
  
  // Clock low
  digitalWrite(CLOCK, LOW);
  delayMicroseconds(3);
  
  return valuer;
}

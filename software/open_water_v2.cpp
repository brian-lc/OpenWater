STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

/* EZO Circuit Serial Default
 9600 bps
 8 data bits
 1 stop bit
 no parity
 no flow control 
*/

// EZO LED notes
// If LED has not been disabled the comms mode is reported as:
// LED is solid Green --> UART mode
// LED is solid Blue ---> I2C mode


int incomingByte = 0;

// For Atlas EZO comms
int orpAddr = 0x62; // ORP EZO I2C default address
int phAddr = 0x63; // PH EZO I2C default address
int tempAddr = 0x66; // Temp EZO I2C default address
int cmdDelay = 1100;  // docs say wait 1 second (or 600ms for some) after read command. Giving a 10% buffer.

// For Sparkfun Si7021 board comms
int tmpHumAddr = 0x40; //Sparkfun Si7021 I2C default address
float reading = 0.0;
int mCode = 0;

String cmdEvent = "CMD_RESP";
String dbgEvent = "DEBUG";

// Lb_ for librato
String tempEvent = "Lib_TEMP";
String phEvent = "Lib_PH";
String orpEvent = "Lib_ORP";

// Internal temp/hum measurments
String iTempEvent = "Lib_ITEMP";
String iHumEvent = "Lib_IHUM";

// Sleep flags
// EZO low power sleep
bool ezoSleep = true;

void setup() {

  // Setting up Serial to talk to the local EZO chip
  //Serial1.begin(9600, SERIAL_8N1);
  
  // Setting I2C (default 100kHz) to talk to remote EZO chips
  Wire.begin();

  // delay waiting for EZO boot
  delay(5000);
  
  // Factory reset (if needed)
  //serialCommand("Factory");
  //wireCommand(orpAddr, "Factory");
  
  // Put a device back in UART mode
  //wireCommand(tempAddr, "Baud,9600");
  //delay(300);
  //wireReadPublish("BaudCMD", tempAddr);
  
  // --Only for the EZO temp module--
  // Turn off continuous mode where
  // the ezo circuit sends readings non-stop
  // [note: LED blinks green each time a reading is sent]
  wireCommand(tempAddr, "C,0");
  wireReadPublish(cmdEvent, tempAddr);
  delay(cmdDelay);
}

void loop() {
  // ---- probe readings -----
  //Reading Temperature via I2C
  managedEZOReading(tempEvent, tempAddr);
 
  // Reading ORP via I2C
  managedEZOReading(orpEvent, orpAddr);

  // Reading pH via I2C
  managedEZOReading(phEvent, phAddr);

  // ---- internal sensor readings -----
  // Slightly different dance for the Si7021 data
  // Reading iternal chassis temp
  mCode = wireTakeMeasure(tmpHumAddr, 0xE3); //Command for Temp 
  reading = (175.25*mCode/65536)-46.85;
  ppub(iTempEvent, String::format("%.3f", reading));
  
  // Reading internal chassis humidity
  mCode = wireTakeMeasure(tmpHumAddr, 0xE5); //Command for Relative Humidity
  reading = (125.0*mCode/65536)-6;
  ppub(iHumEvent, String::format("%.3f", reading));
  
  // --- sleeping ------
  //System.sleep(SLEEP_MODE_DEEP, 300); //Sleep for 5min
  delay(6000); //chill for a min
}

// Manages the EZO circuit lifecyle: wake, read, sleep, publish
// Takes the name of event to publish (string) and device addr I2C (int)
void managedEZOReading(String eventName, int ezoAddr) {
  if (ezoSleep) {
    wakeFromSleep(ezoAddr);
  }
  
  wireCommand(ezoAddr, "R");
  delay(cmdDelay);
  wireReadPublish(eventName, ezoAddr);
  
  if (ezoSleep){
    putToSleep(ezoAddr);
  }
}

void putToSleep(int addr){
  wireCommand(addr, "Sleep");
}

void wakeFromSleep(int addr){
  // Any command will wake the device
  wireCommand(addr, "R");
  delay(cmdDelay);
}

// Reads the wire while data is available. Extracts the reading
// and formats it into a string as the return value.
String wireGetFormattedReading(int addr){
  char respBuffer[32]; // Allocated response buffer 
  int rIdx = 0;
  double reading = 0.0;
  Wire.requestFrom(addr, 8); //I2C reading is "no longer than 8 bytes"
  
  while(Wire.available()) {
    respBuffer[rIdx] = Wire.read();
    rIdx++;
  }
  if(rIdx > 0){
    respBuffer[rIdx] = '\0'; // Terminate the buffer with null char
    reading = getDouble(respBuffer); // Extract double from char buffer
    return String::format("%.3f", reading); //Format reading as string
  }
  else {
    return "???";
  }
}

// Shorthand method to read/publish
void wireReadPublish(String eventName, int addr) {
  String reading = wireGetFormattedReading(addr);
  ppub(eventName, reading);
}

// Added specifically to read the Sparkfun sensor from I2C
// Was being lazy so I lifted it from 
// https://raw.githubusercontent.com/sparkfun/Si7021_Breakout/master/Libraries/Arduino/Si7021/src/SparkFun_Si7021_Breakout_Library.cpp
// And made some slight modifications...
int wireTakeMeasure(int addr, int command){
    
    int nBytes = 3; // fresh readings are 3 bytes long
    Wire.requestFrom(addr, nBytes);
    Wire.beginTransmission(addr);
    Wire.write(command);
    Wire.endTransmission();
    // When not using clock stretching (*_NOHOLD commands) delay here
    // is needed to wait for the measurement.
    // According to datasheet the max. conversion time is ~22ms
    delay(100);
    Wire.requestFrom(addr, nBytes);
        //Wait for data
    int counter = 0;
    while (Wire.available() < nBytes){
      delay(1);
      counter ++;
      if (counter >100){
        // Timeout: Sensor did not return any data
        return 100;
      }
    }

    unsigned int msb = Wire.read();
    unsigned int lsb = Wire.read();
    // Clear the last to bits of LSB to 00.
    // According to datasheet LSB of RH is always xxxxxx10
    lsb &= 0xFC;
    int mesurment = msb << 8 | lsb;
    
    return mesurment;
}


// Optionally used during debugging/prototyping
// The EZO sensors can do serial from their TX RX pins
// This function reads from serial1 on the photon.
// NOTE: Serial1 must be initialied in setup()
void serialReadPublish(String dataName){
  char respBuffer[32]; // Allocated response buffer 
  int rIdx = 0;
  while(Serial1.available() < 0){
    // After issuing Command, EZO circuit takes 1 second to respond
    delay(10);
  }
  while(Serial1.available() > 0){
    incomingByte = Serial1.read();   
    respBuffer[rIdx] = (char) incomingByte;
    rIdx++;
  }
  if(rIdx > 0){
    respBuffer[rIdx] = '-';
    // Transmit buffer
    ppub(dataName, String(respBuffer));
  }
}

// serialCommand - Serial1Print a command string
void serialCommand(String cmd){
  //dbug("Serial<" + cmd + ">");
  Serial1.print(cmd + "\r"); // Serial commands must terminate with CR char
}

// wireCommand - I2C send command to device
void wireCommand(int addr, String cmd){
  //dbug("Wire<" + String(addr) + ":" + cmd + ">");
  Wire.beginTransmission(addr);
  Wire.write(cmd + "\0"); // I2C to EZO must terminate with the NULL char (ASCII 0)
  Wire.endTransmission();
}

void dbug(String msg){
  ppub(dbgEvent, "DEBUG:" + msg);
}

// ppub - ParticlePublish
void ppub(String dataName, String dataVal){
  Particle.publish(dataName, dataVal, 60, PRIVATE);
}

// From http://stackoverflow.com/questions/13698449/extract-double-from-a-string-in-c
double getDouble(const char *str)
{
    /* First skip non-digit characters */
    /* Special case to handle negative numbers and the `+` sign */
    while (*str && !(isdigit(*str) || ((*str == '-' || *str == '+') && isdigit(*(str + 1)))))
        str++;

    /* The parse to a double */
    return strtod(str, NULL);
}
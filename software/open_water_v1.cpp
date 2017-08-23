//STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

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

int orpAddr = 0x62;
int phAddr = 0x63;
int tempAddr = 0x66;
char respBuffer[255];
int rIdx = 0;


String cmdEvent = "CMD_response";
String tempEvent = "TEMP_reading";
String phEvent = "PH_reading";
String orpEvent = "ORP_reading";
String dbgEvent = "DEBUG";

void setup() {

  // Setting up Serial to talk to the local EZO chip
  Serial1.begin(9600, SERIAL_8N1);
  
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
  
  // Turn device UART mode device into I2C mode
  // Make sure you flash the device with this commented OUT
  // before you plug back in the device you will run as serial!
  //serialCommand("I2C,99"); // Addr is 99 (0x63)
  //serialReadPublish(cmdEvent);
  
  // Turn off continuous mode where
  // the ezo circuit sends readings non-stop
  // [note: LED blinks green each time a reading is sent]
  serialCommand("C,0");
  serialReadPublish(cmdEvent);
  
  delay(1000);
}

void loop() {
  // Executing a reading
  //Reading Temperature via Serial
  serialCommand("R");
  serialReadPublish(tempEvent);
  
  // Reading ORP via I2C
  wireCommand(orpAddr, "R");
  delay(1100); // docs say wait 1 second after read command (giving a 10% buffer)
  wireReadPublish(orpEvent, orpAddr);
  
  // Reading pH via I2C
  wireCommand(phAddr, "R");
  delay(1100);
  wireReadPublish(phEvent, phAddr);
  
  delay(5000);
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
  Wire.write(cmd + "\0"); // I2C must terminate with the NULL char (ASCII 0)
  Wire.endTransmission();
}

void dbug(String msg){
  ppub(dbgEvent, "DEBUG:" + msg);
}

// ppub - ParticlePublish
void ppub(String dataName, String dataVal){
  Particle.publish(dataName, dataVal);
}

void wireReadPublish(String dataName, int addr){
  rIdx = 0;
  Wire.requestFrom(addr, 8); //I2C reading is "no longer than 8 bytes"
  
  while(Wire.available()) {
    respBuffer[rIdx] = Wire.read();
    rIdx++;
  }
  if(rIdx > 0){
    respBuffer[rIdx] = '-'; 
    // Transmit buffer
    ppub(dataName, String(respBuffer));
    // Reset buffer position
    rIdx = 0; 
  }
}

void serialReadPublish(String dataName){
  rIdx = 0;
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
    // Reset buffer position
    rIdx = 0;
  }
}
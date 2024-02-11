/* This sketch writes 256 bytes into a simple EEPROM device 
* like the m24c02. It also works on the m24c08 but it only writes
* 265 bytes into the first page. Writing more than 256 bytes (multiple
* pages) needs modifications in the code.
* Since to write multiple pages, basically the I2C address changes
* the modifications should be simple. Just initialize mutiple arrays, 
* one for each page and just copy the for loop in the setup part for
* as many pages as you need, modifying the I2C address every time.
*
* Below is the I2C address structure of the m24c08. A9 and A8 are 
* basically the page address as part of the I2C address
*
* b7 b6 b5 b4 b3 b2 b1 b0
* 1  0  1  0  E2 A9 A8 RW
*/

#include <Wire.h>
#include <Adafruit_SleepyDog.h>

static char buff[16];
unsigned char EEDID[256] = {
  // Offset 0x00000000 to 0x000000FF
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x50, 0x87, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0x22, 0x01, 0x04, 0xA2, 0x18, 0x05, 0x78, 0x07, 0xEE,
  0x95, 0xA3, 0x54, 0x4C, 0x99, 0x26, 0x0F, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x88, 0x08, 0xD8, 0xA0, 0x40, 0xF0, 0x14, 0x00, 0x30, 0x20, 0x3A,
  0x00, 0x63, 0xC8, 0x10, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x17,
  0xF0, 0x0F, 0xFF, 0x02, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,
  0x00, 0x00, 0xFC, 0x00, 0x31, 0x32, 0x34, 0x30, 0x78, 0x32, 0x34, 0x30, 0x20,
  0x43, 0x56, 0x54, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x90, 0x02, 0x03,
  0x14, 0x00, 0x67, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x01, 0x05, 0x67, 0xD8, 0x5D,
  0xC4, 0x01, 0x05, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85
};

/* Function to intentionally trigger the watchdog
* This is a workaround solution for resetting the device
*/
void harakiri(){
  Serial.println("O death, where art thou—");
  while(1){}
  //digitalWrite(D8, 0);
}

void read16regs(uint8_t addr, uint8_t reg){
  uint8_t stat = 1;
  uint8_t i;
  for(i = 0; i < 16; i++){
    buff[i] = 0;
    i++;
  }
  Wire.beginTransmission(addr); //begin write transmission to device addr
  Wire.write(reg); //write the register we wish to read out from
  stat = Wire.endTransmission(false); ////We send everything to the bus without stop condition
  if (stat != 0) {
      harakiri();
    }
  Wire.requestFrom(addr, 16); //second start condition and read 5 bytes
   
  i=0;
  while(Wire.available()){
    buff[i] = Wire.read();
    i++;
  }
}

void report16regstatus(uint8_t I2Cadr, uint8_t regadr){
  Serial.print(regadr, HEX);
  Serial.print(": ");
  read16regs(I2Cadr, regadr);
  uint16_t count = 0;
  while(count < 16){
    Serial.print(buff[count], HEX);
    Serial.print(" ");
    count++;
  }
  Serial.print("\n\r");
}
void writeEDID(){
  Serial.println("Programming EDID");
  uint8_t i, j;
  uint8_t startaddr = 0x00;
  uint8_t status = 1;
  //Writing to I2C address 0x50
  for(j = 0; j < 16; j++){
    Wire.beginTransmission(0x50); //We initiate the write sequence to the right I2C address
    Wire.write(startaddr); //First we write the register address we wish to write to
    for(i = 0; i < 16; i++){
      Wire.write(EEDID[startaddr+i]);
    } //We write as many consecutive registers as required
    startaddr += i;
    status = Wire.endTransmission(); //We send everything to the bus
    if (status != 0) {
      harakiri();
    }
    delay(10);
  }
}
/* Initialisation of individual hardware blocks
* They are grouped by function and not by specific hardware block that we communciate with
* for example for the IO expander initialisation we first need to activate the enable pin 
* in the deserialiser and then configure the IO expander itself
*/

/*function that gets run firs, once on every power-up*/
void setup() {
  /*This delay is helpful during debug to give the serial terminal time to connect to the device.*/
  //delay(5000);
  //pinMode(10, INPUT);
  //attachInterrupt(digitalPinToInterrupt(10), writeEDID, FALLING);
  uint8_t countdownMS = Watchdog.enable(10000);
  Serial.print("Enabled the watchdog with max countdown of ");
  Serial.print(countdownMS, DEC);
  Serial.println(" milliseconds!");
  Serial.begin(115200);
  Wire.begin();
  delay(1000);
  writeEDID();
}

/*infinite loop*/
void loop() {
  Watchdog.reset();
  delay(1000);
  uint8_t startaddr = 0x00;
  uint8_t stepsize = 16;
  Serial.println("\n\rEDID content:");
  for(int j = 0; j < stepsize; j++){
    report16regstatus(0x50, startaddr);
    startaddr += stepsize;
  }
  Serial.println("---------------------------------------------");
  delay(4000);
}
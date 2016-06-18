/* 
  *  Use the I2C bus with EEPROM 24LC64 
  *  Sketch:    eeprom.pde
  *  
  *  Author: hkhijhe
  *  Date: 01/10/2010
  * 
  *   
  */

  #include <Wire.h> //I2C library
  
  struct LocationItem
  {
    float lat;
    float lng;
//    uint8_t hour;
//    uint8_t minute;
//    uint8_t seoncd;
  };

  int state = 0;
  bool promptShow = false;

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data )
{
  int rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));    // Address High Byte
  Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
  Wire.write(rdata);
  Wire.endTransmission();
}

// Address is a page address, 6-bit (63). More and end will wrap around
// But data can be maximum of 28 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page
( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length )
{
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddresspage >> 8)); // Address High Byte
  Wire.write((int)(eeaddresspage & 0xFF)); // Address Low Byte
  byte c;
  for ( c = 0; c < length; c++)
    Wire.write(data[c]);
  Wire.endTransmission();
  delay(10);                           // need some delay
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress )
{
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));    // Address High Byte
  Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

// should not read more than 28 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length )
{
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));    // Address High Byte
  Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,length);
  //int c = 0;
  for ( int c = 0; c < length; c++ )
    if (Wire.available()) buffer[c] = Wire.read();
}



  void setup() 
  {
    char somedata[] = "this is data from the eeprom"; // data to write
    Wire.begin(); // initialise the connection
    Serial.begin(9600);
    //i2c_eeprom_write_page(0x50, 0, (byte *)somedata, sizeof(somedata)); // write to EEPROM 
    //delay(10); //add a small delay

    Serial.println("Ready");
  }

  void loop() 
  {
    if (!promptShow)
    {
       Serial.print("//"); Serial.print(state); Serial.println(" >> ");
       promptShow = true; 
    }
    
    if (Serial.available())
    {
       char inChar = (char)Serial.read();
       Serial.print(">>>> "); Serial.println(inChar);
       state = inChar - '0';
    } 

    if (state == 0)
    {
    }
    if (state == 1)
    {
      int i = 0;
      i2c_eeprom_write_page(0x50, 0, (byte*)&i, sizeof(i));
      
      for (int idx = 0; idx < i; idx++)
      {
        int v = idx * 2 + 32000;
        i2c_eeprom_write_page(0x50, 2 + idx*2, (byte*)&v, sizeof(v));
      }
      Serial.println("memory written");
      
      state = 0;
    }
    if (state == 2)
    {
      Serial.print("sizeof(struct LocationItem)="); Serial.println(sizeof(struct LocationItem));
      
      int i = 0;
      i2c_eeprom_read_buffer(0x50, 0, (byte*)&i, sizeof(i));
      Serial.print("read i="); Serial.println(i);
      
      for (int idx = 0; idx < i; idx++)
      {
        struct LocationItem v;
        i2c_eeprom_read_buffer(0x50, 2 + idx * sizeof(struct LocationItem), (byte*)&v, sizeof(v));
        Serial.print("write ["); Serial.print(idx); Serial.print("]="); 
        Serial.print(v.lng); Serial.print(","); Serial.println(v.lat); //Serial.print(","); Serial.println(v.hour);
      }
      
      state = 0;
    }
    if (state == 3)
    {
      // erase
      int i = 0;
      i2c_eeprom_write_page(0x50, 0, (byte*)&i, sizeof(i));
      state = 0;
    }
    if (state == 4)
    {
       // add
      int i = 0;
      i2c_eeprom_read_buffer(0x50, 0, (byte*)&i, sizeof(i));
      Serial.print("read i="); Serial.println(i);

      struct LocationItem v;
      v.lat = 100 + i * 4;
      v.lng = 400 + i * 4;
//      v.hour = 50 + i;
      i2c_eeprom_write_page(0x50, 2 + i*sizeof(struct LocationItem), (byte*)&v, sizeof(v));
      Serial.print("write ["); Serial.print(i); Serial.print("]="); 
      Serial.print(v.lng); Serial.print(","); Serial.println(v.lat);
      
      i++;
      i2c_eeprom_write_page(0x50, 0, (byte*)&i, sizeof(i));
      
      state = 0;
    }
    
    /*
    int addr=0; //first address
    byte b = i2c_eeprom_read_byte(0x50, 0); // access the first address from the memory

    while (b!=0) 
    {
      Serial.print((char)b); //print content to serial port
      addr++; //increase address
      b = i2c_eeprom_read_byte(0x50, addr); //access an address from the memory
    }
    Serial.println(" ");
    */
    delay(500);

  }

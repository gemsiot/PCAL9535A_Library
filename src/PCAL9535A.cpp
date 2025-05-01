/******************************************************************************
PCAL9535A.cpp
Interface for PCAL9535A 16 bit IO expander
Bobby Schulz @ GEMS IoT
04/25/2022
https://github.com/gemsiot/PCAL9535A_Library

Based on MCP23008 library developed by Northern Widget

Allows control of all aspects of the control of the IO expander 

1.0.0

"All existing things are really one"
-Zhuangzi

Distributed as-is; no warranty is given.

© 2023 Regents of the University of Minnesota. All rights reserved.
******************************************************************************/

// #include <Arduino.h>
#include <PCAL9535A.h>
// #include <Wire.h>

PCAL9535A::PCAL9535A(int _ADR)
{
  ADR = _ADR; //FIX ADR!
}


int PCAL9535A::begin(void)  //FIX! Combine interrupt lines be default!
{
  #if defined(ARDUINO) && ARDUINO >= 100 
    Wire.begin();
  #elif defined(PARTICLE)
    if(!Wire.isEnabled()) Wire.begin(); //Only initialize I2C if not done already //INCLUDE FOR USE WITH PARTICLE 
  #endif
  // Wire.setClock(400000); //DEBUG!
  // PinModeConf[0] = 0xFF; //Default to all inputs //FIX make cleaner
  // PinModeConf[1] = 0xFF; 

  for(int i = 0; i < 2; i++) { //Load current state
    PinModeConf[i] = readByte(DIRA + i); //Read in direction regs
    PortState[i] = readByte(LATA + i); //Read in LAT regs
    PinPolarityConfig[i] = readByte(POLA + i); //Read in input polarity regs
    PullUpConf[i] = readByte(PULLUPA + i); //Read in pullup regs
    PullUpEnable[i] = readByte(PULLUPENA + i); //Read in pullup enable regs
    InterruptMask[i] = readByte(INTMASKA + i); //Read in interrupt enable regs
    // DefaultValConf[i] = readByte(DEFVALA + i); //Read in default interrupt value registers
    // InterruptTypeConf[i] = readByte(INTCONA + i); //Read in interrupt mode registers
    InputLatch[i] = readByte(LATCHA + i); //Read in latch regs
    CurrentControl[i] = readByte(DRIVEA + i); //Read in current control regs for port A
    CurrentControl[i + 2] = readByte(DRIVEB + i); //Read in current control regs for port B
  }
  PortOutput = readByte(PORTOUTPUT);

  Wire.beginTransmission(ADR); //Test if device present 
  int Error = Wire.endTransmission();
  if(Error != 0) return -1;
  else return Error;
}


int PCAL9535A::pinMode(int Pin, uint8_t PinType, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1;  //Fail if pin out of range
  }

  if(PinType == INPUT)
  {
    PinModeConf[Port] = PinModeConf[Port] | (0x01 << Pin); //Set bit for input
    PullUpEnable[Port] = PullUpEnable[Port] & ~(0x01 << Pin);  //Clear bit for pullup 
    // Serial.print("Reg 0x03 = ");  //DEBUG!
    // Serial.println(PinModeConf, HEX); //DEBUG!
    setDirection(PinModeConf[Port], Port);
    setPullupEN(PullUpEnable[Port], Port);
    // setPullup(PullUpConf[Port], Port);
    return 1;
  }

  else if(PinType == OUTPUT)
  {
    PinModeConf[Port] = PinModeConf[Port] & ~(0x01 << Pin); //Clear bit for output
    // PullUpConf[Port] = PullUpConf[Port] | (0x01 << Pin);  //Set pullup bit to allow for "push-pull" operation
    // Serial.print("Reg 0x03 = ");  //DEBUG!
    // Serial.println(PinModeConf[Port], HEX); //DEBUG!
    // Serial.println(PullUpConf[Port], HEX); //DEBUG!
    setDirection(PinModeConf[Port], Port);
    // setPullup(PullUpConf[Port], Port);
    return 0;
  }

  else if(PinType == INPUT_PULLDOWN)
  {
    PinModeConf[Port] = PinModeConf[Port] | (0x01 << Pin); //Set bit for input
    PullUpConf[Port] = PullUpConf[Port] & ~(0x01 << Pin);  //Clear bit for pulldown  
    PullUpEnable[Port] = PullUpEnable[Port] | (0x01 << Pin);  //Set bit for pullup enable 
    // Serial.print("Reg 0x03 = ");  //DEBUG!
    // Serial.println(PinModeConf, HEX); //DEBUG!
    setDirection(PinModeConf[Port], Port); 
    setPullupDir(PullUpConf[Port], Port);
    setPullupEN(PullUpEnable[Port], Port);
    return 3;
  }
  // else if(PinType == OPEN_DRAIN)
  // {
  //   PinModeConf[Port] = PinModeConf[Port] & ~(0x01 << Pin); //Clear bit for output
  //   PullUpConf[Port] = PullUpConf[Port] & ~(0x01 << Pin);  //Clear pullup bit to allow for open drain operation
  //   // Serial.print("Reg 0x03 = ");  //DEBUG!
  //   // Serial.println(PinModeConf[Port], HEX); //DEBUG!
  //   setDirection(PinModeConf[Port], Port);
  //   setPullup(PullUpConf[Port], Port);
  //   return 0;
  // }

  else if(PinType == INPUT_PULLUP)
  {
    PinModeConf[Port] = PinModeConf[Port] | (0x01 << Pin); //Set bit for input
    PullUpConf[Port] = PullUpConf[Port] | (0x01 << Pin);  //Set bit for pullup  
    PullUpEnable[Port] = PullUpEnable[Port] | (0x01 << Pin);  //Set bit for pullup enable 
    // Serial.print("Reg 0x03 = ");  //DEBUG!
    // Serial.println(PinModeConf, HEX); //DEBUG!
    setDirection(PinModeConf[Port], Port); 
    setPullupDir(PullUpConf[Port], Port);
    setPullupEN(PullUpEnable[Port], Port);
    return 2;
  }
  else 
    return -1; //Fail if pin type not defined 
}

int PCAL9535A::pinMode(int Pin, uint8_t PinType)
{
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    return pinMode(Pin - 8, PinType, PORT_B); //Shift pin number, pass along to set port B
  }
  if(Pin <= 7) {
    return pinMode(Pin, PinType, PORT_A); //Pass along to set port A
  }
  return -1; //Fail is state is ill-defined
}

int PCAL9535A::digitalWrite(int Pin, bool State, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  uint8_t TempPort = PortState[Port];
  int Error = 0;
  bool OutputFault = false; 
  if(State == HIGH)
  {
    TempPort = TempPort | (0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!

  }
  else if(State == LOW)
  {
    TempPort = TempPort & ~(0x01 << Pin);
    // PortState[Port] = PortState[Port] & ~(0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    // setPort(PortState[Port], Port);
    // return 0;
  }
  else 
    return -1; //Fail if state is ill-defined
  Error = setPort(TempPort, Port);
  if(systemSafe >= SAFE3) { //If running in aggressive safe mode, test state of pin to make sure write was correct 
    if(digitalRead(Pin, Port) != State && Error == 0) { //Only set if no comm error
      OutputFault = true; //Set output fault flag
      
      setPort(PortState[Port], Port); //Revert port state 
    }
  }
  if(Error == 0 && OutputFault == false) {
    PortState[Port] = TempPort; //If no errors, copy over port status
    return State; //Return the 
  }
  // if(Error != 0) return Error; //Return error state if available
  // else return -1; //Otherwise return general fault
  return -1;
  
}

int PCAL9535A::digitalWrite(int Pin, bool State)
{
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    return digitalWrite(Pin - 8, State, PORT_B); //Shift pin number, pass along to set port B
  }
  if(Pin <= 7) {
    return digitalWrite(Pin, State, PORT_A); //Pass along to set port A
  }
  return -1; //Fail is state is ill-defined
}

int PCAL9535A::digitalRead(int Pin, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  return (readPort(Port) >> Pin) & 0x01; //Return selected bit of the port value
}

int PCAL9535A::digitalRead(int Pin)
{
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    return digitalRead(Pin - 8, PORT_B); //Shift pin number, pass along to set port B
  }
  if(Pin <= 7) {
    return digitalRead(Pin, PORT_A); //Pass along to set port A
  }
  return -1; //Fail is state is ill-defined
}

int PCAL9535A::pinSetDriveStrength(int Pin, DriveStrength State, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }
  uint8_t RegWriteVal = 0;
  switch(State) {
    // case DriveStrength::DEFAULT:
    //   RegWriteVal = 0b11; //Default to max drive, this is POR on chip
    //   break;
    case DriveStrength::HIGH:
      RegWriteVal = 0b11; //Set to max drive
      break;
    case DriveStrength::STANDARD:
      RegWriteVal = 0b00; //Use minimum drive as 'standard'
      break;
    default:
      RegWriteVal = 0b00; //Use minimum drive as 'standard'
      break;
  }
  bool Offset = floor(Pin/4); //Determine if the pin is in the first or second byte of the register 
  int Position = floor(Pin/2); //Determine the shifting position within the given register 
  CurrentControl[Port + Offset] = CurrentControl[Port + Offset] & ~(0b11 << 2*Position); //Clear the specified pair
  CurrentControl[Port + Offset] = CurrentControl[Port + Offset] | (RegWriteVal << 2*Position); //Clear the specified pair
  // Serial.print("Reg 0x01 = ");  //DEBUG!
  // Serial.println(Port, HEX); //DEBUG!
  Serial.print("Reg = "); //DEBUG!
  Serial.println(CurrentControl[Port + Offset], BIN); //DEBUG!
  Serial.print("Pos = "); //DEBUG!
  Serial.println(DRIVEA + Port + Offset, HEX); //DEBUG!
  Serial.print("Mask = "); //DEBUG!
  Serial.println(~(0x11 << 2*Position), HEX); //DEBUG!
  int Error = writeByte(DRIVEA + Port + Offset, CurrentControl[Port + Offset]);

  return Error;
}

int PCAL9535A::pinSetDriveStrength(int Pin, DriveStrength State)
{
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    return pinSetDriveStrength(Pin - 8, State, PORT_B); //Shift pin number, pass along to set port B
  }
  if(Pin <= 7) {
    return pinSetDriveStrength(Pin, State, PORT_A); //Pass along to set port A
  }
  return -1; //Fail is state is ill-defined
}

int PCAL9535A::setInterrupt(int Pin, bool State, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(State == ON)
  {
    InterruptMask[Port] = InterruptMask[Port] & ~(0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    writeByte(INTMASKA + Port, InterruptMask[Port]); //Write adjusted value back
    return 1;
  }
  else if(State == OFF)
  {
    InterruptMask[Port] = InterruptMask[Port] | (0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    writeByte(INTMASKA + Port, InterruptMask[Port]); //Write adjusted value back
    return 0;
  }
  else 
    return -1; //Fail if state is ill-defined
}

int PCAL9535A::setInterrupt(int Pin, bool State)
{
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    return setInterrupt(Pin - 8, State, PORT_B); //Shift pin number, pass along to set port B
  }
  if(Pin <= 7) {
    return setInterrupt(Pin, State, PORT_A); //Pass along to set port A
  }
  return -1; //Fail is state is ill-defined
}

int PCAL9535A::setLatch(int Pin, bool State, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(State == ON)
  {
    InputLatch[Port] = InputLatch[Port] | (0x01 << Pin);
    // InputLatch[Port] = InputLatch[Port] & ~(0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    writeByte(LATCHA + Port, InputLatch[Port]); //Write adjusted value back
    return 1;
  }
  else if(State == OFF)
  {
    // InputLatch[Port] = InputLatch[Port] | (0x01 << Pin);
    InputLatch[Port] = InputLatch[Port] & ~(0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    writeByte(LATCHA + Port, InputLatch[Port]); //Write adjusted value back
    return 0;
  }
  else 
    return -1; //Fail if state is ill-defined
}

int PCAL9535A::setLatch(int Pin, bool State)
{
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    return setLatch(Pin - 8, State, PORT_B); //Shift pin number, pass along to set port B
  }
  if(Pin <= 7) {
    return setLatch(Pin, State, PORT_A); //Pass along to set port A
  }
  return -1; //Fail is state is ill-defined
}

int PCAL9535A::setPort(int Config, bool Port) 
{
  // Wire.beginTransmission(ADR); // transmit to device with address ADR
  // Wire.write(LATA + Port);   //Send to output set register
  // Wire.write(Config);   
  // // Serial.println(LATA + Port, HEX); //DEBUG!
  // // Serial.println(Config, HEX); //DEBUG!
  // // Serial.print("\n\n"); //DEBUG!
  // return Wire.endTransmission();
  if(systemSafe >= SAFE2) {
    int ReadBackError = 0;
    uint8_t CurrentVal = readByte(LATA + Port, ReadBackError);
    if(CurrentVal != PortState[Port] && ReadBackError == 0x00) {
      globalError = globalError | MEMORY_ERROR; //Set memory error 
      // return -1; //Exit without completion //FIX!
      PortState[Port] = CurrentVal; //Update stored value
    }
  }
  int Error = writeByte(LATA + Port, Config);
  if(Error == 0x00) PortState[Port] = Config; //Write value back if no I2C error
  return Error; 
}

int PCAL9535A::setDirection(int Config, bool Port) 
{
  // Serial.println(Config, HEX); //DEBUG!
  // Wire.beginTransmission(ADR); // transmit to device with address ADR
  // Wire.write(DIRA + Port);        //Send to port configuration register
  // Wire.write(Config);              
  // // Serial.println(DIRA + Port, HEX); //DEBUG!
  // // Serial.print("\n\n"); //DEBUG!
  // return Wire.endTransmission();    // stop transmitting
  if(systemSafe >= SAFE2) {
    int ReadBackError = 0;
    uint8_t CurrentVal = readByte(DIRA + Port, ReadBackError);
    if(CurrentVal != PinModeConf[Port] && ReadBackError == 0x00) {
      globalError = globalError | MEMORY_ERROR; //Set memory error 
      // return -1; //Exit without completion //FIX!
      PinModeConf[Port] = CurrentVal; //Update stored value
    }
  }
  int Error = writeByte(DIRA + Port, Config);
  if(Error == 0x00) PinModeConf[Port] = Config; //Write value back if no I2C error
  return Error; 
}

int PCAL9535A::setPolarity(int Config, bool Port) 
{
  // Wire.beginTransmission(ADR); // transmit to device with address ADR
  // Wire.write(POLA + Port);        //Send to port configuration register
  // Wire.write(Config);              
  // return Wire.endTransmission();    // stop transmitting
  if(systemSafe >= SAFE2) {
    int ReadBackError = 0;
    uint8_t CurrentVal = readByte(POLA + Port, ReadBackError);
    if(CurrentVal != PinPolarityConfig[Port] && ReadBackError == 0x00) {
      globalError = globalError | MEMORY_ERROR; //Set memory error 
      // return -1; //Exit without completion //FIX!
      PinPolarityConfig[Port] = CurrentVal; //Update stored value
    }
  }
  int Error = writeByte(POLA + Port, Config);
  if(Error == 0x00) PinPolarityConfig[Port] = Config; //Write value back if no I2C error
  return Error; 
}

int PCAL9535A::setPullupDir(int Config, bool Port) 
{
  // Serial.println(Config, HEX); //DEBUG!
  // Wire.beginTransmission(ADR); // transmit to device with address ADR
  // Wire.write(PULLUPA + Port);        //Send to port configuration register
  // Wire.write(Config);         
  // return Wire.endTransmission();    // stop transmitting
  if(systemSafe >= SAFE2) {
    int ReadBackError = 0;
    uint8_t CurrentVal = readByte(PULLUPA + Port, ReadBackError);
    if(CurrentVal != PullUpConf[Port] && ReadBackError == 0x00) {
      globalError = globalError | MEMORY_ERROR; //Set memory error 
      // return -1; //Exit without completion //FIX!
      PullUpConf[Port] = CurrentVal; //Update stored value
    }
  }
  int Error = writeByte(PULLUPA + Port, Config);
  if(Error == 0x00) PullUpConf[Port] = Config; //Write value back if no I2C error
  return Error; 
}

int PCAL9535A::setPullupEN(int Config, bool Port) 
{
  // Serial.println(Config, HEX); //DEBUG!
  // Wire.beginTransmission(ADR); // transmit to device with address ADR
  // Wire.write(PULLUPENA + Port);        //Send to port configuration register
  // Wire.write(Config);         
  // return Wire.endTransmission();    // stop transmitting
  if(systemSafe >= SAFE2) {
    int ReadBackError = 0;
    uint8_t CurrentVal = readByte(PULLUPENA + Port, ReadBackError);
    if(CurrentVal != PullUpEnable[Port] && ReadBackError == 0x00) {
      globalError = globalError | MEMORY_ERROR; //Set memory error 
      // return -1; //Exit without completion //FIX!
      PullUpEnable[Port] = CurrentVal; //Update stored value
    }
  }
  int Error = writeByte(PULLUPENA + Port, Config);
  if(Error == 0x00) PullUpEnable[Port] = Config; //Write value back if no I2C error
  return Error; 
}

int PCAL9535A::getInterrupt(int Pin)
{
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    int Error = 0;
    uint8_t Data = readByte(FLAGB, Error);
    if(Error == 0x00) return (Data >> (Pin - 8)) & 0x01; //Return flag output
    else return -1; //Return fail if I2C error 
  }
  if(Pin <= 7) {
    int Error = 0;
    uint8_t Data = readByte(FLAGA, Error);
    if(Error == 0x00) return (Data >> (Pin - 8)) & 0x01; //Return flag output
    else return -1; //Return fail if I2C error 
  }
  return -1; //Fail is state is ill-defined
}

unsigned int PCAL9535A::clearInterrupt(uint8_t Option)
{
  // switch(Port){ //Read port to clear interrupts and returns pin state 
  //   case A:
  //     return readByte(PORTA);
  //     break;
  //   case B:
  //     return readByte(PORTB) << 8;
  //     break;
  //   case BOTH:
  //     return (readByte(PORTB) << 8) | readByte(PORTA);
  //     break;
  //   default:
  //     return 0;
  //     break;
  // }
  if(Option == IntAge::CURRENT || Option == IntAge::BOTH) readBus(); //Do a full word read to clear the current interrupt
  if(Option == IntAge::STALE || Option == IntAge::BOTH) staleInterrupt = false; //Clear int flag
  
  return 0; //FIX!
}

//IN DEVLOPMENT
int PCAL9535A::setInputPolarity(int Pin, bool State, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(State == ON)
  {
    PinPolarityConfig[Port] = PinPolarityConfig[Port] | (0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    writeByte(POLA + Port, PinPolarityConfig[Port]); //Write adjusted value back
    return 1;
  }
  else if(State == OFF)
  {
    PinPolarityConfig[Port] = PinPolarityConfig[Port] & ~(0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    writeByte(POLA + Port, PinPolarityConfig[Port]); //Write adjusted value back
    return 0;
  }
  else 
    return -1; //Fail if state is ill-defined
  // return -1; 
}

//IN DEVLOPMENT
int PCAL9535A::setInputPolarity(int Pin, bool State) 
{
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    return setInputPolarity(Pin - 8, State, PORT_B); //Shift pin number, pass along to set port B
  }
  if(Pin <= 7) {
    return setInputPolarity(Pin, State, PORT_A); //Pass along to set port A
  }
  return -1; //Fail is state is ill-defined
  // return -1;
}

//IN DEVLOPMENT
bool PCAL9535A::getInputPolarity(int Pin, bool Port)
{
  // return 0; 
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }
  else {
    int Error = 0;
    uint8_t Data = readByte(POLA, Error);
    if(Error == 0x00) return (Data >> (Pin - 8)) & 0x01; //Return flag output
    else return -1; //Return fail if I2C error 
  }
  return -1; //Return error if unknown
}

//IN DEVLOPMENT
bool PCAL9535A::getInputPolarity(int Pin)
{
  // return 0;
  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    return getInputPolarity(Pin - 8, PORT_B); //Shift pin number, pass along to set port B
  }
  if(Pin <= 7) {
    return getInputPolarity(Pin, PORT_A); //Pass along to set port A
  }
  return -1; //Fail is state is ill-defined
}

uint16_t PCAL9535A::getAllInterrupts(uint8_t Option)
{
  if(Option == IntAge::CURRENT) return readWord(FLAGA); //Return the current interrupt reg status
  if(Option == IntAge::STALE) return staleIntVals.interrupt; //Return stored interrupt reg
  if(Option == IntAge::BOTH) return readWord(FLAGA) | staleIntVals.interrupt; //Combine them to show if an interrupt has been triggered on the pin now or previously
  return -1; //Return error
}

uint16_t PCAL9535A::getInterruptMask()
{
  int Error = 0;
  uint16_t Data = readWord(INTMASKA, Error);
  if(systemSafe >= SAFE2) {
    uint16_t StoredData = InterruptMask[0] | (InterruptMask[1] << 8); //Contatonate for purposes of testing
    if(StoredData != Data && Error == 0x00) {
      globalError = globalError | MEMORY_ERROR; //Set memory error 
      InterruptMask[0] = Data & 0x00FF; //Copy over updated data
      InterruptMask[1] = (Data & 0xFF00) >> 8; 
    }
  }
  if(Error == 0x00) return Data;
  else return 0; //FIX?? 
}

uint16_t PCAL9535A::getLatch()
{
  int Error = 0;
  uint16_t Data = readWord(LATCHA, Error);
  if(systemSafe >= SAFE2) {
    uint16_t StoredData = InputLatch[0] | (InputLatch[1] << 8); //Contatonate for purposes of testing
    if(StoredData != Data && Error == 0x00) {
      globalError = globalError | MEMORY_ERROR; //Set memory error 
      InputLatch[0] = Data & 0x00FF; //Copy over updated data
      InputLatch[1] = (Data & 0xFF00) >> 8; 
    }
  }
  if(Error == 0x00) return Data;
  else return 0; //FIX?? 
}

int PCAL9535A::readPort(bool Port)
{
  if(readWord(FLAGA) != 0x0000) { //If there currently is an interrupt,
    staleIntVals.interrupt = getAllInterrupts(IntAge::CURRENT); //Grab interrupt output
    staleIntVals.latch = getLatch(); //Grab current latch configuration
    staleIntVals.interruptMask = getInterruptMask();
    staleIntVals.configuration = readWord(DIRA);
    staleIntVals.polarity = readWord(POLA);
    staleIntVals.pullupDir = readWord(PULLUPA);
    staleIntVals.pullupEN = readWord(PULLUPENA);
    staleInterrupt = true; //Set interrupt flag
    clearInterrupt(IntAge::CURRENT); //Clear interrupt 

  }

  return readByte(PORTA + Port); //Return specified subset of port
}

//Pin -> The pin (0~15) which should be configured, Latch -> if true, result is moved into the PORT register after the interrupt event and stored there, and the interrupt flag remains set 
int PCAL9535A::setIntPinConfig(int Pin, bool Latch)
{
  // uint8_t DefVals = 0; //Used to store the existing default value configuration of the port in question
  // uint8_t Control = 0; //Used to store the existing int control register configuration of the port in question

  if(Pin > 15 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(Pin >= 8) {
    // Control = readByte(INTCONB); //Read in existing control values
    // DefVals = readByte(DEFVALB); //Read in existing default values
    // InterruptTypeConf[B] = clearBit(InterruptTypeConf[B], Pin - 8); //Clear existing bit
    // InterruptTypeConf[B] = InterruptTypeConf[B] | ((!OnChange) << (Pin - 8)); //Apply new value
    // DefaultValConf[B] = clearBit(DefaultValConf[B], Pin - 8); //Clear existing bit
    // DefaultValConf[B] = DefaultValConf[B] | (DefVal << (Pin - 8)) ; //Apply new value
    // writeByte(INTCONB, InterruptTypeConf[B]); //Write control value back
    // return writeByte(DEFVALB, DefaultValConf[B]); //Write default values back
    if(systemSafe >= SAFE2) {
      int Error = 0;
      uint8_t CurrentMask = readByte(LATCHB, Error);
      if(InterruptMask[B] != CurrentMask && Error == 0x00) {
        globalError = globalError | MEMORY_ERROR; //Set Read Memory error bit, only if problem was not caused by an I2C error
        return -1;
      }
    }
    uint8_t TempMask = InterruptMask[B];
    uint8_t TempLatch = InputLatch[B];

    TempMask = TempMask & ~(0x01 << Pin); //Clear mask bit to enable interrupts on that pin
    TempLatch = TempLatch | (Latch << Pin); //Set the appropriate value for the latch control
    int Error = writeByte(INTMASKB, TempMask); //Write interrupt mask value back
    if(Error == 0x00) InterruptMask[B] = TempMask; //Write value back if no error on send
    Error = writeByte(LATCHB, TempLatch); //Write latch values back
    if(Error == 0x00) InputLatch[B] = TempLatch; //Write values back if no error on send 
  }
  if(Pin <= 7) {
    // Control = readByte(INTCONA); //Read in existing control values
    // DefVals = readByte(DEFVALA); //Read in existing default values
    if(systemSafe >= SAFE2) {
      int Error = 0;
      uint8_t CurrentMask = readByte(LATCHA, Error);
      if(InterruptMask[A] != CurrentMask && Error == 0x00) {
        globalError = globalError | MEMORY_ERROR; //Set Read Memory error bit, only if problem was not caused by an I2C error
        return -1;
      }
    }
    uint8_t TempMask = InterruptMask[B];
    uint8_t TempLatch = InputLatch[B];

    TempMask = TempMask & ~(0x01 << Pin); //Clear mask bit to enable interrupts on that pin
    TempLatch = TempLatch | (Latch << Pin); //Set the appropriate value for the latch control
    int Error = writeByte(INTMASKA, TempMask); //Write interrupt mask value back
    if(Error == 0x00) InterruptMask[A] = TempMask; //Write value back if no error on send
    Error = writeByte(LATCHA, TempLatch); //Write latch values back
    if(Error == 0x00) InputLatch[A] = TempLatch; //Write values back if no error on send 
  }
  return -1; //Fail is state is ill-defined
}

// int PCAL9535A::setIntConfig(bool Mirror, bool OpenDrain, bool Polarity, bool Clearing)
// {
//   uint8_t Config = 0x00 | (Mirror << 6) | (OpenDrain << 2) | (Polarity << 1) | Clearing; //Assembly config register 
//   Wire.beginTransmission(ADR); // transmit to device with address ADR
//   Wire.write(IOCON);        //Send to port configuration register
//   Wire.write(Config);         
//   return Wire.endTransmission();    // stop transmitting
// }

int PCAL9535A::setBusOutput(uint8_t Mode, bool Port)
{
  int Error = 0;
  uint8_t CurrentOutput = readByte(PORTOUTPUT, Error); //Read in current state
  if(CurrentOutput != PortOutput && Error == 0x00 && systemSafe >= SAFE2) { //Only a memory error if read correctly
    globalError = globalError | MEMORY_ERROR; //Set memory error 
    return -1; //Exit with non-complete error code
  }
  if(Error == 0x00) { //Only try to write back if no error occoured 
    if(Mode == OutputType::OPEN_DRAIN) CurrentOutput = CurrentOutput | (0x01 << Port); //If open drain is desired, set specified bit
    if(Mode == OutputType::PUSH_PULL) CurrentOutput = CurrentOutput & ~(0x01 << Port); //If push-pull is desired, clear specified bit
    Error = writeByte(PORTOUTPUT, CurrentOutput); //Write value back, grab error
    if(Error == 0x00) PortOutput = CurrentOutput; //Copy over value if write was succesful 
    return Error; //Return error either way 
  }
  return Error; 
}

uint16_t PCAL9535A::getError()
{
  return (globalError << 8) | globalErrorI2C;
}

uint16_t PCAL9535A::clearError()
{
  uint16_t errorTemp = (globalError << 8) | globalErrorI2C; //Make local copy
  globalError = 0x00; //Clear stored 
  globalErrorI2C = 0x00; //Clear stored I2C
  return errorTemp; //Return value from before clear
}

void PCAL9535A::safeMode(int state)
{
  systemSafe = state; //Set updated state of system
}

uint16_t PCAL9535A::readBus()
{
  // Wire.beginTransmission(ADR); // transmit to device with address ADR
  // Wire.write(PORTA);        //Address appropriate port register
  // Wire.endTransmission();    // stop transmitting

  // Wire.requestFrom(ADR, 2);
  // unsigned int LowByte = Wire.read();
  // unsigned int HighByte = Wire.read();

  // return (HighByte << 8) | LowByte;
  int Error = 0;
  uint16_t Data = readWord(PORTA, Error);  
  // if((Error & 0x07) != 0x00) globalError = globalError & 0xF8; //Clear I2C error bits if there is a new I2C error
  // globalError = globalError | Error; //Add in any new error flags and set I2C error
  if(Error == 0x00) return Data;
  else return 0x00; //FIX??
}

//Error returns the error state of the I2C transmission
uint8_t PCAL9535A::readByte(int Pos, int &Error)
{
  Wire.beginTransmission(ADR); // transmit to device with address ADR
  Wire.write(Pos);        //Address appropriate register
  Wire.endTransmission();    // stop transmitting
  if(systemSafe > SAFEOFF) globalErrorI2C = positionDecode(globalErrorI2C, Error); //Set bit in error byte, if needed 

  unsigned long LocalTime = millis();
  Wire.requestFrom(ADR, 1);
  while(Wire.available() < 1 && (millis() - LocalTime < Timeout)); //Wait for data
  return Wire.read();
}

uint8_t PCAL9535A::readByte(int Pos)
{
  int Dummy = 0; //Pass error into a dummy variable 
  return readByte(Pos, Dummy);
}


//Error returns the error state of the I2C transmission
uint16_t PCAL9535A::readWord(int Pos, int &Error)
{
  Wire.beginTransmission(ADR);
  Wire.write(Pos);
  Error = Wire.endTransmission();
  if(systemSafe > SAFEOFF) globalErrorI2C = positionDecode(globalErrorI2C, Error); //Set bit in error byte, if needed 

  unsigned long LocalTime = millis();
  Wire.requestFrom(ADR, 2);
  while(Wire.available() < 2 && (millis() - LocalTime < Timeout)); //Wait for data
  uint16_t LowByte = Wire.read();
  uint16_t HighByte = Wire.read();
  return (HighByte << 8) | LowByte; //Contatonate and return 
}

uint16_t PCAL9535A::readWord(int Pos)
{
  int Dummy = 0; //Pass error into a dummy variable 
  return readWord(Pos, Dummy);
}

int PCAL9535A::writeByte(int Pos, uint8_t Val)
{
  Wire.beginTransmission(ADR); // transmit to device with address ADR
  Wire.write(Pos);        //Address appropriate register
  Wire.write(Val);
  int Error = Wire.endTransmission();    // stop transmitting

  if(systemSafe > SAFE1 && Error == 0x00) { //Only test for readback error if an I2C error is not to blame and error logging it turned on
    int ReadBackError = 0;
    uint16_t Data = readByte(Pos,ReadBackError);
    if(Data != Val && ReadBackError != 0x00) globalError = globalError | READWRITE_ERROR; //Set Read/Write error bit, only if problem was not caused by an I2C error
  }

  if(systemSafe > SAFEOFF) globalErrorI2C = positionDecode(globalErrorI2C, Error); //Set bit in error byte, if needed 
  return Error; 
}

uint8_t PCAL9535A::clearBit(uint8_t Val, uint8_t Pos)
{
  return Val & ~(0x01 << Pos); //Return adjusted byte
}

//Converts a position encoded value into a bit-wise encoding 
uint8_t PCAL9535A::positionDecode(uint8_t Val, uint8_t Pos)
{
  if(Pos == 0) return Val; //Return unalterned if there is no bit to set
  return Val | (0x01 << (Pos - 1)); //Return adjusted byte
}







/******************************************************************************
PACL9535A.h
Interface for PACL9535A 16 bit IO expander
Bobby Schulz @ GEMS IoT
04/25/2022
https://github.com/gemsiot/PACL9535A_Library

Based on MCP23008 library developed by Northern Widget

Allows control of all aspects of the control of the IO expander 

1.0.0

"All existing things are really one"
-Zhuangzi

Distributed as-is; no warranty is given.

© 2023 Regents of the University of Minnesota. All rights reserved.
******************************************************************************/

#ifndef PCAL9535A_h
#define PCAL9535A_h

#include <Arduino.h>
#include <Particle.h>
#include <Wire.h>

#define ON 1
#define OFF 0

#define PORT_A 0
#define PORT_B 1
// #define BOTH 2

#define INPUT_PULLDOWN 3  //4th potential state of operation for IO expander pins

#define PCAL9535A_BASE_ADR 0x20 //Base address for device, to be or-ed with configurable address
// #define BASE_ADR= 0x20 //Base address for device, to be or-ed with configurable address

// #define DIRA= 0x06
// #define DIRB= 0x07
// #define POLA= 0x04 
// #define POLB= 0x05
// #define INTMASKA= 0x4A
// #define INTMASKB= 0x4B
// #define PULLUPA= 0x48
// #define PULLUPB= 0x49
// #define PULLUPENA= 0x46
// #define PULLUPENB= 0x47 
// #define PORTA= 0x00
// #define PORTB= 0x01
// #define LATA= 0x02
// #define LATB= 0x03
// #define LATCHA= 0x44
// #define LATCHB= 0x45
// #define DRIVEA= 0x40
// #define DRIVEB= 0x42
// #define PORTOUTPUT= 0x4F

// // #define DEFVALA= 0x06
// // #define DEFVALB= 0x07
// // #define IOCON= 0x0A //Also= 0x0B
// #define FLAGA= 0x4C
// #define FLAGB= 0x4D
// // #define INTCAPA= 0x10
// // #define INTCAPB= 0x11
// // #define INTCONA= 0x08
// // #define INTCONB= 0x09

// #define MEMORY_ERROR    0b10000000
// #define READWRITE_ERROR 0b01000000
// #define OUTPUT_ERROR    0b00100000

// //FIX!
// #define SAFEOFF -1 //No additional error logging
// #define SAFE1 0 //I2C error logging only
// #define SAFE2 1 //Memory and read write error logging
// #define SAFE3 2 //Output error logging
// #define SAFE 99 //Turn on all possible safety protocols 

typedef struct  {
    uint16_t interrupt;
    uint16_t latch;
    uint16_t interruptMask;
    uint16_t configuration;
    uint16_t polarity;
    uint16_t pullupDir;
    uint16_t pullupEN;
}intVals;

// enum class IntAge: uint8_t
// {
//     // constexpr uint8_t CURRENT = 0;
//     // constexpr uint8_t STALE = 1;
//     // constexpr uint8_t BOTH = 2;
//     CURRENT = 0,
//     STALE = 1,
//     BOTH = 2,
// };

namespace OutputType 
{
    constexpr uint8_t OPEN_DRAIN = 1;
    constexpr uint8_t PUSH_PULL = 0;
};

// enum class DriveStrength: uint8_t
// {
//     // constexpr uint8_t PER25 = 0b00;
//     // constexpr uint8_t PER50 = 0b01;
//     // constexpr uint8_t PER75 = 0b10;
//     // constexpr uint8_t PER100 = 0b11;
//     DEFAULT = 0,
//     HIGH = 1,
//     STANDARD = 2,
// };

// enum class DriveStrength: uint8_t
// {
//     // constexpr uint8_t PER25 = 0b00;
//     // constexpr uint8_t PER50 = 0b01;
//     // constexpr uint8_t PER75 = 0b10;
//     // constexpr uint8_t PER100 = 0b11;
//     DEFAULT = 0,
//     HIGH = 1,
//     STANDARD = 2,
// };

class PCAL9535A
{
    //     enum IO_Type
    // {
    //     INPUT,
    //     OUTPUT
    // };
    
// struct staleIntVals {
//     uint16_t interrupt;
//     uint16_t latch;
//     uint16_t interruptMask;
//     uint16_t configuration;
//     uint16_t polarity;
//     uint16_t pullupDir;
//     uint16_t pullupEN;
// };

    const int DIRA = 0x06;
    const int DIRB = 0x07;
    const int POLA = 0x04;
    const int POLB = 0x05;
    const int INTMASKA = 0x4A;
    const int INTMASKB = 0x4B;
    const int PULLUPA = 0x48;
    const int PULLUPB = 0x49;
    const int PULLUPENA = 0x46;
    const int PULLUPENB = 0x47; 
    const int PORTA = 0x00;
    const int PORTB = 0x01;
    const int LATA = 0x02;
    const int LATB = 0x03;
    const int LATCHA = 0x44;
    const int LATCHB = 0x45;
    const int DRIVEA = 0x40;
    const int DRIVEB = 0x42;
    const int PORTOUTPUT = 0x4F;

    const int FLAGA = 0x4C;
    const int FLAGB = 0x4D;

    const int MEMORY_ERROR    = 0b10000000;
    const int READWRITE_ERROR = 0b01000000;
    const int OUTPUT_ERROR    = 0b00100000;
    
    public:
    //FIX!
    constexpr static int SAFEOFF = -1; //No additional error logging
    constexpr static int SAFE1 = 0; //I2C error logging only
    constexpr static int SAFE2 = 1; //Memory and read write error logging
    constexpr static int SAFE3 = 2; //Output error logging
    constexpr static int SAFE = 99; //Turn on all possible safety protocols 

  
    PCAL9535A(int _ADR = PCAL9535A_BASE_ADR); //Default to base address if none specified 
    int begin(void);
    int pinMode(int Pin, uint8_t State, bool Port);
    int pinMode(int Pin, uint8_t State);
    int digitalWrite(int Pin, bool State, bool Port);
    int digitalWrite(int Pin, bool State);
    int pinSetDriveStrength(int Pin, DriveStrength State, bool Port);
    int pinSetDriveStrength(int Pin, DriveStrength State);
    int digitalRead(int Pin, bool Port);
    int digitalRead(int Pin);
    int setInterrupt(int Pin, bool State, bool Port);
    int setInterrupt(int Pin, bool State);
    int getInterrupt(int Pin);
    uint16_t getAllInterrupts(uint8_t Option);
    uint16_t getInterruptMask();
    unsigned int clearInterrupt(uint8_t age);
    bool isInterrupt(uint8_t age);
    int setLatch(int Pin, bool State, bool Port);
    int setLatch(int Pin, bool State);
    uint16_t getLatch();
    int setInputPolarity(int Pin, bool State, bool Port);
    int setInputPolarity(int Pin, bool State);
    int setIntPinConfig(int Pin, bool Latch);
    bool getInputPolarity(int Pin, bool Port);
    bool getInputPolarity(int Pin);
    // int setIntConfig(bool Mirror, bool OpenDrain = 0, bool Polarity = 0, bool Clearing = 0);
    int setBusOutput(uint8_t mode, bool Port);
    uint8_t getBusOutput();
    uint16_t readBus(); 
    uint16_t getError();
    uint16_t clearError();
    void safeMode(int state = SAFE); //Turn to max by default 
    intVals staleIntVals;
    // #ifndef DriveStrength
    // enum class DriveStrength: uint8_t
    // {
    //     // constexpr uint8_t PER25 = 0b00;
    //     // constexpr uint8_t PER50 = 0b01;
    //     // constexpr uint8_t PER75 = 0b10;
    //     // constexpr uint8_t PER100 = 0b11;
    //     DEFAULT = 0,
    //     HIGH = 1,
    //     STANDARD = 2,
    // };
    // #endif
    enum IntAge
    {
        // constexpr uint8_t CURRENT = 0;
        // constexpr uint8_t STALE = 1;
        // constexpr uint8_t BOTH = 2;
        CURRENT = 0,
        STALE = 1,
        BOTH = 2,
    };

    uint16_t readWord(int Pos, int &Error);
    

  private:
    int ADR = PCAL9535A_BASE_ADR; //FIX! Replace with equation later
    unsigned long Timeout = 10; //Wait at most 10ms for a response from the device
    uint8_t PinModeConf[2] = {0xFF,0xFF}; //All pins natively inputs (IODIRx)
    uint8_t PortState[2] = {0}; //All pins natively off (LATx)
    uint8_t PinPolarityConfig[2] = {0x00}; //All pins natively non-inverted (IPOLx)
    uint8_t PullUpConf[2] = {0xFF,0xFF}; //Pullup be default 
    uint8_t PullUpEnable[2] = {0x00}; //Natively disabled (GPPUx)
    uint8_t InterruptMask[2] = {0xFF,0xFF}; //Interrupts dissabled by default 
    // uint8_t DefaultValConf[2] = {0x00}; //Default interrupt value (DEFVALx)
    // uint8_t InterruptTypeConf[2] = {0x00}; //Interrupt type values (INTCONx)
    uint8_t InputLatch[2] = {0x00}; //Default to non-latching 
    uint8_t CurrentControl[4] = {0xFF,0xFF,0xFF,0xFF}; //Default to max output current 
    uint8_t PortOutput = 0x00; //Default to push-pull outputs

    bool staleInterrupt = false; //Set if an interrupt has occoured during other process

    uint8_t globalError = 0x00; //Default to no error
    uint8_t globalErrorI2C = 0x00; //Default to no error

    int systemSafe = SAFE; //Default to safe mode enabled 

    int setPort(int Config, bool Port);
    int setDirection(int Config, bool Port);
    int setPolarity(int Config, bool Port);
    int setPullupDir(int Config, bool Port);
    int setPullupEN(int Config, bool Port);
    int setInt(int Config, bool Port);
    int readPort(bool Port); 
    uint8_t readByte(int Pos, int &Error);
    uint8_t readByte(int Pos);
    
    uint16_t readWord(int Pos);
    int writeByte(int Pos, uint8_t Val);
    uint8_t clearBit(uint8_t Val, uint8_t Pos);
    uint8_t positionDecode(uint8_t Val, uint8_t Pos);
    
};

#endif
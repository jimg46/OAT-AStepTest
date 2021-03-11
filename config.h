//TestStep config file 
  //pin assignments
#ifndef config_h
#define config_h

#define EN_PIN      6 // Enable
#define DIR_PIN     7 // Direction
#define STEP_PIN    3 // Step
#define SW_RX       4 // SoftwareSerial receive pin
#define SW_TX       5 // SoftwareSerial transmit pin
#define DIAG_PIN    2 // UNO so need ext int
//#define SERIAL_PORT Serial1 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2

#define R_SENSE 0.11f // Match to your driver
 int const fwd = 0; //RA=0
  
  float spd = 161.f; //RA, V2, 0.9deg
  //float spd = 80.43f; //DEC, V2, 0.9deg
#endif //config_h

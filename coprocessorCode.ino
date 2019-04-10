#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
        // initializations and setup of global variables
int rSpeed,lSpeed;
const byte motRf = 5;   // right motor forwards
const byte motRb = 6;   // right motor backwards
const byte mode0 = 7;   // pic input 0
const byte mode1 = 8;   // pic input 1
const byte motLf = 9;   // left motor forwards
const byte motLb = 11;  // left motor backwards
unsigned char isRefYawAss = 0;      // reference yaw is assigned 
              // when the robot is told to move for the first time
unsigned char comingBackFirst = 1;  // In the first iteration
              // of the coming back loop, the robot will turn
              // slightly to ensure that swivels turn away from 
              // poles and don't disqualify robot

    // begin gyro setup
MPU6050 mpu;
#define OUTPUT_READABLE_YAWPITCHROLL

bool dmpReady = false;  // set true if DMP init was successful
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
    // end gyro setup

void setup() {
// setup pins as input or output
    pinMode(motRf, OUTPUT);
    pinMode(motRb, OUTPUT);
    pinMode(mode0, INPUT);
    pinMode(mode1, INPUT);
    pinMode(motLf, OUTPUT);
    pinMode(motLb, OUTPUT);

// stop driving motors
    digitalWrite(motRf,LOW);
    digitalWrite(motRb,LOW);
    digitalWrite(motLf,LOW);
    digitalWrite(motLb,LOW);
    
// join I2C bus with MPU6050 gyroscope
    #if I2CDEV_IMPLEM
    NTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

// Setup Serial communication for PC debugging as required
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately

// initialize MPU6050 gyroscope
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();

// verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

// device is ready when buffer empty (no extra data stored)
    while (Serial.available() && Serial.read()); // empty buffer

// load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    mpu.setXGyroOffset(220);    // <- these offsets work when gyro is flat with z axis pointing to sky
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788);

// make sure setup worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready!"));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
// setup failed. print error code to PC through Serial
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
}

void loop() {
    float factor = 0;                 // correction factor for driving straight
    static unsigned char mode = 0b00; // pic telling arduino what to do
    unsigned char rSpeed = 160;       // base speed of right motor
    unsigned char lSpeed = 160;       // base speed of left motor
    static float yawRef = 0.0;        // reference yaw value to align robot to
    Serial.print("mode  "); Serial.println(mode);   // <- debugging: print arduino mode to Serial

// if gyro setup failed, try it again (until it works)
    if (!dmpReady) {
      setup();
      return;
    }

// if PIC inputs changed, update arduino mode accordingly
    if (stateRead() != mode){
      mode = stateRead();
// if robot is told to move for the first time since reset, 
// store current yaw as the reference yaw to hold during operation
      if ((isRefYawAss == 0) && (mode != 0)){
          delay(500);           // <- debounce
          if ((isRefYawAss == 0) && (mode != 0)){
              if (mode != 0){
                yawRef = (ypr[0] * 180/M_PI);
                isRefYawAss = 1;            // indicates reference yaw has been assigned
              }
          } else {
            yawRef = 0.0;
          }
      }
    }
    
// begin reading gyroscope values    
    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        fifoCount = mpu.getFIFOCount();
        Serial.println(F("FIFO overflow!"));
        // ^ too many readings came in from gyro. delete them all and wait for fresh ones

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;

        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            Serial.print("ypr\t");
            Serial.print(ypr[0] * 180/M_PI);
            Serial.print("\t");
            Serial.print(ypr[1] * 180/M_PI);
            Serial.print("\t");
            Serial.println(ypr[2] * 180/M_PI);
        #endif
    }
// end reading gyroscope values
// at this point, ypr is a readable variable which shows the robot's
// yaw, pitch, and roll values respectively

// if PIC is telling arduino to stop, stop the robot
    if (mode == 0b00){
      digitalWrite(motRf,LOW);
      digitalWrite(motRb,LOW);
      digitalWrite(motLf,LOW);
      digitalWrite(motLb,LOW);
    } else {
// PIC is telling arduino to move, so calculate the correction factor from yaw value
        factor = (ypr[0] * 180/M_PI) - yawRef;
        factor = calc(factor);
        // Serial.print("Factor: ");Serial.println(factor);           // <- this was for debugging
// if robot is going out, assign speed + correction using PWM
        if (mode == 0b10){
            rSpeed = rSpeed+factor;
            lSpeed = lSpeed-factor;
              analogWrite(motRf,rSpeed);
            digitalWrite(motRb,LOW);
              analogWrite(motLf,lSpeed);
            digitalWrite(motLb,LOW);
// if robot is coming back, assign speed + correction using PWM at a higher speed
        } else if (mode == 0b01){
            if (comingBackFirst == 1){        // <- if robot just started coming back, turn a little
                                              // bit moving forward so the swivel wheels turn away
                                              // from the line of poles (to avoid disqualification)
              comingBackFirst = 0;
              rSpeed = 200;
              lSpeed = 150;
              analogWrite(motRf,rSpeed);
              digitalWrite(motRb,LOW);
              analogWrite(motLf,lSpeed);
              digitalWrite(motLb,LOW);
              delay(2000);
            }
            rSpeed = rSpeed+30-factor/2;
            lSpeed = lSpeed+50+factor/2;
            digitalWrite(motRf,LOW);
              analogWrite(motRb,rSpeed);
            digitalWrite(motLf,LOW);
              analogWrite(motLb,lSpeed);

// if PIC wants arduino to nudge forward, nudge forward for 100 ms and then wait for 200ms before next nudge
        } else if (mode == 0b11) {
            rSpeed = rSpeed+factor;
            lSpeed = lSpeed-factor;
            analogWrite(motRf,rSpeed);
            digitalWrite(motRb,LOW);
            analogWrite(motLf,lSpeed);
            digitalWrite(motLb,LOW);
              delay(100);
            digitalWrite(motRb,LOW);
            digitalWrite(motRf,LOW);
            digitalWrite(motLb,LOW);
            digitalWrite(motLf,LOW);
              delay(200);
            mpu.resetFIFO();                // empty the useless gyroscope readings we got while the arduino
                                            // delaying and just standing still
            
        }
    }
}


// read PIC input pins and convert them to arduino processor mode
unsigned char stateRead(void){
    unsigned char rMode = 0;
    if (digitalRead(mode0) == HIGH){
      rMode += 1;
    }
    if (digitalRead(mode1) == HIGH){
      rMode += 2;
    }
    return rMode;
}

// calculates sqrt correction function function up to a maximum amplitude of
// 'amp' at a variance of 'var' degrees. The trend follows a square root function
// allowing a high rate of increase of correction at small deviations. This
// ensures the robot is not allowed to deviate significantly at all.
// a visual representation of the actual correction function used is shown
// at https://imgur.com/a/Ii4TLHJ and at https://www.desmos.com/calculator/gqj8h4wvb1
float calc(float d){
  unsigned char amp = 80;
  unsigned char var = 3;
  unsigned char maxVar = 180.0;
  if (d < (-360.0+maxVar)){
    if (d < (-360.0+var)){
      return amp*sqrt((d+360)/var);
    }
    return amp;
  } else if (d > 360.0 - maxVar){
    if (d > (360.0-var)){
      return -amp*sqrt(-(d-360.0)/var);
    }
    return -amp;
  } else  if (d > -maxVar && d < maxVar) {
    if (d > -var && d < var){
      if (d > 0){
        return -amp*sqrt(d/var);
      } else if (d < 0){
        return amp*sqrt(-d/var);
      }
    }
    if (d > var){
      return -amp;
    } else if (d < -var){
      return amp;
    }
  } else {
    return 0;
  }
  return 0;
}

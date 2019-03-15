#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

int rSpeed,lSpeed;
const byte motRf = 5;
const byte motRb = 6;
const byte mode0 = 7;
const byte mode1 = 8;
const byte motLf = 9;
const byte motLb = 10;

MPU6050 mpu;
#define OUTPUT_READABLE_YAWPITCHROLL

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

void setup() {
    pinMode(motRf, OUTPUT);
    pinMode(motRb, OUTPUT);
    pinMode(mode0, INPUT);
    pinMode(mode1, INPUT);
    pinMode(motLf, OUTPUT);
    pinMode(motLb, OUTPUT);

    digitalWrite(motRf,LOW);
    digitalWrite(motRb,LOW);
    digitalWrite(motLf,LOW);
    digitalWrite(motLb,LOW);
    
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately

    // initialize device
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // ready when buffer empty
    while (Serial.available() && Serial.read()); // empty buffer

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
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
    float factor = 0;
    static unsigned char mode = 0b00;                     // 00 for stop, 01 for out, 10 for in, 11 is meaningless
    unsigned char lSpeed = 160;
    unsigned char rSpeed = 160;
    static float yawRef = 0.0;
    // if programming failed, don't try to do anything
    if (!dmpReady) return;

    if (stateRead() != mode){
      mode = stateRead();
      yawRef = (ypr[0] * 180/M_PI);
    }
    
    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        fifoCount = mpu.getFIFOCount();
        Serial.println(F("FIFO overflow!"));

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

    if (mode == 0b00){
      digitalWrite(motRf,LOW);
      digitalWrite(motRb,LOW);
      digitalWrite(motLf,LOW);
      digitalWrite(motLb,LOW);
    } else {
        factor = (ypr[0] * 180/M_PI) - yawRef;
        factor = calc(factor);
        Serial.print("Factor: ");Serial.println(factor);
        if (mode == 0b01){
            rSpeed = 160+factor;
            lSpeed = 160-factor;
              analogWrite(motRf,rSpeed);
            digitalWrite(motRb,LOW);
              analogWrite(motLf,lSpeed);
            digitalWrite(motLb,LOW);
            Serial.print("Left Speed: ");Serial.print(lSpeed); Serial.print("Right Speed: "); Serial.println(rSpeed);
        } else {
            rSpeed = 160-factor;
            lSpeed = 160+factor;
            digitalWrite(motRf,LOW);
              analogWrite(motRb,rSpeed);
            digitalWrite(motLf,LOW);
              analogWrite(motLb,lSpeed);
        }
    }
}

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

float calc(float d){
  if (d < -350.0){
    if (d < -359.0){
      return 0;
      return (90.0*sqrt((d+360)));
    }
    return 90.0;
    //return map(d,-360.0,-350.0,0.0,-85.0);
  } else if (d > 350.0){
    if (d > 359.0){
      return 0;
      return -90.0*sqrt(-(d-360.0));
    }
    return -90.0;
    //return map(d,350.0,360.0,85.0,0.0);
  } else  if (d > -10.0 && d < 10.0) {
    if (d > -1.0 && d < 1.0){
      if (d > 0){
        return 0;
        return -90.0*sqrt(d);
      } else if (d < 0){
        return 0;
        return 90.0*sqrt(-d);
      }
    }
    if (d > 1.0){
      return -90.0;
    } else if (d < -1.0){
      return 90.0;
    }
  } else {
    return 0;
  }
  return 0;
}

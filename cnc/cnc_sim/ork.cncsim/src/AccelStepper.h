//

extern "C" {
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
}
#include <string>

typedef char byte;
const int INPUT = 0;
const int OUTPUT = 1;

void pinMode( int pin, int mode );
void delay(int micros);
int millis();
int digitalRead(int pin);
void analogWrite(int pin, byte val);

#define CNC_SIMULATOR 1

struct AccelStepper
{

    static const int HALF4WIRE=0;

    AccelStepper( int type,
                  int pina,
                  int pinb,
                  int pinc,
                  int pind );

    void setMaxSpeed(int sp);
    void setAcceleration( int acc );
    void enableOutputs();
    void disableOutputs();
    void moveTo(int pos);
    void run();
    void setCurrentPosition(int pos);
    int currentPosition();
    int distanceToGo();

    float mPhysActualPosition;
    float mPhysMaximumPosition;

private:

    void resetTimer();
    void dumpState();

    std::string mName;
    float mPrevTime;
    float mPrevDumpTime;

    float mDirection;
    float mCurrentPosition;
    float mTargetPosition;
    float mCurrentSpeed;
    float mMaxSpeed;
    float mAcceleration;
    float mPrevDistance;
    bool mEnabled;
};

struct SerialImpl
{
    void println( const char* c_str );
    void begin(int baudrate);
    byte readBytes( char* buffer, int buffer_size ); // returns num read
};

struct StepperVizState
{
    std::string mName;
    float mSize;
    float mPhysicalPos;
    float mPhysicalMax;
    float mCurrentPos;
    float mTargetPos;
    bool mEnabled;
    float mSpindleSpeed;
};

extern SerialImpl Serial;
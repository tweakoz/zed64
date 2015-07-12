//
#include "AccelStepper.h"
#include <ork/timer.h>
#include <assert.h>
#include <unistd.h>

SerialImpl Serial;
ork::Timer gtimer;

static AccelStepper* stepper_X = nullptr;
static AccelStepper* stepper_Y = nullptr;
static AccelStepper* stepper_Z1 = nullptr;
static AccelStepper* stepper_Z2 = nullptr;

void UpdateStepperViz(StepperVizState vizstate);

void pinMode( int pin, int mode )
{
  printf( "pinMode pin<%d> mode<^%d>\n", pin, mode );
}
float gSpindleSpeed;

int digitalRead(int pin)
{
    int rval = 0;
    if( stepper_X && pin==48 )  // limitsw_pinx1
    {
        rval = stepper_X->mPhysActualPosition<1.0;
        //printf( "PINX1<%d>\n", int(rval));
    }
    else if( stepper_X && pin==49 )  // limitsw_pinx2
    {
        rval = stepper_X->mPhysActualPosition>(stepper_X->mPhysMaximumPosition-1);
        //printf( "PINX2<%d>\n", int(rval));
    }
    else if( stepper_Y && pin==50 )  // limitsw_piny1
    {
        rval = stepper_Y->mPhysActualPosition<1.0;
    }
    else if( stepper_Y && pin==51 )  // limitsw_piny2
    {
        rval = stepper_Y->mPhysActualPosition>(stepper_Y->mPhysMaximumPosition-1);
    }
    else if( stepper_Z1 && pin==52 )  // limitsw_piny1
    {
        rval = stepper_Z1->mPhysActualPosition<1.0;
    }
    else if( stepper_Z1 && pin==53 )  // limitsw_piny2
    {
        rval = stepper_Z1->mPhysActualPosition>(stepper_Z1->mPhysMaximumPosition-1);
    }
  return rval;;
}
void analogWrite(int pin, byte val)
{
    gSpindleSpeed = float(val/255.0);
  printf( "analogWrite pin<%d> val<%d>\n", pin, int(val) );
}
void delay(int micros)
{
  printf( "delay micros<%d>\n", micros );
}
int millis()
{
    return int(gtimer.SecsSinceStart()*8000.0f);
}
void SerialImpl::begin(int baudrate)
{
   printf( "SerialImpl::begin baud<%d>\n", baudrate );
}
byte SerialImpl::readBytes( char* out_buffer, int buffer_size )
{
    static char input_buffer[80];
    static int input_count = 0;

    int count = read( 0, input_buffer, buffer_size );
    if( count>0 )
    {
        memcpy( out_buffer, input_buffer, count );
        return count;

        printf( "r<%d>\n", count );
    }
    return 0;
}
void SerialImpl::println( const char* c_str )
{
    printf( "%s\n", c_str );
}
AccelStepper::AccelStepper(int type,int pina, int pinb, int pinc, int pind )
    : mPrevTime(float(millis())/1000.0f)
    , mCurrentPosition(0)
    , mCurrentSpeed(0)
    , mTargetPosition(0)
    , mMaxSpeed(0)
    , mAcceleration(0)
    , mEnabled(false)
    , mPrevDistance(0.0)
    , mDirection(0.0)
    , mPrevDumpTime(0.0)
{
    if( pina == 22 )
    {    mName = "Y";
         mPhysMaximumPosition = 4079.0;
         mPhysActualPosition = 2000.0;
         stepper_Y = this;
    }
    else if( pina == 28 )
    {    mName = "X";
         mPhysMaximumPosition = 5176.0;
         mPhysActualPosition = 2000.0;
         stepper_X = this;
    }
    else if( pina == 34 )
    {    mName = "Z1";
         mPhysMaximumPosition = 4304.0;
         mPhysActualPosition = 2000.0;
         stepper_Z1 = this;
    }
    else if( pina == 40 )
    {    mName = "Z2";
         mPhysMaximumPosition = 4304.0;
         mPhysActualPosition = 2000.0;
         stepper_Z2 = this;
    }
    resetTimer();
}
void AccelStepper::setMaxSpeed(int sp)
{
    //printf( "Stepper<%s> setMaxSpeed<%d>\n", mName.c_str(), sp );
    mMaxSpeed = sp;
}
void AccelStepper::setAcceleration( int acc )
{
    //printf( "Stepper<%s> setAcceleration<%d>\n", mName.c_str(), acc );
    mAcceleration = acc;
}
void AccelStepper::enableOutputs()
{
    //printf( "Stepper<%s> enableOutputs\n", mName.c_str() );
    dumpState();
    mEnabled = true;
    resetTimer();
}
void AccelStepper::disableOutputs()
{
    //printf( "Stepper<%s> disableOutputs\n", mName.c_str() );
    dumpState();
    mEnabled = false;
    resetTimer();
}
void AccelStepper::moveTo(int pos)
{
    //printf( "Stepper<%s> moveTo<%d> cur<%f> phys<%f>\n", mName.c_str(), pos, mCurrentPosition, mPhysActualPosition );
    mTargetPosition = pos;
    float dpos = mTargetPosition-mCurrentPosition;
    mDirection = (dpos>0.0) ? 1.0 : (dpos<0.0) ? -1.0 : 0.0;
    resetTimer();
}
void AccelStepper::resetTimer()
{
    float curtime = float(millis())/1000.0f;
    mPrevTime = curtime;
}
void AccelStepper::dumpState()
{
    //printf( "/////////////////////////////////\n");
    //printf( "Stepper<%s> stat - phys<%f> cur<%d> tgt<%d> spd<%d>\n", mName.c_str(), mPhysActualPosition, int(mCurrentPosition), int(mTargetPosition), int(mCurrentSpeed) );
    //printf( "/////////////////////////////////\n");

}
void AccelStepper::run()
{
    float curtime = float(millis())/1000.0f;
    float elapsed = curtime-mPrevTime;
    //printf( "curtime<%f> prv<%f> ela<%f>\n", curtime, mPrevTime, elapsed );
    mPrevTime = curtime;

    float distance = distanceToGo();

    float dpos = 0.0f;

    if( mDirection==-1.0f )
    {
        if( distance>0.0 )
        {   //printf( "Stepper<%s> reached target dist<%f> \n", mName.c_str(), distance );
            mCurrentSpeed = 0.0f;
            mCurrentPosition = mTargetPosition;
            mDirection = 0.0f;
            resetTimer();
        }
        else
        {
            mCurrentSpeed = -mMaxSpeed;
            dpos = mCurrentSpeed*elapsed;

            mCurrentPosition += dpos;
            mPhysActualPosition += dpos;
            if( mPhysActualPosition<0.0f )
            {  mCurrentPosition-=mPhysActualPosition;
               mPhysActualPosition-=mPhysActualPosition;
               printf( "OUTOFBOUNDS!!!!\n");
            }
        }
    }
    else if( mDirection==1.0f)
    {
        if( distance<0.0 )
        {   //printf( "Stepper<%s> reached target dist<%d>\n", mName.c_str(), distance );
            mCurrentSpeed = 0.0f;
            mCurrentPosition = mTargetPosition;
            mDirection = 0.0f;
            resetTimer();
        }
        else
        {
            mCurrentSpeed = +mMaxSpeed;
            dpos = mCurrentSpeed*elapsed;
            mCurrentPosition += dpos;
            mPhysActualPosition += dpos;
            if( mPhysActualPosition>mPhysMaximumPosition )
            {  float overshoot = mPhysActualPosition-mPhysMaximumPosition;
            static int count = 0;
            count++;
               printf( "OUTOFBOUNDS!!!! physAct<%f> physmax<%f>\n", mPhysActualPosition, mPhysMaximumPosition );
               mCurrentPosition-=overshoot;
               mPhysActualPosition-=overshoot;
               assert(count<10);
            }
            else if( mCurrentPosition>mTargetPosition )
            {  float overshoot = mCurrentPosition-mTargetPosition;
               mCurrentPosition-=overshoot;
               mPhysActualPosition-=overshoot;
            }
        }
    }
    else
    {
        //printf( "wtf\n" );
        mCurrentSpeed = 0.0f;
    }


    mPrevDistance = distance;

    if( (curtime-mPrevDumpTime) > 3.0 )
    {
        mPrevDumpTime = curtime;
        dumpState();
    }
    if( (curtime-mPrevDumpTime) > 0.03 )
    {
        StepperVizState vstate;
        vstate.mName = mName;
        vstate.mSize = mPhysMaximumPosition;
        vstate.mPhysicalPos = mPhysActualPosition;
        vstate.mPhysicalMax = mPhysMaximumPosition;
        vstate.mCurrentPos = mCurrentPosition;
        vstate.mTargetPos = mTargetPosition;
        vstate.mEnabled = mEnabled;
        vstate.mSpindleSpeed = gSpindleSpeed;
        UpdateStepperViz(vstate);

    }
    usleep(1000);
}
void AccelStepper::setCurrentPosition(int pos)
{
    //printf( "Stepper<%s> setCurrentPosition<%d>\n", mName.c_str(), pos );
    mCurrentPosition = pos;
    float dpos = mTargetPosition-mCurrentPosition;
    mDirection = (dpos>0.0) ? 1.0 : (dpos<0.0) ? -1.0 : 0.0;
    resetTimer();
}
int AccelStepper::currentPosition()
{
    return mCurrentPosition;
}
int AccelStepper::distanceToGo()
{
    return mTargetPosition-mCurrentPosition;
}

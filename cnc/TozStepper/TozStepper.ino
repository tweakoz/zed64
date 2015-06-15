///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <Stepper.h>
#include <AccelStepper.h>
#include <math.h>

// TODO - multi-dimensional lerp (as opposed to multi 1 dimensional interps).
//         may have to modify AccelStepper to pull it off


///////////////////////////////////////////////////////////////////////////////

void home();
void calibrate();
void movxz(int,int);
void drwxz(int,int);
void prser(const char *fmt, ... );
int y_mm_to_steps(float in);
float in_to_mm(float in);


const int kmovexz_speed = 300;
const int kmovexz_accel = 200;
const int kdrawxz_speed = 10;
const int kdrawxz_accel = 10;
const int kmovey_speed = 300;
const int kmovey_accel = 300;
const int spindle_pwm_pin = 13;
const int homestep = 1000;
const float drillspeedIPS = 0.02f; // inches per second
bool lockxz = false;
float drill_depth = 0.5f;

float drill_mm_per_second()
{
  return drillspeedIPS*25.4f;
}
int drill_steps_per_second()
{
  return y_mm_to_steps(drill_mm_per_second());
}
///////////////////////////////////////////////////////////////////////////////

struct axis
{
  axis( const char* nam,
        int llo, // limitsw lo
        int lhi,  // limitsw hi
        AccelStepper* accA = 0,
        AccelStepper* accB = 0
       )
    : name(nam)
    , axis_length(0)
    , center(0)
    , current_pos(0)
    , target_pos(0)
    , limitswpin_lo(llo)
    , limitswpin_hi(lhi)
    , stepperA(accA)
    , stepperB(accB)
    , maxspeed(0)
    , accel(0)
  {

  }

  void init();
  void home();
  int step(int stepsiz);
  int readLimitSwitches() const;

  void moveToMax() { moveTo(axis_length); }
  void moveToMin() { moveTo(0); }

  //////////////////////////////

  void disableOut();
  void enableOut();
  void setCurrentPosition(int p);
  int getCurrentPosition();
  void rawMoveTo(int p );
  void moveTo(int p );
  void runStepper();
  int distanceToTarget();
  void calibrate();
  void setSpeedAccel(int spd, int acc);
  bool is_xz();

  int clamp_pos( int pos )
  {
    int min = 100;
    int max = (axis_length-100);
    return (pos<min) ? min 
                     : (pos>max) ? max
                                 : pos;

  }

  //////////////////////////////

  void zero_pos() { center=current_pos; }

  void move_cenrel(int pos) { moveTo(center+pos); }
  void move_rel(int pos) { moveTo(current_pos+pos); }

  //////////////////////////////

  const char* name;
  int center;
  int cal_center;
  int current_pos;
  int target_pos;
  int axis_length;
  const int maxspeed;
  int accel;
  const int limitswpin_lo;
  const int limitswpin_hi;
  AccelStepper* const stepperA;
  AccelStepper* const stepperB;

};

///////////////////////////////////////////////////////////////////////////////

void axis::init()
{
    pinMode(limitswpin_lo,INPUT);
    pinMode(limitswpin_hi,INPUT);
    setCurrentPosition(0);
}
void axis::setSpeedAccel(int maxspd, int acc)
{
  if( stepperA ) stepperA->setMaxSpeed(maxspd);
  if( stepperA ) stepperA->setAcceleration(acc);
  if( stepperB ) stepperB->setMaxSpeed(maxspd);
  if( stepperB ) stepperB->setAcceleration(acc);

}
void axis::enableOut()
{ 
  if( stepperA ) stepperA->enableOutputs();
  if( stepperB ) stepperB->enableOutputs();
}
void axis::disableOut()
{ 
  if( stepperA ) stepperA->disableOutputs();
  if( stepperB ) stepperB->disableOutputs();
}
void axis::setCurrentPosition(int p)
{ 
  if( stepperA ) stepperA->setCurrentPosition(p);
  if( stepperB ) stepperB->setCurrentPosition(p);
}
int axis::getCurrentPosition()
{ 
  if( stepperA ) return stepperA->currentPosition();
  if( stepperB ) return stepperB->currentPosition();
  return 0;
}
void axis::rawMoveTo(int p)
{ 
  if( stepperA ) stepperA->moveTo(p);
  if( stepperB ) stepperB->moveTo(p);
}
void axis::runStepper()
{ 
  if( stepperA ) stepperA->run();
  if( stepperB ) stepperB->run();
}
int axis::distanceToTarget()
{ 
  if( stepperA ) return stepperA->distanceToGo();
  if( stepperB ) return stepperB->distanceToGo();
  return 0;
}
int axis::step(int stpsiz)
{
  prser( "axis<%s> begin step", name );
  bool done = false;

  disableOut();
  setCurrentPosition(0);
  enableOut();
  int total_delta = 0;

  rawMoveTo(stpsiz);

  while( false == done )
  {
    runStepper();
    int msk = (stpsiz>0) ? 2 : 1;
    bool swdone = readLimitSwitches()&msk;
    bool dist_done = ( distanceToTarget() == 0 );
    done = swdone|dist_done;
  }

  total_delta += getCurrentPosition();

  disableOut();
  setCurrentPosition(0);
  prser( "axis<%s> step - limit reached", name );
  current_pos = 0;
  target_pos = 0;
  return total_delta;
}

void axis::home()
{
  prser( "axis<%s> begin home", name );
  bool done = false;

  if( is_xz() )
     setSpeedAccel(250,250);
  else
     setSpeedAccel(350,350);

  while( step(-homestep)<0 ){}

  setCurrentPosition(0);
  current_pos = 0;
  target_pos = 0;

  prser( "axis<%s> end home", name );

}
void axis::calibrate()
{
  bool done = false;

  /////////////////////
  // move inward from limit
  /////////////////////

  axis_length = 0;

  enableOut();
  setCurrentPosition(0);
       
  while( false == done )
  {
    prser( "axis<%s> CALIBINNER", name );
    axis_length += step(homestep);
    done = readLimitSwitches()&2;
    if(false==done)runStepper();
  }

  prser( "axis<%s> measured<%d>", name, axis_length );

  disableOut();

  center = axis_length/2; 
  cal_center = center;
  setCurrentPosition(axis_length);
}
void axis::moveTo(int pos)
{
    pos = clamp_pos(pos);

    prser( ">> moving axis<%s:%d>", name, pos );

    target_pos = pos;

    rawMoveTo(target_pos);

    while( distanceToTarget() != 0 )
      runStepper();

    current_pos = getCurrentPosition();

    if( is_xz() && lockxz )
    {
      enableOut();
    }
    else
    {
      disableOut();
    }
}
int axis::readLimitSwitches() const
{
  int limitlo = digitalRead(limitswpin_lo);
  int limithi = digitalRead(limitswpin_hi);
  return limitlo|(limithi<<1);
}

///////////////////////////////////////////////////////////////////////////////

AccelStepper stepper_y(  AccelStepper::HALF4WIRE, 22, 23, 24, 25); 
AccelStepper stepper_x(  AccelStepper::HALF4WIRE, 28, 29, 30, 31);
AccelStepper stepper_z1( AccelStepper::HALF4WIRE, 34, 35, 36, 37);
AccelStepper stepper_z2( AccelStepper::HALF4WIRE, 40, 41, 42, 43);
axis x_axis("X",48,49,&stepper_x);
axis y_axis("Y",50,51,&stepper_y);
axis z_axis("Z",52,53,&stepper_z1,&stepper_z2);

bool axis::is_xz()
{
  return (stepperA==&stepper_x) || (stepperA==&stepper_z1);
}

///////////////////////////////////////////////////////////////////////////////


struct settings
{

  settings( int xzspd, int xzacc, float xzang,
            int yspd, int yacc )
    : xzspeed(xzspd)
    , xzaccel(xzacc)
    , xz_angle_degrees(xzang)
    , yspeed(yspd)
    , yaccel(yacc)
    {

    }
    void apply_speed();

  int xzspeed;
  int xzaccel;
  int yspeed;
  int yaccel;
  float xz_angle_degrees;

};

settings stg_init(350,350,0.0f,400,400);
settings stg_drill_metal(350,350,0.0f,400,400);
settings stg_mill_metal(50,50,0.0f,200,200);

settings stg_current = stg_init;

///////////////////////////////////////////////////////////////////////////////

void settings::apply_speed()
{
    stepper_x.setMaxSpeed(xzspeed);
    stepper_x.setAcceleration(xzaccel);
    stepper_y.setMaxSpeed(yspeed);
    stepper_y.setAcceleration(yaccel);
    stepper_z1.setMaxSpeed(xzspeed);
    stepper_z1.setAcceleration(xzaccel);
    stepper_z2.setMaxSpeed(xzspeed);
    stepper_z2.setAcceleration(xzaccel);

}


///////////////////////////////////////////////////////////////////////////////

float angle_degrees = 0.0;

int steptop = 1500;
int stepbot = 0;
int stepsiz = 30;

int prvtim = 0;

int num_steps = 6;

int reg_x1 = 0;
int reg_x2 = 0;
int reg_y1 = -900;
int reg_y2 = -2300;
int reg_z1 = 0;
int reg_z2 = 0;

enum
{
  EUNIT_STEPS = 0,
  EUNIT_MM=1,
  EUNIT_CM=2,
  EUNIT_IN=3

} current_units;

float in_to_mm(float in)
{
  return (in*25.4f);
}
int xz_mm_to_steps(float mm) // belt (10.6 fixed)
{
  //10.0 halfsteps per mm
  return int(mm*10.0f);
}
int y_mm_to_steps(float mm) // acme screw
{
  //200 halfsteps per mm
  return int(mm*50.0f);
}
int steps_xz(float units)
{
  int rval = 0;
  switch(current_units)
  {
    case EUNIT_STEPS:
      rval = units;
      break;
    case EUNIT_MM:
      rval = xz_mm_to_steps(units);
      prser( "mm<%f> steps<%d>", units, rval );
      break;
    case EUNIT_CM:
      rval = xz_mm_to_steps(units*10.0);
      prser( "cm<%f> steps<%d>", units, rval );
      break;
    case EUNIT_IN:
      rval = xz_mm_to_steps(in_to_mm(units));
      prser( "in<%f> steps<%d>", units, rval );
      break;
  }
  return rval;
}
int steps_x(float units)
{
  return steps_xz(units);
}
int steps_z(float units)
{
  return steps_xz(units);
}
int steps_y(float units)
{
  int rval = 0;
  switch(current_units)
  {
    case EUNIT_STEPS:
      rval = units;
      break;
    case EUNIT_MM:
      rval = y_mm_to_steps(units);
      prser( "mm<%f> steps<%d>", units, rval );
      break;
    case EUNIT_CM:
      rval = y_mm_to_steps(units*10.0);
      prser( "cm<%f> steps<%d>", units, rval );
      break;
    case EUNIT_IN:
      rval = y_mm_to_steps(in_to_mm(units));
      prser( "in<%f> steps<%d>", units, rval );
      break;
  }
  return rval;
}
///////////////////////////////////////////////////////////////////////////////

void prser(const char *fmt, ... )
{

  char buf[64]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 64, fmt, args);
  va_end (args);
  Serial.println(buf);
}

///////////////////////////////////////////////////////////////////////////////

void usage()
{
  prser( "////////////////////////" );
  prser( "// unknown command" );
  prser( "//  cmds: home" );
  prser( "//  cmds: calib" );
  prser( "//  cmds: top" );
  prser( "//  cmds: / : info" );
  prser( "//  cmds: ? : help" );
  prser( "//  cmds: 0 : zero" );
  prser( "//  cmds: c : goto center" );
  prser( "//  cmds: cx <pos> : move to x rel 2 ctr" );
  prser( "//  cmds: cy <pos> : move to y rel 2 ctr" );
  prser( "//  cmds: cz <pos> : move to z rel 2 ctr" );
  prser( "//  cmds: cxz <pos2> : move xz rel 2 ctr" );
  prser( "//  cmds: kxz <pos2> : convert xz to steps" );
  prser( "//  cmds: s.x1/x2 <pos> : set x1/x2 register" );
  prser( "//  cmds: s.y1/y2 <pos> : set y1/y2 register" );
  prser( "//  cmds: s.z1/z2 <pos> : set z1/z2 register" );
  prser( "//  cmds: s.steps <num_steps>" );
  prser( "//  cmds: s.angle <degrees>" );
  prser( "//  cmds: s.depth <inches>" );
  prser( "//  cmds: x1/x2 : move x to value in x1/x2 register" );
  prser( "//  cmds: y1/y2 : move y to value in y1/y2 register" );
  prser( "//  cmds: z1/z2 : move z to value in z1/z2 register" );
  prser( "//  cmds: spin <spd> : spindle speed" );
  prser( "//  cmds: lockxz <bool> : lockxz axis on idle" );
  prser( "//  cmds: dr : drill a hole at cxz" );
  prser( "//  cmds: dip28 : drill a 28pin dip at cxz" );
  prser( "//  cmds: units.in : set units to inches" );
  prser( "//  cmds: units.cm : set units to centimeters" );
  prser( "//  cmds: units.mm : set units to millimeters" );
  prser( "//  cmds: units.st : set units to steps" );
  prser( "////////////////////////" );
}

void top()
{
  y_axis.setSpeedAccel(500,500);
  y_axis.moveToMax();
}

void dolockxz( int lxz)
{
  prser( "DOLOCKXZ<%d>", lxz );
  
  if( lxz )
  {  stepper_x.enableOutputs();
     stepper_z1.enableOutputs();
     stepper_z2.enableOutputs();
  }
  else
  {  stepper_x.disableOutputs();
     stepper_z1.disableOutputs();
     stepper_z2.disableOutputs();
  }
  lockxz = (bool) lxz ;
}

void spindle_power( int p )
{
  prser("set spindle power<%d>", p );
  analogWrite( spindle_pwm_pin, p );

}
void stepdrill_inner()
{
  prser( "stepdrill depth<%f> steps<%d>)", drill_depth, num_steps );

  float drill_mm_per_sec = drillspeedIPS*25.4f;
  const int drillspeed = y_mm_to_steps(drill_mm_per_sec);

  float dep = abs(steps_y(drill_depth));

  const int top = 0.0f;
  const int bot = -dep;

  int nsteps = (num_steps<2) ? 2 : num_steps;
  float finsteps = 1.0/float(nsteps);
  float dy = bot-top;

  for( int i=0; i<num_steps; i++ )
  {
    float fitop = float(i)*finsteps;
    float fibot = float(i+1)*finsteps;
    float ftop = float(top)+dy*fitop;
    float fbot = float(top)+dy*fibot;
    y_axis.setSpeedAccel(500,500);
    if((i&3)==0)
    { y_axis.move_cenrel(top);
      y_axis.move_cenrel((top+int(ftop))/2);      
    }
    else 
      y_axis.move_cenrel((top+int(ftop))/2);
    y_axis.setSpeedAccel(drillspeed,drillspeed);
    y_axis.move_cenrel(int(fbot));
  }
  y_axis.setSpeedAccel(500,500);
  y_axis.move_cenrel(top);
}
void stepdrill_top()
{
  y_axis.move_cenrel(0.0f);
  spindle_power(100);
  stepdrill_inner();
  spindle_power(0);
  y_axis.move_cenrel(0.0f);
}


void movxz_xf(float x, float z)
{
    float theta = -angle_degrees*.0174532925f;
    float ct = cos(theta);
    float st = sin(theta);
    float xprime = x*ct - z*st;
    float zprime = x*st + z*ct;

    int ix = steps_xz(xprime);
    int iz = steps_xz(zprime);

    prser( "centerxz<%d %d>", x_axis.center, z_axis.center );

    int crix = x_axis.center+ix;
    int criz = z_axis.center+iz;

    movxz(crix,criz);

}
void dip28()
{
  spindle_power(100);
  stepdrill_inner();

  // pins 1..14
  for(int i=0; i<14; i++)
  {
    float fz = float(-i)*0.1f;
    movxz_xf( 0.0f, fz );
    stepdrill_inner();
  }
  // pins 15..28
  for(int i=0; i<14; i++)
  {
    float fz = float(13-i)*0.1f;
    movxz_xf( 0.6f, fz );
    stepdrill_inner();
  }
  top();
  spindle_power(0);
}
///////////////////////////////////////////////////////////////////////////////
// PARSER
///////////////////////////////////////////////////////////////////////////////

void info()
{
  float xzspmm = xz_mm_to_steps(1.0);
  float yspmm = y_mm_to_steps(1.0);
  float dimxmm = float(x_axis.axis_length)/xzspmm;
  float dimzmm = float(z_axis.axis_length)/xzspmm;
  float dimymm = float(y_axis.axis_length)/yspmm;
  float dimxin = dimxmm/25.4f;
  float dimyin = dimymm/25.4f;
  float dimzin = dimzmm/25.4f;
  float cposx = x_axis.current_pos-x_axis.center;
  float cposy = y_axis.current_pos-y_axis.center;
  float cposz = z_axis.current_pos-z_axis.center;
  float cctrx = x_axis.center;
  float cctry = y_axis.center;
  float cctrz = z_axis.center;
  float cposxmm = cposx/xzspmm;
  float cposymm = cposy/yspmm;
  float cposzmm = cposz/xzspmm;
  float cposxin = cposxmm/25.4;
  float cposyin = cposymm/25.4;
  float cposzin = cposzmm/25.4;
  float cctrxmm = cctrx/xzspmm;
  float cctrymm = cctry/yspmm;
  float cctrzmm = cctrz/xzspmm;
  float cctrxin = cctrxmm/25.4;
  float cctryin = cctrymm/25.4;
  float cctrzin = cctrzmm/25.4;

  prser("////////////////////////////");
  prser("dimx steps<%d> mm<%f> in<%f>", x_axis.axis_length,dimxmm,dimxin);
  prser("dimz steps<%d> mm<%f> in<%f>", z_axis.axis_length,dimzmm,dimzin);
  prser("dimy steps<%d> mm<%f> in<%f>", y_axis.axis_length,dimymm,dimyin);
  prser("////////////////////////////");
  prser("ctrx steps<%d> mm<%f> in<%f>", int(cctrx), cctrxmm, cctrxin );
  prser("ctrz steps<%d> mm<%f> in<%f>", int(cposz), cctrzmm, cctrzin );
  prser("ctry steps<%d> mm<%f> in<%f>", int(cposy), cctrymm, cctryin );
  prser("////////////////////////////");
  prser("curx steps<%d> mm<%f> in<%f>", int(cposx), cposxmm, cposxin );
  prser("curz steps<%d> mm<%f> in<%f>", int(cposz), cposzmm, cposzin );
  prser("cury steps<%d> mm<%f> in<%f>", int(cposy), cposymm, cposyin );
  prser("////////////////////////////");
  prser("drillspeed steps/sec<%d> mm/sec<%f> in/sec<%f>", drill_steps_per_second(), drill_mm_per_second(), drillspeedIPS );
  prser("drilldepth steps<%d> mm<%f> in<%f>", y_mm_to_steps(in_to_mm(drill_depth)), in_to_mm(drill_depth), drill_depth );
  prser("////////////////////////////");
}

void ready()
{
    info();
    prser( "ready.. ? : help" );
}


// the loop routine runs over and over again forever:
void parse()
{
  int curtim = millis();
  int delta = curtim-prvtim;

  if( delta<100 )
    return;

  prvtim = curtim;

  static const int INPUT_SIZE=32;

  // Calculate based on max input size expected for one command

  // Get next command from Serial (add 1 for final 0)
  char input[INPUT_SIZE + 1];

  byte size = Serial.readBytes(input, INPUT_SIZE);

  //prser( "size<%d>", size );
  if( (size!=0) )
  {
    // Add the final 0 to end the C string
    input[size] = 0;

    // Read each command line
    char* command = strtok(input, "\n");
    while (command != 0)
    {
      // Split the command in 2 values
      char* arg1 = strchr(command, ' ');

////////////////////////////////
// 0 arg commands
////////////////////////////////

      if (arg1 == 0) 
      {

        if( 0 == strcmp(command, "/") )
          info();
        else if( 0 == strcmp(command, "c") )
        {
          x_axis.moveTo(x_axis.center);
          z_axis.moveTo(z_axis.center);
          y_axis.moveTo(y_axis.center);
        }
        else if( 0 == strcmp(command, "top") )
          y_axis.moveToMax();
        else if( 0 == strcmp(command, "bottom") )
          y_axis.moveToMin();
        else if( 0 == strcmp(command, "left") )
          x_axis.moveToMin();
        else if( 0 == strcmp(command, "right") )
          x_axis.moveToMax();
        else if( 0 == strcmp(command, "front") )
          z_axis.moveToMin();
        else if( 0 == strcmp(command, "back") )
          z_axis.moveToMax();
        else if( 0 == strcmp(command, "home") )
          home();
        else if( 0 == strcmp(command, "calib") )
          calibrate();
        else if( 0 == strcmp(command, "dr") )
          stepdrill_top();
        else if( 0 == strcmp(command, "dip28") )
          dip28();
        else if( 0 == strcmp(command, "units.in") )
        {
          current_units = EUNIT_IN;
          prser( "Set units to inches");
        }
        else if( 0 == strcmp(command, "units.cm") )
        {
          current_units = EUNIT_CM;
          prser( "Set units to centimeters");
        }
        else if( 0 == strcmp(command, "units.mm") )
        {
          current_units = EUNIT_MM;
          prser( "Set units to millimeters");
        }
        else if( 0 == strcmp(command, "units.st") )
        {
          current_units = EUNIT_STEPS;
          prser( "Set units to steps");
        }
        else if( 0 == strcmp(command, "x1") )
          x_axis.move_cenrel(reg_x1);
        else if( 0 == strcmp(command, "x2") )
          x_axis.move_cenrel(reg_x2);
        else if( 0 == strcmp(command, "y1") )
          y_axis.move_cenrel(reg_y1);
        else if( 0 == strcmp(command, "y2") )
          y_axis.move_cenrel(reg_y2);
        else if( 0 == strcmp(command, "z1") )
          z_axis.move_cenrel(reg_z1);
        else if( 0 == strcmp(command, "z2") )
          z_axis.move_cenrel(reg_z2);
        else if( 0 == strcmp(command, "0") )
        {
            x_axis.zero_pos();
            y_axis.zero_pos();
            z_axis.zero_pos();
        }
        else
          usage();

        ready();

      }
      else //if (arg1 != 0)
      {
        // Actually split the string in 2: replace ':' with 0
        *arg1 = 0;
        ++arg1;

        int arg1_as_int = atoi(arg1);
        float arg1_as_float = atof(arg1);

        char* arg2 = strchr(arg1, ' ');

////////////////////////////////
// 1 arg commands
////////////////////////////////

        if (arg2 == 0) // 1 arg
        {
          if( 0 == strcmp(command,"spin" ) )
            spindle_power(arg1_as_int);
          else if( 0 == strcmp(command,"cx" ) )
            x_axis.move_cenrel(arg1_as_int);
          else if( 0 == strcmp(command,"cy" ) )
            y_axis.move_cenrel(steps_y(arg1_as_float));
          else if( 0 == strcmp(command,"cz" ) )
            z_axis.move_cenrel(arg1_as_int);
          else if( 0 == strcmp(command,"ny" ) )
          {
            int iy = steps_y(arg1_as_float);
            y_axis.center = iy;
            y_axis.move_cenrel(0);
          }
          else if( 0 == strcmp(command,"dx" ) )
            x_axis.move_rel(steps_x(arg1_as_float));
          else if( 0 == strcmp(command,"dy" ) )
            y_axis.move_rel(steps_y(arg1_as_float));
          else if( 0 == strcmp(command,"dz" ) )
            z_axis.move_rel(steps_z(arg1_as_float));
          else if( 0 == strcmp(command,"lockxz") )
            dolockxz(arg1_as_int);
          else if( 0 == strcmp(command,"s.depth") )
          {
            drill_depth = arg1_as_float;
          }
          else if( 0 == strcmp(command,"s.x1" ) )
          {
            int ix = steps_xz(arg1_as_float);
            reg_x1 = ix;
            prser( "x1 set to val<%f> steps<%d>", arg1_as_float, ix );
          }
          else if( 0 == strcmp(command,"s.x2" ) )
          {
            int ix = steps_xz(arg1_as_float);
            reg_x2 = ix;
            prser( "x2 set to val<%f> steps<%d>", arg1_as_float, ix );
          }
          else if( 0 == strcmp(command,"s.y1" ) )
          {
            int iy = steps_y(arg1_as_float);
            reg_y1 = iy;
            prser( "y1 set to val<%f> steps<%d>", arg1_as_float, iy );
          }
          else if( 0 == strcmp(command,"s.y2" ) )
          {
            int iy = steps_y(arg1_as_float);
            reg_y2 = iy;
            prser( "y2 set to val<%f> steps<%d>", arg1_as_float, iy );
          }
          else if( 0 == strcmp(command,"s.z1" ) )
          {
            int iz = steps_xz(arg1_as_float);
            reg_z1 = iz;
            prser( "z1 set to val<%f> steps<%d>", arg1_as_float, iz );
          }
          else if( 0 == strcmp(command,"s.z2" ) )
          {
            int iz = steps_xz(arg1_as_float);
            reg_z2 = iz;
            prser( "z2 set to val<%f> steps<%d>", arg1_as_float, iz );
          }
          else if( 0 == strcmp(command,"s.angle" ) )
          {
            angle_degrees = arg1_as_float;
            prser( "angle set to val<%f>", arg1_as_float );
          }
          else if( 0 == strcmp(command, "steptop") )
          {
            steptop = arg1_as_int;
            prser( "STEPTOP<%d>", steptop );
          }
          else if( 0 == strcmp(command, "s.steps") )
          {
            num_steps = arg1_as_int;
            prser( "numsteps<%d>", num_steps );
          }
          else
            usage();

          ready();

        }
        else // 2 args
        {
          *arg2 = 0;
          ++arg2;

          int arg2_as_int = atoi(arg2);
          float arg2_as_float = atof(arg2);

          if( 0 == strcmp(command,"axz" ) )
            movxz(arg1_as_int, arg2_as_int);
          else if( 0 == strcmp(command,"cxz" ) )
          {
            movxz_xf( arg1_as_float, arg2_as_float );
          }
          else if( 0 == strcmp(command,"kxz" ) )
          {
            int ix = steps_xz(arg1_as_float);
            int iz = steps_xz(arg2_as_float);
          }
          else if( 0 == strcmp(command,"nxz" ) )
          {
            int ix = steps_xz(arg1_as_float)+x_axis.cal_center;
            int iz = steps_xz(arg2_as_float)+z_axis.cal_center;
            x_axis.center = ix;
            z_axis.center = iz;
            movxz(ix,iz);
          }
          else
            usage();

          ready();

        }

      }

      // Find the next command in input string
      command = strtok(0, "\n");

    }

  }
}

///////////////////////////////////////////////////////////////////////////////

void movxz_inner( int xpos, int zpos )
{
    xpos = x_axis.clamp_pos(xpos);
    zpos = z_axis.clamp_pos(zpos);

    bool done = false;

    prser( ">> movingXZ<%d,%d>", xpos, zpos );

    x_axis.target_pos = xpos;
    z_axis.target_pos = zpos;

    x_axis.enableOut();
    z_axis.enableOut();

    x_axis.moveTo(x_axis.target_pos);
    z_axis.moveTo(z_axis.target_pos);

    while( false == done )
    {
      x_axis.current_pos = x_axis.getCurrentPosition();
      z_axis.current_pos = z_axis.getCurrentPosition();

      done = (x_axis.distanceToTarget() == 0)
          && (z_axis.distanceToTarget() == 0);

      if( false==done )
      {
        x_axis.runStepper();
        y_axis.runStepper();
      }

    }
    if( lockxz )
    {  x_axis.enableOut();
       z_axis.enableOut();
    }
    else
    {  x_axis.disableOut();
       z_axis.disableOut();
    }
}

void movxz( int xpos, int zpos )
{
  x_axis.setSpeedAccel(kmovexz_speed,kmovexz_accel);
  z_axis.setSpeedAccel(kmovexz_speed,kmovexz_accel);
  movxz_inner(xpos,zpos);
}
void drwxz( int xpos, int zpos )
{
  x_axis.setSpeedAccel(kdrawxz_speed,kdrawxz_accel);
  z_axis.setSpeedAccel(kdrawxz_speed,kdrawxz_accel);
  movxz_inner(xpos,zpos);
}


///////////////////////////////////////////////////////////////////////////////

void home()
{
  x_axis.home();
  y_axis.home();
  z_axis.home();  
}
void calibrate()
{
  y_axis.setSpeedAccel(400,400);
  y_axis.step(10000);

  x_axis.home();
  x_axis.calibrate();
  z_axis.home();
  z_axis.calibrate();

  y_axis.home();
  y_axis.calibrate();
}

///////////////////////////////////////////////////////////////////////////////

void setup()
{
    current_units = EUNIT_IN;
    // nothing to do inside the setup
    Serial.begin(115200);

    x_axis.init();
    y_axis.init();
    z_axis.init();

    pinMode( spindle_pwm_pin, OUTPUT );
  
    prvtim = millis();

    spindle_power(0);

}

void phase2init()
{
    calibrate();
    ready();
}

///////////////////////////////////////////////////////////////////////////////

bool firstloop = true;

void loop()
{
  if( firstloop )
  {
    phase2init();
    firstloop = false;
  }
  parse();
}

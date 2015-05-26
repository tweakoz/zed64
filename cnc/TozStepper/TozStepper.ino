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

AccelStepper stepper_y(AccelStepper::HALF4WIRE, 22, 23, 24, 25);

AccelStepper stepper_x(AccelStepper::HALF4WIRE, 28, 29, 30, 31);
AccelStepper stepper_z1(AccelStepper::HALF4WIRE, 34,35,36,37);
AccelStepper stepper_z2(AccelStepper::HALF4WIRE, 40,41,42,43);

///////////////////////////////////////////////////////////////////////////////

int yspeed = 400;
int yaccel = 400;


const int initspeed = 200;
const int initaccel = 2500;
const int homestep = 50;
const int maxaccel = 1200;
const int maxspeed = 700;
const int slospeed = 100;
const int slozonsiz = 250;

int steptop = 1500;
int stepbot = 0;
int stepsiz = 30;

int stepCount = 0;  // number of steps the motor has taken

// limit switches - 48 - 53

int limitsw_pinx1 = 48;
int limitsw_pinx2 = 49;

int limitsw_piny1 = 50;
int limitsw_piny2 = 51;

int limitsw_pinz1 = 52;
int limitsw_pinz2 = 53;

int spindle_pwm_pin = 13;

int prvtim = 0;
int has_homed = 0;

bool lockxz = false;

int current_command = 0;
int previous_command = 1;

int target_x = 0;
int target_y = 0;
int target_z = 0;

int current_x = 0;
int current_y = 0;
int current_z = 0;

int xaxislength = 2586;
int yaxislength = 100;
int zaxislength = 2156;

int center_x()
{
  return xaxislength/2;
}
int center_y()
{
  return yaxislength/2;
}
int center_z()
{
  return zaxislength/2;
}
///////////////////////////////////////////////////////////////////////////////

void prser(char *fmt, ... )
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
  prser( "//  cmds: testpat" );
  prser( "//  cmds: ax <pos> : move to absolute x" );
  prser( "//  cmds: az <pos> : move to absolute z" );
  prser( "//  cmds: cx <pos> : move to x rel 2 ctr" );
  prser( "//  cmds: cz <pos> : move to z rel 2 ctr" );
  prser( "//  cmds: cxz <pos2> : move xz rel 2 ctr" );
  prser( "//  cmds: spin <spd> : spindle speed" );
  prser( "//  cmds: lockxz <bool> : lockxz axis on idle" );
  prser( "//  cmds: s.xz <pos>" );
  prser( "//  cmds: s.y <pos>" );
  prser( "//  cmds: stepdrill" );
  prser( "//  cmds: stepline <x>" );
  prser( "////////////////////////" );
}

void speedxz(int sp)
{
    stepper_z1.setMaxSpeed(sp);
    stepper_z2.setMaxSpeed(sp);
    stepper_x.setMaxSpeed(sp);
}
void speedy(int sp)
{
    stepper_y.setMaxSpeed(sp);
    stepper_y.setAcceleration(sp);
    yspeed = sp;
    yaccel = sp;
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
void slowspeed()
{
    stepper_x.setMaxSpeed(initspeed);
    stepper_x.setAcceleration(initaccel);
    stepper_y.setMaxSpeed(yspeed);
    stepper_y.setAcceleration(yaccel);
    stepper_z1.setMaxSpeed(initspeed);
    stepper_z1.setAcceleration(initaccel);
    stepper_z2.setMaxSpeed(initspeed);
    stepper_z2.setAcceleration(initaccel);

}

void highspeed()
{
  stepper_x.setAcceleration(maxaccel);
  stepper_y.setAcceleration(yspeed/10);
  stepper_z1.setAcceleration(maxaccel);
  stepper_z2.setAcceleration(maxaccel);

  stepper_x.setMaxSpeed(maxspeed);
  stepper_y.setMaxSpeed(yaccel/10);
  stepper_z1.setMaxSpeed(maxspeed);
  stepper_z2.setMaxSpeed(maxspeed);

}

void spindle_power( int p )
{
  prser("set spindle power<%d>", p );
  analogWrite( spindle_pwm_pin, p );

}
void stepdrill()
{
  prser( "stepdrill from_y<%d> to_y<%d> step_y<%d)", steptop, stepbot, stepsiz );
  movy(steptop);
  int top = steptop;
  int bot = steptop-stepsiz;
  while( bot>stepbot )
  {
    stepper_y.setMaxSpeed(yspeed);
    stepper_y.setAcceleration(yaccel);
    movy(bot);

    stepper_y.setMaxSpeed(500);
    stepper_y.setAcceleration(500);

    int dtb = (steptop-bot);
    int top = bot+(dtb/2);
    movy(top);
    top -= stepsiz;
    bot -= stepsiz;
  }
  movy(steptop);
}
void stepline(int x)
{
  prser( "stepline x<%d>", x );
  for( int z=400; z<=700; z+=50 )
  { movxz(center_x()+x,center_z()+z);
    stepdrill();
  }
}

///////////////////////////////////////////////////////////////////////////////
// PARSER
///////////////////////////////////////////////////////////////////////////////

void ready()
{
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
        
        if( 0 == strcmp(command, "pxyz") )
        {
          int cx = (target_x-center_x());
          int cy = (target_y-center_y());
          int cz = (target_z-center_z());
          prser( "print xyz" );
          prser( " abs <x:%d z:%d y:%d>", target_x, target_z, target_y );
          prser( " ctr <x:%d z:%d y:%d>", cx, cz, cy );
        }
        else if( 0 == strcmp(command, "home") )
        {
          //prser( ">> going home" );
          home();
        }
        else if( 0 == strcmp(command, "calib") )
        {
          //prser( ">> going home" );
          calibrate();
        }
        else if( 0 == strcmp(command, "testpat") )
        {
          //prser( ">> going home" );
          test_pattern();
        }
          else if( 0 == strcmp(command, "stepdrill") )
          {
            stepdrill();
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

        char* arg2 = strchr(arg1, ' ');

////////////////////////////////
// 1 arg commands
////////////////////////////////

        if (arg2 == 0) // 1 arg
        {
          if( 0 == strcmp(command,"spin" ) )
            spindle_power(arg1_as_int);
          else if( 0 == strcmp(command,"ax" ) )
            movx(arg1_as_int);
          else if( 0 == strcmp(command,"ay" ) )
            movy(arg1_as_int);
          else if( 0 == strcmp(command,"cx" ) )
            movx(center_x()+arg1_as_int);
          else if( 0 == strcmp(command,"az" ) )
            movz(arg1_as_int);
          else if( 0 == strcmp(command,"cz" ) )
            movx(center_z()+arg1_as_int);
          else if( 0 == strcmp(command,"s.xz" ) )
            speedxz(arg1_as_int);
          else if( 0 == strcmp(command,"s.y" ) )
            speedy(arg1_as_int);
           else if( 0 == strcmp(command,"lockxz") )
            dolockxz(arg1_as_int);
          else if( 0 == strcmp(command, "steptop") )
          {
            steptop = arg1_as_int;
            prser( "STEPTOP<%d>", steptop );
          }
          else if( 0 == strcmp(command, "stepsiz") )
          {
            stepsiz = arg1_as_int;
            prser( "STEPSIZ<%d>", stepsiz );
          }
          else if( 0 == strcmp(command, "stepline") )
          {
            stepline(arg1_as_int);
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

          if( 0 == strcmp(command,"axz" ) )
            movxz(arg1_as_int, arg2_as_int);
          if( 0 == strcmp(command,"cxz" ) )
            movxz(center_x()+arg1_as_int, center_z()+arg2_as_int);
          else
            usage();

          ready();


////////////////////////////////
// 1 arg commands
////////////////////////////////

        }

      }

      // Find the next command in input string
      command = strtok(0, "\n");

    }

  }

  previous_command = current_command;
}

///////////////////////////////////////////////////////////////////////////////

int read_limit_mask()
{
  int limitx1 = digitalRead(limitsw_pinx1);
  int limitx2 = digitalRead(limitsw_pinx2);
  int limity1 = digitalRead(limitsw_piny1);
  int limity2 = digitalRead(limitsw_piny2);
  int limitz1 = digitalRead(limitsw_pinz1);
  int limitz2 = digitalRead(limitsw_pinz2);

  int mask = (limitx1)|(limitx2<<1)|(limity1<<2)|(limity2<<3)|(limitz1<<4)|(limitz2<<5);

  //prser( "limmask<%d>", mask );

  return mask;
}

///////////////////////////////////////////////////////////////////////////////

void begin_idle()
{
  if( lockxz )
  {  stepper_x.enableOutputs();
    stepper_z1.enableOutputs();
    stepper_z2.enableOutputs();
  }
  else
  {  stepper_x.disableOutputs();
    stepper_z1.disableOutputs();
    stepper_z2.disableOutputs();
  }
}
void update_idle()
{

}
void end_idle()
{
}

///////////////////////////////////////////////////////////////////////////////
void home_n( AccelStepper& stepper, int mask )
{
  bool done = false;

  stepper.disableOutputs();
  stepper.setCurrentPosition(0);
  stepper.enableOutputs();

  while( false == done )
  {
    if( stepper.distanceToGo() == 0 )
    {  stepper.setCurrentPosition(0);
       stepper.moveTo(-homestep);
    }

    stepper.run();

    done = read_limit_mask()&mask;
  }
       
  stepper.disableOutputs();
  stepper.setCurrentPosition(0);

}

void home_x()
{
  prser( "begin homex" );
  home_n( stepper_x,1 );
  prser( "homing x limit reached" );
  current_x = 0;
  target_x = 0;
}

void home_y()
{
  prser( "begin homey" );
  home_n( stepper_y,4 );
  prser( "homing y limit reached" );
  current_y = 0;
  target_y = 0;
}
void home_z()
{
  bool done = false;

  prser( "begin homez" );

  stepper_z1.disableOutputs();
  stepper_z2.disableOutputs();
  stepper_z1.setCurrentPosition(0);
  stepper_z2.setCurrentPosition(0);
  stepper_z1.enableOutputs();
  stepper_z2.enableOutputs();

  while( false == done )
  {
    if( stepper_z1.distanceToGo() == 0 )
    {  stepper_z1.setCurrentPosition(0);
       stepper_z2.setCurrentPosition(0);
       stepper_z1.moveTo(-homestep);
       stepper_z2.moveTo(-homestep);
    }

    stepper_z1.run();
    stepper_z2.run();

    done = read_limit_mask()&16;
  }
       
  prser( "homing z limit reached" );

  current_z = 0;
  target_z = 0;

  stepper_z1.disableOutputs();
  stepper_z2.disableOutputs();
  stepper_z1.setCurrentPosition(0);
  stepper_z2.setCurrentPosition(0);

}



///////////////////////////////////////////////////////////////////////////////

void calibrate_x()
{
  bool done = false;

  /////////////////////
  // move inward from limit
  /////////////////////

  xaxislength = homestep;

  stepper_x.enableOutputs();
  stepper_x.moveTo(xaxislength);
       
  while( false == done )
  {
    if( stepper_x.distanceToGo() == 0 )
    {  stepper_x.moveTo(xaxislength);
       xaxislength += homestep;
    }
    stepper_x.run();
    done = read_limit_mask()&2;
  }

  xaxislength = stepper_x.currentPosition();

  prser( "xaxis measured<%d>", xaxislength );

  stepper_x.disableOutputs();
 
}

void calibrate_y()
{
  bool done = false;

  /////////////////////
  // move inward from limit
  /////////////////////

  yaxislength = homestep;

  stepper_y.disableOutputs();
  stepper_y.setCurrentPosition(0);
  stepper_y.enableOutputs();
  stepper_y.moveTo(homestep);
       
  while( false == done )
  {
    if( stepper_y.distanceToGo() == 0 )
    {  stepper_y.moveTo(yaxislength);
       yaxislength += homestep;
    }
    stepper_y.run();
    done = read_limit_mask()&8;
  }

  yaxislength = stepper_y.currentPosition();

  prser( "yaxis measured<%d>", yaxislength );

  stepper_y.disableOutputs();

}

void calibrate_z()
{
  bool done = false;

  /////////////////////
  // move inward from limit
  /////////////////////

  zaxislength = homestep;

  stepper_z1.enableOutputs();
  stepper_z2.enableOutputs();
  stepper_z1.moveTo(homestep);
  stepper_z2.moveTo(homestep);
       
  while( false == done )
  {
    if( stepper_z1.distanceToGo() == 0 )
    {  stepper_z1.moveTo(zaxislength);
       stepper_z2.moveTo(zaxislength);
       zaxislength += homestep;
    }
    stepper_z1.run();
    stepper_z2.run();
    done = read_limit_mask()&32;
  }

  zaxislength = stepper_z1.currentPosition();

  prser( "zaxis measured<%d>", zaxislength );

  stepper_z1.disableOutputs();
  stepper_z2.disableOutputs();

}


void test_pattern()
{
  highspeed();

  for( int i=0; i<100; i++ )
  {

    int rx = rand()%((xaxislength*2)/3);
    int ry = rand()%((yaxislength*2)/3);
    int rz = rand()%((zaxislength*2)/3);

    int x = xaxislength/6+rx;
    int y = yaxislength/6+ry;
    int z = zaxislength/6+rz;


    movxz(x,z);
    movy(y);
    spindle_power(rand()%255);
  }
  movxz(xaxislength/2,yaxislength/2);
  movy(yaxislength);
  spindle_power(0);
}

///////////////////////////////////////////////////////////////////////////////
int clampz( int zpos )
{
    if( zpos<100 )
      zpos=100;

    if( zpos>(zaxislength-100) )
      zpos = zaxislength-100;

    return zpos;
}
int clampx( int xpos )
{
    if( xpos<100 )
      xpos=100;

    if( xpos>(xaxislength-100) )
      xpos = xaxislength-100;

    return xpos;
}
int clampy( int ypos )
{
    if( ypos<100 )
      ypos=100;

    if( ypos>(yaxislength-100) )
      ypos = yaxislength-100;

    return ypos;
}

///////////////////////////////////////////////////////////////////////////////

void movx(int xpos)
{
    xpos = clampx(xpos);

    bool done = false;

    prser( ">> movingX<%d>", xpos );

    target_x = xpos;

    stepper_x.moveTo(target_x);

    while( false == done )
    {
      current_x = stepper_x.currentPosition();

      done = (stepper_x.distanceToGo() == 0);

      if( false==done )
        stepper_x.run();

    }
    if( lockxz )
      stepper_x.enableOutputs();
    else
      stepper_x.disableOutputs();

}

///////////////////////////////////////////////////////////////////////////////

void movy(int ypos)
{
    ypos = clampy(ypos);

    bool done = false;

    prser( ">> movingY<%d>", ypos );

    target_y = ypos;
    stepper_y.enableOutputs();

    stepper_y.moveTo(target_y);

    while( false == done )
    {
      current_y = stepper_y.currentPosition();

      done = (stepper_y.distanceToGo() == 0);

      if( false==done )
        stepper_y.run();

    }
    stepper_y.disableOutputs();

}

///////////////////////////////////////////////////////////////////////////////

void movz(int zpos)
{
    zpos = clampz(zpos);

    bool done = false;

    prser( ">> movingZ<%d>", zpos );

    target_z = zpos;

    stepper_z1.moveTo(target_z);
    stepper_z2.moveTo(target_z);

    while( false == done )
    {
      current_z = stepper_z1.currentPosition();

      done = (stepper_z1.distanceToGo() == 0);

      if( false==done )
      { stepper_z1.run();
        stepper_z2.run();
      }

    }

    if( lockxz )
    {
      stepper_z1.enableOutputs();
      stepper_z2.enableOutputs();
    }
    else
    {
      stepper_z1.disableOutputs();
      stepper_z2.disableOutputs();
    }
}

///////////////////////////////////////////////////////////////////////////////

void movxz( int xpos, int zpos )
{
    xpos = clampz(xpos);
    zpos = clampz(zpos);

    bool done = false;

    prser( ">> movingXZ<%d,%d>", xpos, zpos );

    target_x = xpos;
    target_z = zpos;

    //stepper_x.setSpeed(maxspeed);
    //stepper_z1.setSpeed(maxspeed);
    //stepper_z2.setSpeed(maxspeed);

   stepper_x.enableOutputs();
   stepper_z1.enableOutputs();
   stepper_z2.enableOutputs();

    stepper_x.moveTo(target_x);
    stepper_z1.moveTo(target_z);
    stepper_z2.moveTo(target_z);

    while( false == done )
    {
      current_x = stepper_x.currentPosition();
      current_z = stepper_z1.currentPosition();

      done = (stepper_x.distanceToGo() == 0)
          && (stepper_z1.distanceToGo() == 0);

      if( false==done )
      {
        stepper_x.run();
        stepper_z1.run();
        stepper_z2.run();
      }

    }
    if( lockxz )
    {  stepper_x.enableOutputs();
       stepper_z1.enableOutputs();
       stepper_z2.enableOutputs();
    }
    else
    {  stepper_x.disableOutputs();
       stepper_z1.disableOutputs();
       stepper_z2.disableOutputs();
    }
}

///////////////////////////////////////////////////////////////////////////////

void home()
{
    while(0)
    {   int msk = read_limit_mask();

        delay(2000);
      }

    home_x();
    home_y();
    home_z();
}

void calibrate()
{
  prser( "calibrating" );

  slowspeed();

  home();
  calibrate_x();
  calibrate_y();
  calibrate_z();


  //movxz( xaxislength/2, zaxislength/2 );

}

///////////////////////////////////////////////////////////////////////////////

void setup()
{
    // nothing to do inside the setup
    Serial.begin(115200);

    stepper_x.setCurrentPosition(0);
    stepper_y.setCurrentPosition(0);
    stepper_z1.setCurrentPosition(0);
    stepper_z2.setCurrentPosition(0);


    pinMode(limitsw_piny1,INPUT);
    pinMode(limitsw_piny2,INPUT);
    pinMode(limitsw_pinx1,INPUT);
    pinMode(limitsw_pinx2,INPUT);
    pinMode(limitsw_pinz1,INPUT);
    pinMode(limitsw_pinz2,INPUT);

    pinMode( spindle_pwm_pin, OUTPUT );
  
    prvtim = millis();
    //pinMode(9, OUTPUT);

    spindle_power(0);

    slowspeed();

    calibrate();

    highspeed();

    ready();

}

///////////////////////////////////////////////////////////////////////////////

void loop()
{
  parse();
}



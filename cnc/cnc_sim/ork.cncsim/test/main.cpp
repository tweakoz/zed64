#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>
#include <AccelStepper.h>

void loop();
void setup();
//fd_set readset;
//struct timeval tv;
//struct termios ttystate, ttysave;
#ifdef __APPLE__
#include <GLUT/glut.h>          /* Open GL Util    APPLE */
#else
#include <GL/glut.h>            /* Open GL Util    OpenGL*/
#endif

//////////

#include <ork/timer.h>
#include <ork/opq.h>
#include <ork/mutex.h>
#include <ork/cvector3.h>
#include <ork/cvector4.h>
#include <ork/fixedstring.h>

using namespace ork;

void Render();
void Resize(int w, int h);
void RenderThreadUpdate();

int width = 1280;
int height = 720;

Timer timer;

struct window
{
        window(int argc,char** argv)
                : omq(64)
        {
            mopg = omq.CreateOpGroup("l0");

            glutInit(&argc, argv);
            glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
            glutInitWindowSize(width, height);
            glutInitWindowPosition(200, 50);
            glutCreateWindow("ZedCncSimulator");
            glutDisplayFunc(Render);
            glutReshapeFunc(Resize);
            glutIdleFunc(RenderThreadUpdate);

            t.Start();

        }

        void run_loop()
        {
             glutMainLoop();
        }

        ~window()
        {
        }



        OpMultiQ omq;
        OpGroup* mopg;
        //render_graph mRG;
        Timer t;
};

window* gwin = nullptr;

void Resize(int w, int h)
{
    width = w;
    height = h;
    //gwin->mRG.Resize(width,height);
}

////////////////////////////////////////
// called from update thread
////////////////////////////////////////

StepperVizState vs_x, vs_y, vs_z1, vs_z2;
Mutex mtx;

void UpdateStepperViz(StepperVizState vizstate)
{
    mtx.Lock();
    if( vizstate.mName == "X" )
        vs_x = vizstate;
    if( vizstate.mName == "Y" )
        vs_y = vizstate;
    if( vizstate.mName == "Z1" )
        vs_z1 = vizstate;
    if( vizstate.mName == "Z2" )
        vs_z2 = vizstate;
    mtx.Unlock();
}

////////////////////////////////////////

float fphase = 0.0f;
float spindle_ang = 0.0;
extern float gSpindleSpeed;

void RenderThreadUpdate()
{
    fphase += 1.f;
    spindle_ang += 60.0f*gSpindleSpeed;
    glutPostRedisplay();

}

////////////////////////////////////////

void drawstring2d( const char* string, float sx, float sy, float scale )
{
    float scx = sx;
    float scy = sy;

    for( int i=0; ; i++ )
    {
        if( (string[i]>=32) && (string[i]<=127) ) // in range for font?
        {
            int char_w = glutStrokeWidth( GLUT_STROKE_MONO_ROMAN, string[i] );

            scx += char_w*scale;

            glPushMatrix();
            glTranslatef( scx, scy, 0.0f );
            glScalef( scale, scale, 0 );
            glutStrokeCharacter( GLUT_STROKE_MONO_ROMAN, string[i] );
            glPopMatrix();

        }
        else if( string[i] == 0x0D ) // handle newline
        {
            scy += scale;
            scx = sx;
        }
        else break;
    }

}

fvec3 v3vtx( const fvec3& vx, const fvec3& vy, const fvec3& vz )
{
    return fvec3( vx.GetX(),vy.GetY(), vz.GetZ() );
}
void v3glvtx( const fvec3& vx, const fvec3& vy, const fvec3& vz )
{
    auto v = v3vtx(vx,vy,vz);
    glVertex3fv( v.GetArray() );
}
void v3quad( const fvec3& va,
             const fvec3& vb,
             const fvec3& vc,
             const fvec3& vd )
{
    v3glvtx( va, va, va );
    v3glvtx( vb, vb, vb );
    v3glvtx( vc, vc, vc );

    v3glvtx( va, va, va );
    v3glvtx( vc, vc, vc );
    v3glvtx( vd, vd, vd );
}
void DrawSolidBox( fvec3 ilo, fvec3 ihi, fvec3 color )
{
    fvec3 lo,hi;
    lo.SetX(std::min(ilo.GetX(),ihi.GetX()));
    lo.SetY(std::min(ilo.GetY(),ihi.GetY()));
    lo.SetZ(std::min(ilo.GetZ(),ihi.GetZ()));
    hi.SetX(std::max(ilo.GetX(),ihi.GetX()));
    hi.SetY(std::max(ilo.GetY(),ihi.GetY()));
    hi.SetZ(std::max(ilo.GetZ(),ihi.GetZ()));

    glEnable(GL_LIGHTING);
    //glDisable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    fvec4 white(1.0,1.0,1.0,1.0);
    fvec4 diffuse(color,1.0);
    fvec4 ambm(0.5,0.5,0.5,1.0);
    fvec4 ambl(0.1,0.1,0.2,1.0);
    fvec4 spec(0.0,0.0,0.0,1.0);
    fvec4 lpos( 0.0,5000.0,10000.0);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse.GetArray());
    glMaterialfv(GL_FRONT,GL_SPECULAR,spec.GetArray());
    glMaterialfv(GL_FRONT,GL_AMBIENT,ambm.GetArray());
    //glMaterialfv(GL_LIGHT0,GL_POSITION,lpos.GetArray());


    glLightfv(GL_LIGHT0,GL_POSITION,lpos.GetArray());
    glLightfv(GL_LIGHT0,GL_DIFFUSE,white.GetArray());
    glLightfv(GL_LIGHT0,GL_AMBIENT,ambl.GetArray());

    auto normpx = fvec3(1,0,0);
    auto normnx = fvec3(-1,0,0);
    auto normpy = fvec3(0,1,0);
    auto normny = fvec3(0,-1,0);
    auto normpz = fvec3(0,0,1);
    auto normnz = fvec3(0,0,-1);

    glBegin(GL_TRIANGLES); 
        glNormal3fv(normny.GetArray());
        v3quad( v3vtx( lo, lo, lo ), // XZ loY
                v3vtx( hi, lo, lo ),
                v3vtx( hi, lo, hi ),
                v3vtx( lo, lo, hi ) );
        glNormal3fv(normpy.GetArray());
        v3quad( v3vtx( hi, hi, lo ), // XZ hiY
                v3vtx( lo, hi, lo ),
                v3vtx( lo, hi, hi ),
                v3vtx( hi, hi, hi ));
    glEnd();
    glBegin(GL_TRIANGLES); 
        glNormal3fv(normnz.GetArray());
        v3quad( v3vtx( lo, lo, hi ), // XY loZ
                v3vtx( lo, hi, hi ),
                v3vtx( hi, hi, hi ),
                v3vtx( hi, lo, hi ) );
        glNormal3fv(normpz.GetArray());
        v3quad( v3vtx( lo, lo, hi ), // XY hiZ
                v3vtx( hi, lo, hi ),
                v3vtx( hi, hi, hi ),
                v3vtx( lo, hi, hi ) );
    glEnd();
    glBegin(GL_TRIANGLES); // YZ loX
        glNormal3fv(normpx.GetArray());
        v3quad( v3vtx( lo, hi, hi ),
                v3vtx( lo, hi, lo ),
                v3vtx( lo, lo, lo ), // YZ loX
                v3vtx( lo, lo, hi ) );
        glNormal3fv(normnx.GetArray());
        v3quad( v3vtx( hi, hi, hi ),
                v3vtx( hi, lo, hi ), // YZ loX
                v3vtx( hi, lo, lo ),
                v3vtx( hi, hi, lo ) );
    glEnd();
    glDisable(GL_LIGHTING);

}
void Draw80x20X( fvec3 ilo, fvec3 ihi, fvec3 color )
{

    //DrawSolidBox( fvec3 ilo, fvec3 ihi, fvec3 color );

}

void DrawWireBox( fvec3 lo, fvec3 hi, fvec3 color )
{
    glColor4f(color.GetX(),color.GetY(),color.GetZ(),1.0);

    glDisable(GL_CULL_FACE);
    glBegin(GL_LINE_LOOP);
    glVertex3f( lo.GetX(), lo.GetY(), lo.GetZ() );
    glVertex3f( hi.GetX(), lo.GetY(), lo.GetZ() );
    glVertex3f( hi.GetX(), lo.GetY(), hi.GetZ() );
    glVertex3f( lo.GetX(), lo.GetY(), hi.GetZ() );
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex3f( lo.GetX(), hi.GetY(), lo.GetZ() );
    glVertex3f( hi.GetX(), hi.GetY(), lo.GetZ() );
    glVertex3f( hi.GetX(), hi.GetY(), hi.GetZ() );
    glVertex3f( lo.GetX(), hi.GetY(), hi.GetZ() );
    glEnd();
    glBegin(GL_LINES);
    glVertex3f( lo.GetX(), lo.GetY(), lo.GetZ() );
    glVertex3f( lo.GetX(), hi.GetY(), lo.GetZ() );
    glVertex3f( hi.GetX(), lo.GetY(), lo.GetZ() );
    glVertex3f( hi.GetX(), hi.GetY(), lo.GetZ() );
    glVertex3f( hi.GetX(), lo.GetY(), hi.GetZ() );
    glVertex3f( hi.GetX(), hi.GetY(), hi.GetZ() );
    glVertex3f( lo.GetX(), lo.GetY(), hi.GetZ() );
    glVertex3f( lo.GetX(), hi.GetY(), hi.GetZ() );
    glEnd();
}

void DrawBox( fvec3 lo, fvec3 hi, fvec3 color )
{
    DrawSolidBox(lo,hi,color*0.5);
    DrawWireBox(lo,hi,color);
}

void Render()
{
    glClearColor(0.0,0.0,0.0,1.0);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glViewport(0,0,width,height);
    glScissor(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glP(-500, 5500, -500, 5500, -500, 5500);
    gluPerspective(45.0,float(width)/float(height),10.0,20000.0);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    mtx.Lock();
    float max_x = vs_x.mSize;
    float max_y = vs_y.mSize;
    float max_z = vs_z1.mSize;
    float phy_x = vs_x.mPhysicalPos;
    float phy_y = vs_y.mPhysicalPos;
    float phy_z = vs_z1.mPhysicalMax-vs_z1.mPhysicalPos;
    bool ena_x = vs_x.mEnabled;
    bool ena_y = vs_y.mEnabled;
    bool ena_z = vs_z1.mEnabled;

    auto ctr = fvec3(max_x*0.5,0,max_z*0.5);
   // auto eye = fvec3(max_x,max_x*0.25,max_z)*1.5f;
    auto eye = fvec3(max_x*0.5,2000,9000);

    auto e2c = (ctr-eye).Normal();
    auto x1 = (fvec3(0.0,1.0,0.0).Cross(e2c)).Normal();
    auto up = (e2c.Cross(x1)).Normal();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt( eye.GetX(),eye.GetY(),eye.GetZ(),
               ctr.GetX(),ctr.GetY(),ctr.GetZ(),
               up.GetX(), up.GetY(), up.GetZ() );
    //glRotatef(fphase,0,0,1);

    ////////////////////////////////////////
    // general geom constants
    ////////////////////////////////////////

    const float yfactor = 0.2f;
    float YHI = max_y*yfactor;
    const float dim80mm = 400.0;
    const float dim20mm = 100.0;
    const float znw = 150.0;
    const float y1 = YHI*0.4;
    const float y2 = YHI*0.8;
    const float znx2 = znw*2.0;
    const float znx3 = znx2-100;

    ////////////////////////////////////////
    // green XZ origin
    ////////////////////////////////////////

    glColor4f(0.0,1.0,0.0f,1.0);
    glBegin(GL_LINES);
    glVertex3f( 0.0, 0.0f, 0.0 );
    glVertex3f( 0.0, 1000.0f, 0.0 );
    glEnd();

    ////////////////////////////////////////
    // grey XZ surface
    ////////////////////////////////////////

    glColor4f(1.0,1.0,1.0f,1.0);

    auto white = fvec3(1.0,1.0,1.0);
    auto red = fvec3(1.0f,0.0,0.0);

    auto ctr_z = max_z*0.5;
    auto xzlo = fvec3(-dim80mm,-150.0,ctr_z-200);
    auto xzhi = fvec3(max_x+dim80mm, -200.0, ctr_z+200);
    DrawBox( xzlo,xzhi,white*0.3 );

    ////////////////////////////////////////
    // X lower frame
    ////////////////////////////////////////

    float xfy1 = y1-dim20mm;
    float xfy2 = xfy1-dim80mm;
    float xfz1 = -dim20mm*4;
    float xfz2 = max_z+dim20mm*4;
    float xfx1 = -znx2;
    float xfx2 = max_x+znx2;
    auto xfra1_lo = fvec3(xfx1,xfy1,xfz1);
    auto xfra1_hi = fvec3(xfx2,xfy2,xfz1-dim20mm);

    auto xfra2_lo = fvec3(xfx1,xfy1,xfz2);
    auto xfra2_hi = fvec3(xfx2,xfy2,xfz2+dim20mm);

    DrawSolidBox(xfra1_lo,xfra1_hi,white*0.4);
    DrawSolidBox(xfra2_lo,xfra2_hi,white*0.4);

    ////////////////////////////////////////
    // Z frame/axis
    ////////////////////////////////////////

    float zfz1 = 0.0-dim80mm;
    float zfz2 = max_z+dim80mm;
    float zfy1 = y1;
    float zfy2 = zfy1-dim80mm;
    auto z1lo = fvec3(-znx2,zfy1,zfz1);
    auto z1hi = fvec3(-znx3,zfy2,zfz2);
    DrawSolidBox(z1lo,z1hi,white*0.5);

    auto z2lo = fvec3(max_x+znx2,zfy1,zfz1);
    auto z2hi = fvec3(max_x+znx3,zfy2,zfz2);
    DrawSolidBox(z2lo,z2hi,white*0.5);

    ////////////////////////////////////////
    // X rail
    ////////////////////////////////////////

    auto xrail_lo = fvec3(-znw,y1,phy_z-25.0);
    auto xrail_hi = fvec3(max_x+znw,y2,phy_z+25.0);

    DrawSolidBox(xrail_lo,xrail_hi,red);

    ////////////////////////////////////////
    // y platform
    ////////////////////////////////////////

    float ypy1 = dim80mm*5.0;
    float ypy2 = ypy1-dim80mm*4.5;
    auto yplat_lo = fvec3(phy_x-250,ypy1,phy_z+100.0);
    auto yplat_hi = fvec3(phy_x+250,ypy2,phy_z+50.0);
    DrawSolidBox(yplat_lo,yplat_hi,red*0.5);

    float zpy1 = 0;
    float zpy2 = zpy1+dim80mm;
    auto zp1x1 = -dim20mm*2.75;
    auto zp1x2 = zp1x1+dim20mm*1.3;
    auto zp2x1 = max_x+dim20mm*2.75;
    auto zp2x2 = zp2x1-dim20mm*1.3;
    auto zp1lo = fvec3(zp1x1,zpy1,phy_z-dim80mm);
    auto zp1hi = fvec3(zp1x2,zpy2,phy_z+dim80mm);
    auto zp2lo = fvec3(zp2x1,zpy1,phy_z-dim80mm);
    auto zp2hi = fvec3(zp2x2,zpy2,phy_z+dim80mm);
    DrawSolidBox(zp1lo,zp1hi,red*0.5);
    DrawSolidBox(zp2lo,zp2hi,red*0.5);

    ////////////////////////////////////////
    // blue y axis
    ////////////////////////////////////////

    glColor4f( 0.0f, 0.0, 1.0f, 1.0f );
    glBegin(GL_LINES);
    glVertex3f( phy_x, max_y*yfactor, phy_z );
    glVertex3f( phy_x, 0.0, phy_z );
    glEnd();
    

    auto slo = fvec3(-30,-200,-30 );
    auto shi = fvec3(+30,+00,+30 );
    auto sblo = fvec3(-80,+400,-80 );
    auto sbhi = fvec3(+80,+00,+80 );
    glTranslatef( phy_x, 150.0+phy_y*yfactor, phy_z+200 );
    DrawSolidBox(sblo,sbhi,fvec3(0.5,0.5,0.7));

    glRotatef(spindle_ang,0.0,1.0,0.0);
    DrawSolidBox(slo,shi,white);
    DrawWireBox(slo*1.1,shi*1.1,white);
    //glutWireSphere(100.0,10,10);

    glColor4f( 1.0, 1.0f, 0.0f, 1.0f );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    fxstring<256> fxs;
    fxs.format( "px<%04.1f> py<%04.1f> pz<%04.1f> max_x<%f>", phy_x, phy_y, phy_z, max_x );
    drawstring2d( fxs.c_str(), 0, height-50, 0.1 );

    mtx.Unlock();

    glutSwapBuffers();
}

//////////

int main( int argc, char** argv )
{
    auto curflags = fcntl(0,F_GETFL);
    fcntl(0,F_SETFL,curflags|O_NONBLOCK);
    setvbuf(stdin,nullptr,_IONBF,0);

    setup();
    gwin = new window(argc,argv);
    //gwin->mRG.Resize(width,height);

    gwin->mopg->push( Op([]()
    {
        while(1)
            loop();
    }));

    gwin->run_loop();
    return 0;
}

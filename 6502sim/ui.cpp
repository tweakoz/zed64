#include "mos6502.h"
#include <GLFW/glfw3.h>
#include "drawtext.h"

using namespace ork;
///////////////////////////////////////////////////////////////////////////////
std::list<insline> _inslinelist;
spsc_bounded_queue<insline> _inslineQ;
spsc_bounded_queue<uicmd> _uiQ;
extern LockedResource<addrset_t> addr_read_set;
extern LockedResource<addrset_t> addr_write_set;
uint8_t busReadNT( uint16_t addr );
///////////////////////////////////////////////////////////////////////////////
static int width = 0;
static int height = 0;
static float fontscale = 0.5f;
///////////////////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action != GLFW_PRESS)
        return;

    switch( key )
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
    }
    uicmd uic = { key, mods };
    _uiQ.push(uic);
}
///////////////////////////////////////////////////////////////////////////////
void PushOrthoMVP(float VPW, float VPH)
{   glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,VPW,VPH,0,0,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}
///////////////////////////////////////////////////////////////////////////////
void PopMVP()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}
///////////////////////////////////////////////////////////////////////////////
void drawtext( const std::string& str, float x, float y, float scale, float r, float g, float b )
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,width,height,0,0,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();    
    glTranslatef(x,y, 0);
    glScalef(scale,-scale,1);

    glColor4f(r,g,b,1);
    dtx_string(str.c_str());

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
void drawtext( const std::string& str, float x, float y, float scale, fvec3 color )
{
    drawtext(str,x,y,scale,color.GetX(),color.GetY(),color.GetZ());
}
///////////////////////////////////////////////////////////////////////////////
std::string statusstring(uint8_t st);
uint8_t busRead( uint16_t addr );
void runUI()
{
    std::set<uint16_t> pages;

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
      return;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1152, 760, "6502Sim", NULL, NULL);
    if (!window)
    {
      glfwTerminate();
      return;
    }

    glfwMakeContextCurrent(window);

    auto font = dtx_open_font("/System/Library/Fonts/Menlo.ttc", 48);
    assert(font!=nullptr);
    dtx_use_font(font, 48);

    glfwSetKeyCallback(window, key_callback);

    ///////////////////////////////////////////////////////

    fvec3 STCHG(1,.7,.4);
    fvec3 R0X(1,.7,.7);
    fvec3 G0X(.7,1,.7);

    fvec3 RED(1,0,0);
    fvec3 PAL(1,.5,.5);
    fvec3 ORA(1,.5,0);
    fvec3 GRN(0,1,0);
    fvec3 BLU(0,0,1);
    fvec3 YEL(1,1,0);
    fvec3 WHI(1,1,1);
    fvec3 PMAG(1,.5,1);
    fvec3 GRY(.7,.7,.7);

    while (!glfwWindowShouldClose(window))
    {

        glfwGetFramebufferSize(window, &width, &height);

        int numins = ((height)-(20*18))/24;

        bool pullins = true;
        while(pullins)
        {
            insline il;
            pullins = _inslineQ.try_pop(il);
            if( pullins )
            {
                while(_inslinelist.size()>numins)
                    _inslinelist.pop_front();
                _inslinelist.push_back(il);
            }
        }

        glClearColor(.25,.25,.25,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0,0,width,height);
        glScissor(0,0,width,height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0,width,0,height,0,1);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        ///////////////////////////////

        glLoadIdentity();    
        glColor4f(1,1,0,1);

        int x = 64;
        int y = 32;
        drawtext( "PC", x,y, fontscale, PMAG );
        x += 84;
        drawtext( "INS", x,y, fontscale, PMAG );
        x += 256;
        drawtext( "A", x,y, fontscale, PMAG );
        x += 48;
        drawtext( "X", x,y, fontscale, PMAG );
        x += 48;
        drawtext( "Y", x,y, fontscale, PMAG );
        x += 48;
        drawtext( "ST", x,y, fontscale, PMAG );
        x += 48;
        drawtext( "SP", x,y, fontscale, PMAG );
        x += 128;
        drawtext( "MEM", x,y, fontscale, PMAG );

        /////////////////////////////////////////
        // draw backgrounds
        /////////////////////////////////////////

        int mpbasey = height-(20*12);

        PushOrthoMVP(width,height);

        glBegin(GL_QUADS);

            y = 64;

            glColor3f(.2,0,.2);
            glVertex2i(0,mpbasey-20);
            glVertex2i(width,mpbasey-20);
            glVertex2i(width,height);
            glVertex2i(0,height);

            for(auto& item : _inslinelist)
            {
                auto bgc = item._bgcolor;
                glColor3f(bgc.GetX(),bgc.GetY(),bgc.GetZ());
                glVertex2i(0,y-0);
                glVertex2i(width,y-0);
                glVertex2i(width,y-20);
                glVertex2i(0,y-20);

                y+=24;
            }
        glEnd();            
        PopMVP();

        /////////////////////////////////////////
        // draw instructions
        /////////////////////////////////////////

        drawtext( "MEMORY PAGE VIEW", 64,mpbasey-20, fontscale, PMAG );

        y = 64;

        bool pcchg = false;
        fvec3 pcclr;

        for(auto& item : _inslinelist)
        {
            int x = 64;
            drawtext( FormatString("%04X",item._lpc), x,y, fontscale, pcchg ? pcclr : WHI );
            x += 84;
            drawtext( item._instext, x,y, fontscale, YEL );
            x += 256;
            drawtext( FormatString("%02X",item._a), x,y, fontscale, (item._la!=item._a) ? STCHG : GRY );
            x += 48;
            drawtext( FormatString("%02X",item._x), x,y, fontscale, (item._lx!=item._x) ? STCHG : GRY );
            x += 48;
            drawtext( FormatString("%02X",item._y), x,y, fontscale, (item._ly!=item._y) ? STCHG : GRY );
            x += 48;

            auto st1 = statusstring(item._st);
            drawtext( st1, x,y, fontscale, (item._lst!=item._st) ? STCHG : GRY );
            x += 48;

            drawtext( FormatString("%02X",item._sp), x,y, fontscale, (item._lsp!=item._sp) ? STCHG : GRY );

            x += 128;

            ///////////////////

            auto doacc = [&](accesslist& acclist){
                for( auto accitem : acclist )
                {
                    uint16_t page = accitem._addr&0xfff0;
                    pages.insert(page);

                    auto color1 = accitem._write ? R0X : G0X;
                    auto color2 = accitem._write ? RED : GRN;
                    drawtext( FormatString("%04X:",accitem._addr), x,y, fontscale, color1 );
                    drawtext( FormatString("%02X", accitem._val),x+72,y, fontscale, color2 );
                    x += 108;                
                }
            };


            ///////////////////

            doacc(item._preacc);
            x += 8; 
            drawtext( "|", x,y, fontscale, WHI );
            x += 32; 
            doacc(item._postacc);

            ///////////////////

            y+=24;

            pcchg = item._pcjmp;
            pcclr = (item._bgcolor*2)+fvec3(.2,.2,.2);
        }

        ///////////////////////////////
        // draw memory pages
        ///////////////////////////////

        x = 64;

        int ipage = 0;

        for( const auto& page : pages )
        {
            x = 64 + (ipage/12)*640;
            y = mpbasey + (ipage%12)*20;

            fvec3 color = WHI;
            if( page<256 )
                color = fvec3(.7,.7,1);
            else if( page<512 )
                color = fvec3(.7,1,1);
            else if( page>=0xd000 and page<0xfff0 )
                color = fvec3(1,1,.5);
            else if( page==0xfff0 )
                color = fvec3(1,.7,.2);

            drawtext( FormatString("%04x",page), x,y, fontscale*0.8, color );

            std::string linestr;
            for( int i=0; i<16; i++ )
            {   u16 addr = page+i;
                u8 val = busReadNT(addr);
                color = PAL;

                bool was_read;
                bool was_writ;
                
                addr_read_set.AtomicOp([&](addrset_t& aset){
                    was_read = (aset.find(addr)!=aset.end());
                });
                addr_write_set.AtomicOp([&](addrset_t& aset){
                    was_writ = (aset.find(addr)!=aset.end());
                });

                if(was_read and was_writ)
                    color = YEL;
                else if(was_writ)
                    color = RED;
                else if(was_read)
                    color = GRN;
                drawtext( FormatString("%02x",val), x+64+(i*32),y, fontscale*0.8, color );
            }

            ipage++;
        }

        ///////////////////////////////

        glFinish();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
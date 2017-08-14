#include "mos6502.h"
#include <assert.h>
#include <GLFW/glfw3.h>
#include "drawtext.h"

///////////////////////////////////////////////////////////////////////////////

static int width = 0;
static int height = 0;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool down = ( action == GLFW_PRESS );
    bool up = ( action == GLFW_RELEASE );

    bool shift = (mods&1);

    switch( key )
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        default:
            break;
    }
}

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

void runUI()
{
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

    auto font = dtx_open_font("/Library/Fonts/Tahoma.ttf", 48);
    assert(font!=nullptr);
    dtx_use_font(font, 48);

    glfwSetKeyCallback(window, key_callback);

    ///////////////////////////////////////////////////////

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &width, &height);

        glClearColor(.20,0,.20,1);
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

        ///////////////////////////////

        glFinish();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
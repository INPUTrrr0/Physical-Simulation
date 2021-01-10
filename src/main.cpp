#include "gl.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include "vertexrecorder.h"
#include "spring.h"
#include "starter3_util.h"
#include "camera.h"
#include "timestepper.h"
#include "simplesystem.h"
#include "pendulumsystem.h"
#include "clothsystem.h"
using namespace std;

namespace
{
// Set the scene
Camera camera;
GLuint program_color;
GLuint program_light;
void initSystem(); void stepSystem(); void drawSystem();
void freeSystem(); void resetTime(); void initRendering();
void drawAxis();
const Vector3f LIGHT_POS(2.0f, 5.0f, 3.0f);
const Vector3f LIGHT_COLOR(120.0f, 120.0f, 120.0f);
const Vector3f FLOOR_COLOR(0.6f, 0.6f, 0.9f);

// time keeping
uint64_t start_tick; double elapsed_s; double simulated_s;

// Globals here.
float h; char integrator; bool gMousePressed = false;

TimeStepper* timeStepper; 
SimpleSystem* simpleSystem;
PendulumSystem* pendulumSystem;
ClothSystem* clothSystem;

// key handling
static void keyCallback(GLFWwindow* window, int key,
    int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) { // only handle PRESS and REPEAT
        return;
    }

    // Special keys (arrows, CTRL, ...) are documented
    // here: http://www.glfw.org/docs/latest/group__keys.html
    switch (key) {
    case GLFW_KEY_ESCAPE: 
        exit(0);
        break;

    case ' ':
    {
        Matrix4f eye = Matrix4f::identity();
        camera.SetRotation(eye);
        camera.SetCenter(Vector3f(0, 0, 0));
        break;
    }

    case 'R':
    {
        cout << "Resetting simulation\n";
        freeSystem();
        initSystem();
        resetTime();
        break;
    }
    default:
        cout << "Unhandled key press " << key << "." << endl;
    }
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    double xd, yd;
    glfwGetCursorPos(window, &xd, &yd);
    int x = (int)xd;
    int y = (int)yd;

    int lstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int rstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    int mstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
    if (lstate == GLFW_PRESS) {
        gMousePressed = true;
        camera.MouseClick(Camera::LEFT, x, y);
    }
    else if (rstate == GLFW_PRESS) {
        gMousePressed = true;
        camera.MouseClick(Camera::RIGHT, x, y);
    }
    else if (mstate == GLFW_PRESS) {
        gMousePressed = true;
        camera.MouseClick(Camera::MIDDLE, x, y);
    }
    else {
        gMousePressed = true;
        camera.MouseRelease(x, y);
        gMousePressed = false;
    }
}

static void motionCallback(GLFWwindow* window, double x, double y)
{
    if (!gMousePressed) {
        return;
    }
    camera.MouseDrag((int)x, (int)y);
}

void setViewport(GLFWwindow* window)
{
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    camera.SetDimensions(w, h);
    camera.SetViewport(0, 0, w, h);
    camera.ApplyViewport();
}

void drawAxis()
{
    glUseProgram(program_color);
    Matrix4f M = Matrix4f::translation(camera.GetCenter()).inverse();
    camera.SetUniforms(program_color, M);

    const Vector3f DKRED(1.0f, 0.5f, 0.5f);
    const Vector3f DKGREEN(0.5f, 1.0f, 0.5f);
    const Vector3f DKBLUE(0.5f, 0.5f, 1.0f);
    const Vector3f GREY(0.5f, 0.5f, 0.5f);

    const Vector3f ORGN(0, 0, 0);
    const Vector3f AXISX(5, 0, 0);
    const Vector3f AXISY(0, 5, 0);
    const Vector3f AXISZ(0, 0, 5);

    VertexRecorder recorder;
    recorder.record_poscolor(ORGN, DKRED);
    recorder.record_poscolor(AXISX, DKRED);
    recorder.record_poscolor(ORGN, DKGREEN);
    recorder.record_poscolor(AXISY, DKGREEN);
    recorder.record_poscolor(ORGN, DKBLUE);
    recorder.record_poscolor(AXISZ, DKBLUE);

    recorder.record_poscolor(ORGN, GREY);
    recorder.record_poscolor(-AXISX, GREY);
    recorder.record_poscolor(ORGN, GREY);
    recorder.record_poscolor(-AXISY, GREY);
    recorder.record_poscolor(ORGN, GREY);
    recorder.record_poscolor(-AXISZ, GREY);

    glLineWidth(3);
    recorder.draw(GL_LINES);
}


// initialize your particle systems
void initSystem()
{
    switch (integrator) {
    case 'e': timeStepper = new ForwardEuler(); break;
    case 't': timeStepper = new Trapezoidal(); break;
    case 'r': timeStepper = new RK4(); break;
    default: printf("Unrecognized integrator\n"); exit(-1);
    }

    simpleSystem = new SimpleSystem();
    pendulumSystem = new PendulumSystem();
    clothSystem = new ClothSystem();
}

void freeSystem() {
    delete simpleSystem; simpleSystem = nullptr;
    delete timeStepper; timeStepper = nullptr;
    delete pendulumSystem; pendulumSystem = nullptr;
    delete clothSystem; clothSystem = nullptr;
}

void resetTime() {
    elapsed_s = 0;
    simulated_s = 0;
    start_tick = glfwGetTimerValue();
}

//external forces update?
void stepSystem()
{
    // step until simulated_s -> elapsed_s.
    while (simulated_s < elapsed_s) {
        timeStepper->takeStep(simpleSystem, h);
        timeStepper->takeStep(pendulumSystem, h);
        timeStepper->takeStep(clothSystem, h);
        simulated_s += h;
    }
}

// Draw the current particle positions
void drawSystem()
{
    // GLProgram wraps up all object that
    // particle systems need for drawing themselves
    GLProgram gl(program_light, program_color, &camera);
    gl.updateLight(LIGHT_POS, LIGHT_COLOR.xyz()); // once per frame

    //draw the systems
    simpleSystem->draw(gl); 
    pendulumSystem->draw(gl); 
    clothSystem->draw(gl);

    //draw floor
    gl.updateMaterial(FLOOR_COLOR); 
    gl.updateModelMatrix(Matrix4f::translation(0, -5.0f, 0));
    drawQuad(50.0f);
}

//-------------------------------------------------------------------

void initRendering()
{
    // Clear to black
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
}

int errorStatement(int arguments, char** inputs){

        if (arguments != 3) {
        printf("Usage: %s <e|t|r> <timestep>\n", inputs[0]);
        printf("       e: Integrator: Forward Euler\n");
        printf("       t: Integrator: Trapezoid\n");
        printf("       r: Integrator: RK 4\n");
        printf("\n");
        printf("Try  : %s t 0.001\n", inputs[0]);
        printf("       for trapezoid (1ms steps)\n");
        printf("Or   : %s r 0.01\n", inputs[0]);
        printf("       for RK4 (10ms steps)\n");
        return -1;
    }

    if (*inputs[1]=='r'||*inputs[1]=='t'||*inputs[1]=='e'){
        return 0;
    }
    else{
        printf("Usage: %s <e|t|r> <timestep>\n", inputs[0]);
        printf("       e: Integrator: Forward Euler\n");
        printf("       t: Integrator: Trapezoid\n");
        printf("       r: Integrator: RK 4\n");
        printf("\n");
        printf("Try  : %s t 0.001\n", inputs[0]);
        printf("       for trapezoid (1ms steps)\n");
        printf("Or   : %s r 0.01\n", inputs[0]);
        printf("       for RK4 (10ms steps)\n");
        return -1;
    }
}


int main(int argc, char** argv)
{
    // Set up OpenGL, define the callbacks and start the main loop
    //if bad, return -1

    if (errorStatement(argc,argv)==-1){
        return -1;
        printf("hello");
    }

    integrator = argv[1][0]; h = (float)atof(argv[2]);
    printf("Using Integrator %c with time step %.4f\n", integrator, h);

    GLFWwindow* window = createOpenGLWindow(1024, 1024, "Assignment 3");

    //event handlers
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetCursorPosCallback(window, motionCallback);

    initRendering();

    //OpenGL: defines vertex shader and fragment shader.
    program_color = compileProgram(c_vertexshader, c_fragmentshader_color);
    if (!program_color) {
        printf("Cannot compile program\n");
        return -1;
    }
    program_light = compileProgram(c_vertexshader, c_fragmentshader_light);
    if (!program_light) {
        printf("Cannot compile program\n");
        return -1;
    }

    camera.SetDimensions(600, 600);
    camera.SetPerspective(45);
    camera.SetDistance(15);

    // Setup particle system
    initSystem();
    uint64_t freq = glfwGetTimerFrequency();
    resetTime();

    //main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the rendering window
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        setViewport(window);
        if (gMousePressed) {drawAxis();}

        uint64_t now = glfwGetTimerValue();
        elapsed_s = (double)(now - start_tick) / freq;
        stepSystem(); drawSystem();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //free used systems
    glDeleteProgram(program_color);
    glDeleteProgram(program_light);
    return 0;	
}

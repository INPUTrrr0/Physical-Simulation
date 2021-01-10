#include "simplesystem.h"
#include "camera.h"
#include "vertexrecorder.h"

#include <iostream>
using namespace std;


SimpleSystem::SimpleSystem()
{
    m_vVecState.push_back(Vector3f(1.0, 0.0, 0.0));
}

std::vector<Vector3f> SimpleSystem::evalF(std::vector<Vector3f> state)
{
    std::vector<Vector3f> f;

    //compute derivative of F 
    for (int i = 0; i < (int)state.size(); ++i) {
        f.push_back(Vector3f(-state[i].y(), state[i].x(), 0));
    }
    return f;
}

// render the system (ie draw the particles)
void SimpleSystem::draw(GLProgram& gl)
{
    //uses methods in GLProgram, same in java
    const Vector3f PARTICLE_COLOR(0.4f, 0.7f, 1.0f);
    gl.updateMaterial(PARTICLE_COLOR); //update material uniforms (Color in this case) 
    // Vector3f pos(2, 2, 0); 
    // gl.updateModelMatrix(Matrix4f::translation(pos)); //update transform uniforms
    // drawSphere(0.5f, 10, 10);
    Vector3f pos2(1, 1, 0); 
    gl.updateModelMatrix(Matrix4f::translation(pos2)); //update transform uniforms
    drawCylinder(10, 0.5f, 1);
    //Vector3f pos3(0, 0, 0); 
    //gl.updateModelMatrix(Matrix4f::translation(pos2)); //update transform uniforms
    //drawCube(2);
}

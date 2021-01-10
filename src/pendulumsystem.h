#ifndef PENDULUMSYSTEM_H
#define PENDULUMSYSTEM_H

#include <vector>

#include "particlesystem.h"
#include "spring.h"

class PendulumSystem : public ParticleSystem
{
public:
    PendulumSystem();

    std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;
    void draw(GLProgram&);
    std::vector<Spring> springs;
};

#endif

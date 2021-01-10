#ifndef SPRING_H
#define SPRING_H

#include <vector>

#include "particlesystem.h"

class Spring
{
public:
    Spring();
    Spring(float r);
    void addConnection(int i);
    std::vector<int> getConnections();

private:
    std::vector<int> connections;
    float restLen;
};

#endif

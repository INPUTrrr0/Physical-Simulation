#include "spring.h" //its own header must be the first
#include "particlesystem.h"
#include <iostream>
#include <vector>
using namespace std;

Spring::Spring()
{
    // Any extra logic here.
   // cout << "I just got executed!";
}

Spring::Spring(float r)
{
    restLen = r;
}

void Spring::addConnection(int i)
{
    connections.push_back(i);
}

vector<int> Spring::getConnections()
{
    return connections;
}
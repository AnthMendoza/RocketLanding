#ifndef RUNGEKUTTA_H
#define RUNGEKUTTA_H

namespace SimCore{
float acceleration(float force , float mass);

void RungeKutta4th(float force, float mass, float timeStep, float &velocity, float &position);
}
#endif
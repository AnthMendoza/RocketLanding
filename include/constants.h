#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <array>

    namespace constants{
        float gravitationalAcceleration = -9.81; //

        std::array<float,3> initPosition = {0,0,1000};
        std::array<float,3> wind = {0,0,0}; //  m/s x,y,z  this will be stacked on top of whatever the current velocity is on an absolute cordinate system.
        std::array<float,3> initVehicleState = {.3,.4,0}; // this the vehicle vector, its direction is the directionthe nose points. it will be normilized, magnatiude is meain

        std::array<float , 3> MOI = {.4,.4,.4}; //get rid of this 
        float centerOfPressure = 5;

        float timeStep = .0001; //seconds
        float maxThrust = 981000;//netwons
        float minThrust = maxThrust *.4; //newtons, 40 percent of max;
        float mass = 42000; //kg booster mass
        float maxGimbleAngle = 20;// degrees

    }

#endif
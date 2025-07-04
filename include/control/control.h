#ifndef CONTROL_H
#define CONTROL_H

#include "../dynamics/vehicle.h"
#include "../sim/logs.h"

namespace SimCore {
class StanleyController {
    private:

    float k;              // gain for cross track error term
    float maxSteering;    // max steering angle in radians

    public:
        StanleyController(float gain, float maxSteeringAngle);

        float computeSteering(float headingError, float crossTrackError, float velocity);
};


//Predicition Vehicle is used to simulate the vehicle in ideal conditions.
//perfect state estimation contorl will be active unlike lookAhead.
//class predictionVehicle{
//
//    public:
//
//    predictionVehicle(Vehicle &vehicle);
//    //short term Prediction that the vehicle will weight heavily as the vehicle approches the gorund
//    //horizion is time in seconds for casting the predition {keep short}
//    void landingPrediction(float horizion);
//
//
//    
//
//};
    

}


#endif
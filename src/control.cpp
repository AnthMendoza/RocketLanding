
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "../include/vehicle.h"
#include "../include/constants.h"
#include "../include/logs.h"
#include "../include/vectorMath.h"
#include "../include/getRotation.h"



std::vector<float> lookAhead(Vehicle &rocket, float lookAheadTime , std::function<float(Vehicle&)> valueToLog){
    //make a copy of the rocket so that we can keep current conditions on the main vehicle and test parameters on the look ahead
    int initalIterations = rocket.iterations;
    float limit = lookAheadTime/constants::timeStep;
    std::vector<float> log;
    while(rocket.iterations - initalIterations < limit && rocket.Zposition > 0){
        rocket.drag();
        rocket.lift();
        rocket.updateState();
        log.push_back(valueToLog(rocket));
        rocket.iterations++;
    }

    return log;
}



float PID(float target , float currentState , float &previousError , float &sumOfError , float timeStep, float Pgain , float Igain , float Dgain){

    float error = target - currentState;

    sumOfError = error * timeStep;

    float slope = (error - previousError) / timeStep;

    previousError = error;

    return Pgain * error + Igain * sumOfError + Dgain * slope;


}


void reentryBurn(Vehicle &rocket){
    
    if(rocket.reentry == true) return;


    if(rocket.Zposition < 55000 && rocket.reentry == false){

        rocket.reentry = true;
        float currentMaxGForce = constants::maxGAllowedEntry + 1;
        float lastMaxGForce = currentMaxGForce;
        //main loop moves forward 1 second every lookAHead cylce
        float stepInterval = 1;
        std::array<float,2> direction = {0,0};
        float count = 0;
        while(currentMaxGForce > constants::maxGAllowedEntry){
            Vehicle lookAheadRocket = rocket;
            std::vector<float> gForces = lookAhead(lookAheadRocket , 105 , [](Vehicle &r) { return r.gForce; }); // 105 is the lookahead time in seconds. this may be stoppped earlier if the vehicle hits the ground
            
            currentMaxGForce = *std::max_element(gForces.begin(),gForces.end());
            if(currentMaxGForce < constants::maxGAllowedEntry && count > 0) return;
            if(currentMaxGForce > lastMaxGForce && count > 0) return;
            lastMaxGForce = currentMaxGForce;
            float currentIteration = rocket.iterations;
            count++;
            while(rocket.iterations < currentIteration + stepInterval/constants::timeStep && rocket.Zposition > 0){
                rocket.drag();
                rocket.lift();

                rocket.applyEngineForce(direction , constants::maxThrust * .6 * 3); 
                
                rocket.finVectors = rocket.getFinForceVectors();
                if(rocket.iterations == 0)logRocketPosition(rocket);
                logRocketPosition(rocket);
                rocket.updateState();
                rocket.iterations++;
            }

        }

    }

}



void lookAheadGlide(Vehicle &rocket ,float* log , int logSize){
    //make a copy of the rocket so that we can keep current conditions on the main vehicle and test parameters on the look ahead

    int initalIterations = rocket.iterations;
    while(rocket.iterations - initalIterations < logSize && rocket.Zposition > 0){
        rocket.drag();
        rocket.lift();
        rocket.updateState();
        log[initalIterations - rocket.iterations] = rocket.Xvelocity; 
        rocket.iterations++;

        logSize = 1;
    }
    
}





void glidToTarget(Vehicle &rocket){
    //rate of change is what we are looking for here
    //lookahead to see what the rate of change given 
    //goal: arrest any drift
    //vehicle vertical is not needed but would be nice
    if(rocket.reentry == false) return;

    float delta  = rocket.Xposition - constants::LandingTarget[1];

    std::array<float , 3> velo = {rocket.Xvelocity , rocket.Yvelocity , rocket.Zvelocity};
    std::array<float,3> targetVector = {0,0,1};

    float stanley = vectorAngleBetween(velo , targetVector) + atanf(2.0f * (delta/rocket.getVelocity()));
    
    rocket.sumOfMoments[0] = stanley * 10000;
    

}



void landingBurn(Vehicle &rocket){

    if(rocket.reentry == false) return;

    float landingAccelZ = ((rocket.Zvelocity * rocket.Zvelocity ) / (2 * rocket.Zposition)) - constants::gravitationalAcceleration;

    float landingBurnDuration =  rocket.Zvelocity / landingAccelZ;

    float landingAccelX = rocket.Xvelocity / landingBurnDuration;

    float landingAccelY = rocket.Yvelocity / landingBurnDuration;
    // as landing duration approches 0 landing acceleration X and Y grows rapidly 
    // lim 1/x as x approches +0 is infinity 
    if(landingBurnDuration < 1.0f ){ 
        landingAccelX = 0;
        landingAccelY = 0;
    }
    
    float landingForceX = landingAccelX * rocket.mass;

    float landingForceY = landingAccelY * rocket.mass;

    float landingForceZ = landingAccelZ * rocket.mass;

    float requiredThrust = sqrtf(landingForceX * landingForceX + landingForceY * landingForceY + landingForceZ * landingForceZ);

    if(requiredThrust >= constants::landingThrust){
        rocket.landingInProgress = true;
    }

    if(requiredThrust <= constants::minThrust || rocket.landingInProgress == false){
        rocket.landingInProgress = false; 
        return;
    }

    if(requiredThrust > constants::maxThrust) requiredThrust = constants::maxThrust;
    

    std::array<float,3> forceVector = {landingForceX , landingForceY , landingForceZ};


    std::array<float , 3> directionVector = normalizeVector(forceVector);

    std::array<float,2> direction = {0,0};
        
    std::array<float,2> twoDState = {rocket.vehicleState[1] , rocket.vehicleState[2]};

    std::array<float,2> targetState = {directionVector[1] , directionVector[2]};


    float error = twodAngleDiffrence( twoDState, targetState); 

    rocket.error = error;

    rocket.twoDAngle = twoDState;

    float PIDOutputY = PID(0,error,rocket.vehicleYError, rocket.sumOfVehicleYError, constants::timeStep , 2 ,0 , 1);
    
    //std::cout<< twoDState[0] << "," << twoDState[1] << "  " << targetState[0] << "," << targetState[1]<< "   "<< error  << "   " << PIDOutputY << "\n";

    twoDState = {rocket.vehicleState[0] , rocket.vehicleState[2]};

    targetState = {directionVector[0] , directionVector[2]};

    error  = twodAngleDiffrence( twoDState, targetState);

    float PIDOutputX = PID(0,error,rocket.vehicleXError, rocket.sumOfVehicleXError, constants::timeStep , 2 ,0, 1);



    rocket.engineGimbal( PIDOutputY , PIDOutputX );


    direction[1] = -rocket.gimbalY;
    direction[0] = rocket.gimbalX;


    rocket.applyEngineForce(direction , requiredThrust);
    
    


}





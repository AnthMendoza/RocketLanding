
#include <array>
#include <cmath>
#include <memory>
#include <cmath>
#include <memory>
#include <iostream>
#include "../../include/core/forceApplied.h"
#include "../../include/dynamics/vehicle.h"
#include "../../include/core/vectorMath.h"
#include "../../include/dynamics/aero.h"
#include "../../include/core/RungeKutta.h"
#include "../../include/core/odeIterator.h"
#include "../../include/core/getRotation.h"
#include "../../include/control/control.h"
#include "../../include/subsystems/sensors.h"
#include "../../include/sim/toml.h"
#include "../../include/core/quaternion.h"

namespace SimCore{
void Vehicle::initSensors(){
    toml::tomlParse vParse;
    vParse.parseConfig( configFile,"vehicle");

    // MOI
    auto moiArray = vParse.getArray("MOI");
    MOI[0] = moiArray[0];
    MOI[1] = moiArray[1];
    MOI[2] = moiArray[2];

    // Mass is deinfed by dryMass + fuel for some vehicles. If mass > 0 it by passes a normal Mass check.
    if(mass <= 0) mass = vParse.getFloat("mass");
    
    sensorMap = std::make_unique<std::unordered_map<std::string, std::unique_ptr<sensor>>>();
    
    toml::tomlParse accelParse;
    accelParse.parseConfig( configFile,"accelerometer");

    // Accelerometer
    std::unique_ptr<accelerometer> accel = std::make_unique<accelerometer>(
        accelParse.getFloat("frequency"),
        accelParse.getFloat("NoisePowerSpectralDensity"),
        accelParse.getFloat("bandwidth"),
        accelParse.getFloat("bias")
    );
    if (accelParse.getBool("burst") == true) {
        accel->setBurst(
            accelParse.getFloat("burstStdDeviation"),
            accelParse.getFloat("maxBurstDuration")
        );
    }
    sensorMap->insert({"accelerometer", std::move(accel)});

    // GNSS
    toml::tomlParse gpsParse;
    gpsParse.parseConfig( configFile,"gps");

    std::unique_ptr<GNSS> gps = std::make_unique<GNSS>(
        gpsParse.getFloat("frequency"),
        gpsParse.getFloat("NoisePowerSpectralDensity"),
        gpsParse.getFloat("bandwidth"),
        gpsParse.getFloat("bias")
    );

    if (gpsParse.getBool("burst") == true) {
        gps->setBurst(
            gpsParse.getFloat("burstStdDeviation"),
            gpsParse.getFloat("maxBurstDuration")
        );
    }
    sensorMap->insert({"GNSS", std::move(gps)});

    // Gyroscope
    toml::tomlParse gyroParse;
    gyroParse.parseConfig( configFile,"gyro");
    
    std::unique_ptr<gyroscope> gyro = std::make_unique<gyroscope>(
        gyroParse.getFloat("frequency"),
        gyroParse.getFloat("NoisePowerSpectralDensity"),
        gyroParse.getFloat("bandwidth"),
        gyroParse.getFloat("bias")
    );
    if (gyroParse.getBool("burst") == true) {
        gyro->setBurst(
            gyroParse.getFloat("burstStdDeviation"),
            gyroParse.getFloat("maxBurstDuration")
        );
    }
    sensorMap->insert({"gyro",std::move(gyro)});


}


Vehicle::Vehicle(): stateEstimation(){

}


void Vehicle::operator++(){
    ++iterations;
}


Vehicle& Vehicle::operator=(const Vehicle& other){
    if (this == &other) return *this;
    
    Xposition = other.Xposition;
    Yposition = other.Yposition;
    Zposition = other.Zposition;
    Xvelocity = other.Xvelocity;
    Yvelocity = other.Yvelocity;
    Zvelocity = other.Zvelocity;
    timeStep = other.timeStep;
    iterations = other.iterations;
    mass = other.mass;
    centerOfPressure = other.centerOfPressure;
    gForce = other.gForce;
    wind = other.wind;
    angularVelocity = other.angularVelocity;
    vehicleState = other.vehicleState;
    MOI = other.MOI;
    sumOfForces = other.sumOfForces;
    sumOfMoments = other.sumOfMoments;
    acceleration = other.acceleration;
    gravitationalAcceleration = other.gravitationalAcceleration;
    configFile = other.configFile;
    outputFile = other.outputFile;

    if (other.turbulantX)
        turbulantX = std::make_unique<turbulence>(*other.turbulantX);
    if (other.turbulantY)
        turbulantY = std::make_unique<turbulence>(*other.turbulantY);
    if (other.turbulantZ)
        turbulantZ = std::make_unique<turbulence>(*other.turbulantZ);
    if (other.pose)
        pose = std::make_unique<quaternionVehicle>(*other.pose);

    return *this;
}

Vehicle::Vehicle(const Vehicle& other)
    :   stateEstimation(other),
        Xposition(other.Xposition),
        Yposition(other.Yposition),
        Zposition(other.Zposition),
        Xvelocity(other.Xvelocity),
        Yvelocity(other.Yvelocity),
        Zvelocity(other.Zvelocity),
        timeStep(other.timeStep),
        iterations(other.iterations),
        mass(other.mass),
        centerOfPressure(other.centerOfPressure),
        gForce(other.gForce),
        wind(other.wind),
        angularVelocity(other.angularVelocity),
        vehicleState(other.vehicleState),
        MOI(other.MOI),
        sumOfForces(other.sumOfForces),
        sumOfMoments(other.sumOfMoments),
        acceleration(other.acceleration),
        gravitationalAcceleration(other.gravitationalAcceleration),
        configFile(other.configFile),
        outputFile(other.outputFile){
    if (other.turbulantX)
        turbulantX = std::make_unique<turbulence>(*other.turbulantX);
    if (other.turbulantY)
        turbulantY = std::make_unique<turbulence>(*other.turbulantY);
    if (other.turbulantZ)
        turbulantZ = std::make_unique<turbulence>(*other.turbulantZ);
    if (other.pose)
        pose = std::make_unique<quaternionVehicle>(*other.pose);
}



void Vehicle::init(string& vehicleConfig){
    outputFile = "../output.csv";
    toml::tomlParse vParse;
    vParse.parseConfig(vehicleConfig,"vehicle");

    iterations = 0;
    // Read initPosition
    auto pos = vParse.getArray("initPosition");
    Xposition = pos[0];
    Yposition = pos[1];
    Zposition = pos[2];


    gForce = 0;
    angularVelocity = {0, 0, 0};

    // Read initVehicleState
    auto state = vParse.getArray("initVehicleState");

    vehicleState[0] = state[0];
    vehicleState[1] = state[1];
    vehicleState[2] = state[2];


    // Read initVelocity
    auto velo = vParse.getArray("initVelocity");
    Xvelocity = velo[0];
    Yvelocity = velo[1];
    Zvelocity = velo[2];


    gravitationalAcceleration = vParse.getFloat("gravitationalAcceleration"); 

    //Turblant wind Set windvect to mean of turbalant object
    auto windVect = vParse.getArray("wind");
    wind[0] = windVect[0];
    wind[1] = windVect[1];
    wind[2] = windVect[2];

    sumOfForces[0] = 0;
    sumOfForces[1] = 0;
    sumOfForces[2] = 0;

    sumOfMoments[0] = 0;
    sumOfMoments[1] = 0;
    sumOfMoments[2] = 0;
    yawMoment = 0;

    acceleration = {0,0,0};
    
    timeStep = vParse.getFloat("timeStep");
    // todo 
    //change init vectors to match setup in config file. need computation to ensure init vectors are valid.

    pose = std::make_unique<quaternionVehicle>();
    turbulantX = std::make_unique<turbulence> ();
    turbulantY = std::make_unique<turbulence> ();
    turbulantZ = std::make_unique<turbulence> ();

}





float Vehicle::getVelocity(){
    std::array<float ,3> vector = {Xvelocity , Yvelocity , Zvelocity};
    return vectorMag( vector );
}



float Vehicle::getGForce(){
    
    std::array<float ,3> vector = {sumOfForces[0]/mass,sumOfForces[1]/mass,sumOfForces[2]/mass};

    return vectorMag(vector)/9.8;
}

void Vehicle::getAccel(std::array<float,3> &accel){
    for(int i = 0 ; i < 3 ; i++) accel[i] = sumOfForces[i] / mass;
}


void Vehicle::drag(float (*aeroArea)(float),float (*coefOfDrag)(float)){

    std::array<float,3> airVelocityVector;

    airVelocityVector[0] = Xvelocity +  wind[0];
    airVelocityVector[1] = Yvelocity +  wind[1];
    airVelocityVector[2] = Zvelocity +  wind[2];


    float absVelocity = vectorMag(airVelocityVector);

    float dragAngle = vectorAngleBetween(airVelocityVector , vehicleState);

    
    float drag = -.5 * (absVelocity * absVelocity) * aeroArea(dragAngle) * coefOfDrag(dragAngle) * airDensity(Zposition);

    std::array<float,3> dragVector;
    std::array<float,3> normalVelocityVector = normalizeVector(airVelocityVector);
    
    dragVector[0] = drag * normalVelocityVector[0];
    dragVector[1] = drag * normalVelocityVector[1];
    dragVector[2] = drag * normalVelocityVector[2];

    addForce(dragVector);

    addMoment(forceToMoment(dragVector, vehicleState , centerOfPressure));
}


void Vehicle::lift(float (*aeroArea)(float),float (*coefOfLift)(float)){

    //lift acting on the center of pressure.

    std::array<float,3> airVelocityVector;

    airVelocityVector[0] = Xvelocity +  wind[0];
    airVelocityVector[1] = Yvelocity +  wind[1];
    airVelocityVector[2] = Zvelocity +  wind[2];
    

    float absVelocity = vectorMag(airVelocityVector);

    // Project vector V onto the plane with normal N: V_proj_plane = V - ((V • N) / (N • N)) * N (subtracting the projection onto N).
    //using normilzed vectors N • N is removed becuase it equals 1, this may or may not be faster than the equation abouve


    std::array<float , 3> normalAirVelocityVector = normalizeVector(airVelocityVector);

    float projection = vectorDotProduct( vehicleState , normalAirVelocityVector );

    std::array<float , 3> projectedVector;


    for(int i = 0 ; i < 3 ; i++){
        projectedVector[i] =  vehicleState[i] - projection * normalAirVelocityVector[i];
    }

    //we normilized this vector so that we can multiply it by a scalar with expected results
    projectedVector = normalizeVector(projectedVector); 

    std::array<float,3> reverseVehicleState;

    for(int i = 0 ; i < 3 ; i++) reverseVehicleState[i] = -vehicleState[i];


    float liftAngle = vectorAngleBetween(airVelocityVector , reverseVehicleState); 

    //create a mapping function for lift to force, this is a crude esimate.
    
    float lift = -.5 * (absVelocity * absVelocity) * aeroArea(liftAngle) * coefOfLift(liftAngle) * airDensity(Zposition); //calculating abs drag 
    
    std::array<float,3> liftVector;

    liftVector[0] = lift * projectedVector[0];
    liftVector[1] = lift * projectedVector[1];
    liftVector[2] = lift * projectedVector[2];

    if(directionality(normalAirVelocityVector , vehicleState) == false){
        liftVector[0] = -liftVector[0];
        liftVector[1] = -liftVector[1];
        liftVector[2] = -liftVector[2];
    }

    addForce(liftVector);

    addMoment(forceToMoment(liftVector, vehicleState , centerOfPressure));

}


void  Vehicle::addForce(std::array<float,3> forceVector){
    sumOfForces[0] += forceVector[0];
    sumOfForces[1] += forceVector[1];
    sumOfForces[2] += forceVector[2];
}



void  Vehicle::addMoment(std::array<float,3> moments){
    sumOfMoments[0] += moments[0];
    sumOfMoments[1] += moments[1];
    sumOfMoments[2] += moments[2];
}

void Vehicle::addYawMoment(float moment){
    yawMoment += moment;
}


void Vehicle::updateState(){
        //adding gravity to the force of Z, becuase this is an acceleration and not a force; The addForce function cannot handle it
        // Before using GNSS velocity
        if(iterations == 0){
            auto& ptr = sensorMap->find("GNSS")->second;
            GNSS* GPS = dynamic_cast<GNSS*> (ptr.get());
            GPS->setGNSSVelocity({Xvelocity,Yvelocity,Zvelocity});
        }
        updateSensors(this);
        updateEstimation( timeStep);
    
        sumOfForces[2] = sumOfForces[2] + gravitationalAcceleration * mass; 
        // After gravitational force added


        updateAcceleration();
        // Before Runge-Kutta

        RungeKutta4th(sumOfForces[0] , mass ,  timeStep , Xvelocity,Xposition);
        RungeKutta4th(sumOfForces[1] , mass ,  timeStep , Yvelocity,Yposition);
        RungeKutta4th(sumOfForces[2] , mass ,  timeStep , Zvelocity,Zposition);
    // Before rotational ODE

        Quaternion quant = pose->eularRotation(rotationalOde(sumOfMoments[0] , MOI[0],  timeStep ,angularVelocity[0]),
                            rotationalOde(sumOfMoments[1] , MOI[1],  timeStep ,angularVelocity[1]),
                            rotationalOde(sumOfMoments[2] , MOI[2],  timeStep ,angularVelocity[2]));
        rotateLocalEntities(quant);
        
        vehicleState = pose->getdirVector();
    
        gForce = getGForce();
    
        getAccel(acceleration);
    
        sumOfForces[0] = 0; //reset forces to zero for next iteration
        sumOfForces[1] = 0;
        sumOfForces[2] = 0;

        logSumOfMoments = sumOfMoments;
        
        sumOfMoments[0] = 0;
        sumOfMoments[1] = 0;
        sumOfMoments[2] = 0;
        
        wind[0] = 0;
        wind[1] = 0;
        wind[2] = 0;
        // After pose rotation
}




float Vehicle::PID(float target , float currentState , float &previousError , float &sumOfError , float timeStep, float Pgain , float Igain , float Dgain){

    float error = target - currentState;

    sumOfError = error * timeStep;

    float slope = (error - previousError) / timeStep;

    previousError = error;

    return Pgain * error + Igain * sumOfError + Dgain * slope;


}

//future model if vehicle is moving.Dryden Wind Turbulence Model (https://en.wikipedia.org/wiki/Dryden_Wind_Turbulence_Model)
//The industry standard way of generating turblance using a stochastic model. 
//white noise will be generated and smoothed using a low pass filter.  
void Vehicle::turbulantWind(){
    wind[0] += turbulantX->getNext();
    wind[1] += turbulantY->getNext();
    wind[2] += turbulantZ->getNext();
}


void Vehicle::rotateLocalEntities(Quaternion& quant){
    return;
}


}




#ifndef VEHICLE_H
#define VEHICLE_H


#include <array>


class Vehicle{
    public:
        float   Xposition , Yposition , Zposition;     // position 
        float Xvelocity , Yvelocity , Zvelocity;
        
        //distance between the center of gravity to the center of pressure, this allows us to not have COG defined explicitly.
        //all forces will be in refrance to the Center of gravity
        float mass;  
        float centerOfPressure;
        float cogToEngine;

        float liftAngleLog;
        bool reentry;
        bool glidePhase;
        std::array<float,3> engineState;

        int iterations;

        float gForce;


        std::array<float,3> angularVelocity;
        std::array<float,3> vehicleState;
        std::array<float,3> MOI;
        std::array<float,3> sumOfForces;
        std::array<float,3> sumOfMoments;
        std::array<float,3> logEngineVector;
        
        Vehicle();

        void display();

        float getVelocity();

        float getGForce();

        void drag();

        void lift();

        void applyEngineForce(std::array<float,2> twoDEngineRadians , float thrust);

        void addForce(std::array<float,3> forceVector);

        void addMoment(std::array<float,3> moments);

        void updateState();

        void finForce();

};



#endif
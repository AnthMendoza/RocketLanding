#ifndef MYSIM_H
#define MYSIM_H
#include "vehicle.h"
#include "rocket.h"
#include "drone.h"
#include <memory>
namespace SimCore{
//#pragma pack(push,1)
struct unrealDataRocket{
    float timeStamp;
    float position[3];
    float velocity[3];
    float rotation[3];
};
//#pragma pack(pop)


class unrealRocket{
    private:
    unrealDataRocket packet;
    std::string configFile;
    void setPacket();
    void getPacket(unrealDataRocket &dataPacket);
    void iterator(float totalTime);
    float totalTime;
    public:
    std::unique_ptr<Rocket> unrealVehicle;
    unrealRocket(std::string tomlData);
    ~unrealRocket();
    unrealDataRocket* simFrameRequest(float deltaTime);
};

struct unrealDataDrone{
    float timeStamp;
    float position[3];
    float velocity[3];
    float dirVector[3]; 
    float fwdVector[3]; 
    float rightVector[3];
};

class unrealDrone{
    private:
    unrealDataDrone packet;
    void setPacket();
    void iterator(float totalTime);
    float totalTime;
    public:
    std::unique_ptr<droneBody> drone;
    unrealDrone(std::string motorConfig, std::string batteryConfig , std::string droneConfig,std::string propellerConfig);
    unrealDataDrone* simFrameRequest(float deltaTime);
    /**
    * @brief Sets Pid object target location to argument setpoint.
    * @param x in meters
    * @param y in meters
    * @param z in meters
    */
    void setTargetPosition(float x , float y , float z , float yaw);

};

}

 
#endif 
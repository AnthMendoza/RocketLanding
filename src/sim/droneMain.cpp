#include "../../include/sim/MySim.h"
#include "../../include/control/PIDGains.h"
#include <string>
#include <chrono>
#include <thread>
#include <tuple>

using namespace SimCore;
std::string readFileAsString(const std::string& filePath) {
    std::ifstream inFile(filePath);
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]){
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <motor_config> <battery_config> <propeller_config> <drone_config>\n";
        return 1;
    }
    
    std::string configMotor = readFileAsString(argv[1]);    
    std::string configBattery = readFileAsString(argv[2]);   
    std::string configPropeller = readFileAsString(argv[3]);
    std::string configDrone = readFileAsString(argv[4]);


    unrealDrone drone(configMotor,configBattery,configDrone,configPropeller);
    drone.setTargetPosition(0,0,50,0);
    while(true){
        unrealDataDrone* data =  drone.simFrameRequest(1.0f);
        //display methods. Not needed when using MySim
        drone.drone->display();
        drone.drone->droneDisplay();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
#include "../../include/subsystems/droneDependencyInjector.h"
#include "../../include/subsystems/motor.h"
#include "../../include/subsystems/propeller.h"
#include <stdexcept>

namespace SimCore{


propMotorPair setSquare(float x, float y, propeller& prop, motor& mot) {
    std::vector<std::unique_ptr<propeller>> propellers;
    std::vector<std::unique_ptr<motor>> motors;
    if(x <= 0 || y <= 0) throw std::runtime_error("X and Y values of SetSquare cannot be <=0 \n");
    x = x / 2;
    y = y / 2;


    std::array<std::array<float, 3>, 4> positions = {{
        { x,  y, 0},
        {-x,  y, 0},
        { x, -y, 0},
        {-x, -y, 0}
    }};

    for (const auto& pos : positions) {
        motors.push_back(std::make_unique<motor>(mot));
        auto p = std::make_unique<propeller>(prop);
        p->location = pos;
        p->locationTransposed = pos;
        p->direction = {0, 0, 1};
        p->directionTransposed = p->direction;
        propellers.push_back(std::move(p));
    }

    propMotorPair pAndM = {std::move(motors),std::move(propellers)};

    return pAndM;
}
}
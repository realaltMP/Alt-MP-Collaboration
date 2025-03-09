#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>


class Player {
public:
    int id;
    std::string name;
    float x, y, z; // Position
    float heading;

    Player(int _id, const std::string& _name);
};
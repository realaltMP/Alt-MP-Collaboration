#include "VehicleManager.h"
#include <iostream>
#include <algorithm>

namespace CustomMP {
    // VehicleManager implementation
    VehicleManager::VehicleManager()
        : m_NextVehicleID(1)
    {
    }

    VehicleManager::~VehicleManager() {
        // Clean up all vehicles
        std::lock_guard<std::mutex> lock(m_VehiclesMutex);
        for (auto& pair : m_Vehicles) {
            delete pair.second;
        }
        m_Vehicles.clear();
    }

    Vehicle* VehicleManager::CreateVehicle(uint32_t model, const Vector3& position, const Quaternion& rotation) {
        std::lock_guard<std::mutex> lock(m_VehiclesMutex);

        uint32_t vehicleID = m_NextVehicleID++;

        // Create new vehicle
        Vehicle* vehicle = new Vehicle(vehicleID, model);
        vehicle->SetPosition(position);
        vehicle->SetRotation(rotation);

        m_Vehicles[vehicleID] = vehicle;

        std::cout << "Created vehicle " << vehicleID << " with model " << model << std::endl;
        return vehicle;
    }

    bool VehicleManager::DestroyVehicle(uint32_t vehicleID) {
        std::lock_guard<std::mutex> lock(m_VehiclesMutex);

        auto it = m_Vehicles.find(vehicleID);
        if (it == m_Vehicles.end()) {
            std::cerr << "Vehicle with ID " << vehicleID << " not found" << std::endl;
            return false;
        }

        delete it->second;
        m_Vehicles.erase(it);

        std::cout << "Destroyed vehicle " << vehicleID << std::endl;
        return true;
    }

    Vehicle* VehicleManager::GetVehicle(uint32_t vehicleID) {
        std::lock_guard<std::mutex> lock(m_VehiclesMutex);

        auto it = m_Vehicles.find(vehicleID);
        if (it == m_Vehicles.end()) {
            return nullptr;
        }

        return it->second;
    }

    std::vector<Vehicle*> VehicleManager::GetAllVehicles() {
        std::lock_guard<std::mutex> lock(m_VehiclesMutex);

        std::vector<Vehicle*> vehicles;
        vehicles.reserve(m_Vehicles.size());

        for (const auto& pair : m_Vehicles) {
            vehicles.push_back(pair.second);
        }

        return vehicles;
    }

    void VehicleManager::Update() {
        // Update logic for all vehicles
        std::lock_guard<std::mutex> lock(m_VehiclesMutex);

        for (auto& pair : m_Vehicles) {
            // Custom update logic would go here
        }
    }

    // Vehicle implementation
    Vehicle::Vehicle(uint32_t id, uint32_t model)
        : Entity(id, model)
        , m_Health(1000)
        , m_DriverID(0)
        , m_EngineRunning(false)
        , m_PrimaryColor(0)
        , m_SecondaryColor(0)
    {
    }

    Vehicle::~Vehicle() {
    }

    void Vehicle::SetDriver(uint32_t playerID) {
        m_DriverID = playerID;
    }

    void Vehicle::RemoveDriver() {
        m_DriverID = 0;
    }

    void Vehicle::AddPassenger(uint32_t playerID) {
        if (std::find(m_PassengerIDs.begin(), m_PassengerIDs.end(), playerID) == m_PassengerIDs.end()) {
            m_PassengerIDs.push_back(playerID);
        }
    }

    void Vehicle::RemovePassenger(uint32_t playerID) {
        auto it = std::find(m_PassengerIDs.begin(), m_PassengerIDs.end(), playerID);
        if (it != m_PassengerIDs.end()) {
            m_PassengerIDs.erase(it);
        }
    }

    std::vector<uint8_t> Vehicle::Serialize() const {
        std::vector<uint8_t> data;

        // Add vehicle ID (4 bytes)
        data.push_back((m_ID >> 0) & 0xFF);
        data.push_back((m_ID >> 8) & 0xFF);
        data.push_back((m_ID >> 16) & 0xFF);
        data.push_back((m_ID >> 24) & 0xFF);

        // Add model (4 bytes)
        data.push_back((m_Model >> 0) & 0xFF);
        data.push_back((m_Model >> 8) & 0xFF);
        data.push_back((m_Model >> 16) & 0xFF);
        data.push_back((m_Model >> 24) & 0xFF);

        // Add position (12 bytes)
        const uint8_t* posBytes = reinterpret_cast<const uint8_t*>(&m_Position);
        data.insert(data.end(), posBytes, posBytes + sizeof(Vector3));

        // Add rotation (16 bytes)
        const uint8_t* rotBytes = reinterpret_cast<const uint8_t*>(&m_Rotation);
        data.insert(data.end(), rotBytes, rotBytes + sizeof(Quaternion));

        // Add health (2 bytes)
        data.push_back((m_Health >> 0) & 0xFF);
        data.push_back((m_Health >> 8) & 0xFF);

        // Add driver ID (4 bytes)
        data.push_back((m_DriverID >> 0) & 0xFF);
        data.push_back((m_DriverID >> 8) & 0xFF);
        data.push_back((m_DriverID >> 16) & 0xFF);
        data.push_back((m_DriverID >> 24) & 0xFF);

        // Add passenger count and IDs
        data.push_back(static_cast<uint8_t>(m_PassengerIDs.size()));
        for (uint32_t passengerID : m_PassengerIDs) {
            data.push_back((passengerID >> 0) & 0xFF);
            data.push_back((passengerID >> 8) & 0xFF);
            data.push_back((passengerID >> 16) & 0xFF);
            data.push_back((passengerID >> 24) & 0xFF);
        }

        // Add engine state and colors
        data.push_back(m_EngineRunning ? 1 : 0);
        data.push_back(static_cast<uint8_t>(m_PrimaryColor));
        data.push_back(static_cast<uint8_t>(m_SecondaryColor));

        return data;
    }

    Vehicle* Vehicle::Deserialize(const std::vector<uint8_t>& data) {
        if (data.size() < 44) { // Minimum size for basic vehicle data
            return nullptr;
        }

        size_t offset = 0;

        // Read vehicle ID (4 bytes)
        uint32_t vehicleID = 0;
        vehicleID |= data[offset++] << 0;
        vehicleID |= data[offset++] << 8;
        vehicleID |= data[offset++] << 16;
        vehicleID |= data[offset++] << 24;

        // Read model (4 bytes)
        uint32_t model = 0;
        model |= data[offset++] << 0;
        model |= data[offset++] << 8;
        model |= data[offset++] << 16;
        model |= data[offset++] << 24;

        // Create vehicle object
        Vehicle* vehicle = new Vehicle(vehicleID, model);

        // Read position (12 bytes)
        if (offset + sizeof(Vector3) > data.size()) {
            delete vehicle;
            return nullptr;
        }

        Vector3 position;
        memcpy(&position, &data[offset], sizeof(Vector3));
        vehicle->SetPosition(position);
        offset += sizeof(Vector3);

        // Read rotation (16 bytes)
        if (offset + sizeof(Quaternion) > data.size()) {
            delete vehicle;
            return nullptr;
        }

        Quaternion rotation;
        memcpy(&rotation, &data[offset], sizeof(Quaternion));
        vehicle->SetRotation(rotation);
        offset += sizeof(Quaternion);

        // Read health (2 bytes)
        if (offset + 2 > data.size()) {
            delete vehicle;
            return nullptr;
        }

        int health = 0;
        health |= data[offset++] << 0;
        health |= data[offset++] << 8;
        vehicle->SetHealth(health);

        // Read driver ID (4 bytes)
        if (offset + 4 > data.size()) {
            delete vehicle;
            return nullptr;
        }

        uint32_t driverID = 0;
        driverID |= data[offset++] << 0;
        driverID |= data[offset++] << 8;
        driverID |= data[offset++] << 16;
        driverID |= data[offset++] << 24;

        if (driverID != 0) {
            vehicle->SetDriver(driverID);
        }

        // Read passengers
        if (offset + 1 > data.size()) {
            delete vehicle;
            return nullptr;
        }

        uint8_t passengerCount = data[offset++];
        if (offset + passengerCount * 4 > data.size()) {
            delete vehicle;
            return nullptr;
        }

        for (uint8_t i = 0; i < passengerCount; i++) {
            uint32_t passengerID = 0;
            passengerID |= data[offset++] << 0;
            passengerID |= data[offset++] << 8;
            passengerID |= data[offset++] << 16;
            passengerID |= data[offset++] << 24;

            vehicle->AddPassenger(passengerID);
        }

        // Read engine state and colors
        if (offset + 3 > data.size()) {
            delete vehicle;
            return nullptr;
        }

        vehicle->SetEngineRunning(data[offset++] != 0);
        vehicle->SetPrimaryColor(data[offset++]);
        vehicle->SetSecondaryColor(data[offset++]);

        return vehicle;
    }
}
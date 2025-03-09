#pragma once

#include "../../sdk/SDK.h"
#include <mutex>
#include <unordered_map>

namespace CustomMP {
    class VehicleManager {
    private:
        std::unordered_map<uint32_t, Vehicle*> m_Vehicles;
        std::mutex m_VehiclesMutex;
        uint32_t m_NextVehicleID;

    public:
        VehicleManager();
        ~VehicleManager();

        Vehicle* CreateVehicle(uint32_t model, const Vector3& position, const Quaternion& rotation);
        bool DestroyVehicle(uint32_t vehicleID);

        Vehicle* GetVehicle(uint32_t vehicleID);
        std::vector<Vehicle*> GetAllVehicles();

        void Update();
    };

    class Vehicle : public Entity {
    private:
        int m_Health;
        Vector3 m_Velocity;
        uint32_t m_DriverID;
        std::vector<uint32_t> m_PassengerIDs;
        bool m_EngineRunning;
        int m_PrimaryColor;
        int m_SecondaryColor;

    public:
        Vehicle(uint32_t id, uint32_t model);
        ~Vehicle();

        int GetHealth() const { return m_Health; }
        void SetHealth(int health) { m_Health = health; }

        const Vector3& GetVelocity() const { return m_Velocity; }
        void SetVelocity(const Vector3& velocity) { m_Velocity = velocity; }

        uint32_t GetDriverID() const { return m_DriverID; }
        void SetDriver(uint32_t playerID);
        void RemoveDriver();

        const std::vector<uint32_t>& GetPassengerIDs() const { return m_PassengerIDs; }
        void AddPassenger(uint32_t playerID);
        void RemovePassenger(uint32_t playerID);

        bool IsEngineRunning() const { return m_EngineRunning; }
        void SetEngineRunning(bool running) { m_EngineRunning = running; }

        int GetPrimaryColor() const { return m_PrimaryColor; }
        void SetPrimaryColor(int color) { m_PrimaryColor = color; }

        int GetSecondaryColor() const { return m_SecondaryColor; }
        void SetSecondaryColor(int color) { m_SecondaryColor = color; }

        // Serialize vehicle data for network transmission
        std::vector<uint8_t> Serialize() const;
        // Deserialize vehicle data from network transmission
        static Vehicle* Deserialize(const std::vector<uint8_t>& data);
    };
}
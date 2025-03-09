#pragma once

#include "../../sdk/SDK.h"
#include <mutex>
#include <unordered_map>

namespace CustomMP {
    class PlayerManager {
    private:
        std::unordered_map<uint32_t, Player*> m_Players;
        std::mutex m_PlayersMutex;

    public:
        PlayerManager();
        ~PlayerManager();

        Player* CreatePlayer(uint32_t clientID);
        bool DestroyPlayer(uint32_t clientID);

        Player* GetPlayer(uint32_t clientID);
        std::vector<Player*> GetAllPlayers();

        void Update();
    };

    class Player : public Entity {
    private:
        uint32_t m_ClientID;
        std::string m_Name;
        int m_Health;
        int m_Armor;
        Vector3 m_Velocity;
        bool m_IsInVehicle;
        uint32_t m_VehicleID;

    public:
        Player(uint32_t clientID, uint32_t model);
        ~Player();

        uint32_t GetClientID() const { return m_ClientID; }

        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        int GetHealth() const { return m_Health; }
        void SetHealth(int health) { m_Health = health; }

        int GetArmor() const { return m_Armor; }
        void SetArmor(int armor) { m_Armor = armor; }

        const Vector3& GetVelocity() const { return m_Velocity; }
        void SetVelocity(const Vector3& velocity) { m_Velocity = velocity; }

        bool IsInVehicle() const { return m_IsInVehicle; }
        uint32_t GetVehicleID() const { return m_VehicleID; }
        void SetVehicle(uint32_t vehicleID);
        void RemoveFromVehicle();

        // Serialize player data for network transmission
        std::vector<uint8_t> Serialize() const;
        // Deserialize player data from network transmission
        static Player* Deserialize(uint32_t clientID, const std::vector<uint8_t>& data);
    };
}
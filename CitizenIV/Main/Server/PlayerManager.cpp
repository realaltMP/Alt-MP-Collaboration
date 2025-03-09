#include "PlayerManager.h"
#include <iostream>

namespace CustomMP {
    // PlayerManager implementation
    PlayerManager::PlayerManager() {
    }

    PlayerManager::~PlayerManager() {
        // Clean up all players
        std::lock_guard<std::mutex> lock(m_PlayersMutex);
        for (auto& pair : m_Players) {
            delete pair.second;
        }
        m_Players.clear();
    }

    Player* PlayerManager::CreatePlayer(uint32_t clientID) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        // Check if player already exists
        auto it = m_Players.find(clientID);
        if (it != m_Players.end()) {
            std::cerr << "Player with client ID " << clientID << " already exists" << std::endl;
            return it->second;
        }

        // Default model ID for player (Michael in GTA:V)
        uint32_t playerModel = 0x0D7114C9;

        // Create new player
        Player* player = new Player(clientID, playerModel);
        m_Players[clientID] = player;

        std::cout << "Created player for client " << clientID << std::endl;
        return player;
    }

    bool PlayerManager::DestroyPlayer(uint32_t clientID) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(clientID);
        if (it == m_Players.end()) {
            std::cerr << "Player with client ID " << clientID << " not found" << std::endl;
            return false;
        }

        delete it->second;
        m_Players.erase(it);

        std::cout << "Destroyed player for client " << clientID << std::endl;
        return true;
    }

    Player* PlayerManager::GetPlayer(uint32_t clientID) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(clientID);
        if (it == m_Players.end()) {
            return nullptr;
        }

        return it->second;
    }

    std::vector<Player*> PlayerManager::GetAllPlayers() {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        std::vector<Player*> players;
        players.reserve(m_Players.size());

        for (const auto& pair : m_Players) {
            players.push_back(pair.second);
        }

        return players;
    }

    void PlayerManager::Update() {
        // Update logic for all players
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        for (auto& pair : m_Players) {
            // Custom update logic would go here
        }
    }

    // Player implementation
    Player::Player(uint32_t clientID, uint32_t model)
        : Entity(clientID, model)
        , m_ClientID(clientID)
        , m_Name("Player_" + std::to_string(clientID))
        , m_Health(100)
        , m_Armor(0)
        , m_IsInVehicle(false)
        , m_VehicleID(0)
    {
    }

    Player::~Player() {
    }

    void Player::SetVehicle(uint32_t vehicleID) {
        m_IsInVehicle = true;
        m_VehicleID = vehicleID;
    }

    void Player::RemoveFromVehicle() {
        m_IsInVehicle = false;
        m_VehicleID = 0;
    }

    std::vector<uint8_t> Player::Serialize() const {
        std::vector<uint8_t> data;

        // Add client ID (4 bytes)
        data.push_back((m_ClientID >> 0) & 0xFF);
        data.push_back((m_ClientID >> 8) & 0xFF);
        data.push_back((m_ClientID >> 16) & 0xFF);
        data.push_back((m_ClientID >> 24) & 0xFF);

        // Add model (4 bytes)
        data.push_back((m_Model >> 0) & 0xFF);
        data.push_back((m_Model >> 8) & 0xFF);
        data.push_back((m_Model >> 16) & 0xFF);
        data.push_back((m_Model >> 24) & 0xFF);

        // Add name length (1 byte) and name
        data.push_back(static_cast<uint8_t>(m_Name.size()));
        data.insert(data.end(), m_Name.begin(), m_Name.end());

        // Add position components (12 bytes)
        float posX = m_Position.x;
        float posY = m_Position.y;
        float posZ = m_Position.z;
        uint8_t* posXBytes = reinterpret_cast<uint8_t*>(&posX);
        uint8_t* posYBytes = reinterpret_cast<uint8_t*>(&posY);
        uint8_t* posZBytes = reinterpret_cast<uint8_t*>(&posZ);
        data.insert(data.end(), posXBytes, posXBytes + sizeof(float));
        data.insert(data.end(), posYBytes, posYBytes + sizeof(float));
        data.insert(data.end(), posZBytes, posZBytes + sizeof(float));

        // Add rotation components (16 bytes)
        float rotX = m_Rotation.x;
        float rotY = m_Rotation.y;
        float rotZ = m_Rotation.z;
        float rotW = m_Rotation.w;
        uint8_t* rotXBytes = reinterpret_cast<uint8_t*>(&rotX);
        uint8_t* rotYBytes = reinterpret_cast<uint8_t*>(&rotY);
        uint8_t* rotZBytes = reinterpret_cast<uint8_t*>(&rotZ);
        uint8_t* rotWBytes = reinterpret_cast<uint8_t*>(&rotW);
        data.insert(data.end(), rotXBytes, rotXBytes + sizeof(float));
        data.insert(data.end(), rotYBytes, rotYBytes + sizeof(float));
        data.insert(data.end(), rotZBytes, rotZBytes + sizeof(float));
        data.insert(data.end(), rotWBytes, rotWBytes + sizeof(float));

        // Add health and armor (2 bytes)
        data.push_back(static_cast<uint8_t>(m_Health));
        data.push_back(static_cast<uint8_t>(m_Armor));

        // Add vehicle state (5 bytes)
        data.push_back(m_IsInVehicle ? 1 : 0);
        data.push_back((m_VehicleID >> 0) & 0xFF);
        data.push_back((m_VehicleID >> 8) & 0xFF);
        data.push_back((m_VehicleID >> 16) & 0xFF);
        data.push_back((m_VehicleID >> 24) & 0xFF);

        return data;
    }

    Player* Player::Deserialize(uint32_t clientID, const std::vector<uint8_t>& data) {
        if (data.size() < 30) { // Minimum size for basic player data
            return nullptr;
        }

        size_t offset = 0;

        // Read client ID (4 bytes)
        uint32_t readClientID = 0;
        readClientID |= data[offset++] << 0;
        readClientID |= data[offset++] << 8;
        readClientID |= data[offset++] << 16;
        readClientID |= data[offset++] << 24;

        // Verify client ID matches
        if (readClientID != clientID) {
            return nullptr;
        }

        // Read model (4 bytes)
        uint32_t model = 0;
        model |= data[offset++] << 0;
        model |= data[offset++] << 8;
        model |= data[offset++] << 16;
        model |= data[offset++] << 24;

        // Create player object
        Player* player = new Player(clientID, model);

        // Read name
        uint8_t nameLength = data[offset++];
        if (offset + nameLength > data.size()) {
            delete player;
            return nullptr;
        }

        std::string name(data.begin() + offset, data.begin() + offset + nameLength);
        player->SetName(name);
        offset += nameLength;

        // Read position (12 bytes)
        if (offset + sizeof(Vector3) > data.size()) {
            delete player;
            return nullptr;
        }

        Vector3 position;
        memcpy(&position, &data[offset], sizeof(Vector3));
        player->SetPosition(position);
        offset += sizeof(Vector3);

        // Read rotation (16 bytes)
        if (offset + sizeof(Quaternion) > data.size()) {
            delete player;
            return nullptr;
        }

        Quaternion rotation;
        memcpy(&rotation, &data[offset], sizeof(Quaternion));
        player->SetRotation(rotation);
        offset += sizeof(Quaternion);

        // Read health and armor (2 bytes)
        if (offset + 2 > data.size()) {
            delete player;
            return nullptr;
        }

        player->SetHealth(data[offset++]);
        player->SetArmor(data[offset++]);

        // Read vehicle state (5 bytes)
        if (offset + 5 > data.size()) {
            delete player;
            return nullptr;
        }

        bool isInVehicle = data[offset++] != 0;

        uint32_t vehicleID = 0;
        vehicleID |= data[offset++] << 0;
        vehicleID |= data[offset++] << 8;
        vehicleID |= data[offset++] << 16;
        vehicleID |= data[offset++] << 24;

        if (isInVehicle) {
            player->SetVehicle(vehicleID);
        }

        return player;
    }
}
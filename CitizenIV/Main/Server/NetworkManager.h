#pragma once

#include "../../sdk/SDK.h"
#include <functional>
#include <unordered_map>
#include <mutex>

namespace CustomMP {
    // Packet types
    enum class PacketType : uint8_t {
        CONNECT_REQUEST = 0,
        CONNECT_ACCEPT = 1,
        CONNECT_REJECT = 2,
        DISCONNECT = 3,
        PLAYER_DATA = 4,
        VEHICLE_DATA = 5,
        WORLD_DATA = 6,
        CHAT_MESSAGE = 7,
        COMMAND = 8,
        CUSTOM_EVENT = 9
    };

    // Client connection information
    struct ClientInfo {
        uint32_t clientID;
        std::string ipAddress;
        uint16_t port;
        bool authenticated;
        int64_t lastPingTime;
    };

    // Event callback type
    using EventCallback = std::function<void(const ClientInfo&, const std::vector<uint8_t>&)>;

    class NetworkManager {
    private:
        uint16_t m_Port;
        bool m_Initialized;

        // sockets
        void* m_Socket;

        // Connected clients
        std::unordered_map<uint32_t, ClientInfo> m_Clients;

        // Event handlers
        std::unordered_map<PacketType, std::vector<EventCallback>> m_EventHandlers;

        // Mutexes for thread safety
        mutable std::mutex m_ClientsMutex;
        mutable std::mutex m_EventHandlersMutex;

    public:
        NetworkManager();
        ~NetworkManager();

        bool Initialize(uint16_t port = 22220);
        void Shutdown();

        void Update();

        bool SendToClient(uint32_t clientID, PacketType type, const std::vector<uint8_t>& data);
        bool SendToAll(PacketType type, const std::vector<uint8_t>& data, uint32_t excludeClientID = 0);

        void RegisterEventHandler(PacketType type, EventCallback callback);

        // Client management
        const ClientInfo* GetClientInfo(uint32_t clientID) const;
        bool DisconnectClient(uint32_t clientID, const std::string& reason);

    private:
        void ProcessIncomingPackets();
        void HandlePacket(const std::string& senderIP, uint16_t senderPort, const std::vector<uint8_t>& packetData);
    };
}
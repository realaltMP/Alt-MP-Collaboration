#include "NetworkManager.h"
#include <iostream>
#include <random>
#include <ctime>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace CustomMP {
    NetworkManager::NetworkManager()
        : m_Port(22220)
        , m_Initialized(false)
        , m_Socket(nullptr)
    {
    }

    NetworkManager::~NetworkManager() {
        Shutdown();
    }

    bool NetworkManager::Initialize(uint16_t port) {
        if (m_Initialized) {
            std::cerr << "Network manager already initialized" << std::endl;
            return false;
        }

        m_Port = port;

        // Initialize sockets
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize WinSock" << std::endl;
            return false;
        }
#endif

        // Create UDP socket
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#ifdef _WIN32
        if (sock == INVALID_SOCKET) {
            std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return false;
        }
#else
        if (sock < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
#endif

        // Bind socket to port
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(m_Port);

#ifdef _WIN32
        if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Failed to bind socket: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            WSACleanup();
            return false;
        }
#else
        if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            close(sock);
            return false;
        }
#endif

        // Set non-blocking mode
#ifdef _WIN32
        u_long mode = 1;
        if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR) {
            std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            WSACleanup();
            return false;
        }
#else
        int flags = fcntl(sock, F_GETFL, 0);
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
            std::cerr << "Failed to set non-blocking mode" << std::endl;
            close(sock);
            return false;
        }
#endif

        m_Socket = reinterpret_cast<void*>(sock);
        m_Initialized = true;

        std::cout << "Network manager initialized on port " << m_Port << std::endl;
        return true;
    }

    void NetworkManager::Shutdown() {
        if (!m_Initialized) {
            return;
        }

        // Close socket
#ifdef _WIN32
        closesocket(reinterpret_cast<SOCKET>(m_Socket));
        WSACleanup();
#else
        close(reinterpret_cast<int>(m_Socket));
#endif

        m_Socket = nullptr;
        m_Initialized = false;

        // Clear clients
        m_ClientsMutex.lock();
        m_Clients.clear();
        m_ClientsMutex.unlock();

        std::cout << "Network manager shut down" << std::endl;
    }

    void NetworkManager::Update() {
        if (!m_Initialized) {
            return;
        }

        // Process incoming packets
        ProcessIncomingPackets();
    }

    bool NetworkManager::SendToClient(uint32_t clientID, PacketType type, const std::vector<uint8_t>& data) {
        if (!m_Initialized) {
            return false;
        }

        // Find client
        const ClientInfo* client = GetClientInfo(clientID);
        if (!client) {
            return false;
        }

        // Create packet (type + data)
        std::vector<uint8_t> packet;
        packet.push_back(static_cast<uint8_t>(type));
        packet.insert(packet.end(), data.begin(), data.end());

        // Send data to client
        sockaddr_in clientAddr;
        clientAddr.sin_family = AF_INET;
        clientAddr.sin_port = htons(client->port);
        inet_pton(AF_INET, client->ipAddress.c_str(), &clientAddr.sin_addr);

#ifdef _WIN32
        if (sendto(reinterpret_cast<SOCKET>(m_Socket),
            reinterpret_cast<const char*>(packet.data()),
            static_cast<int>(packet.size()),
            0,
            reinterpret_cast<sockaddr*>(&clientAddr),
            sizeof(clientAddr)) == SOCKET_ERROR) {
            std::cerr << "Failed to send data to client " << clientID << ": " << WSAGetLastError() << std::endl;
            return false;
        }
#else
        if (sendto(reinterpret_cast<int>(m_Socket),
            packet.data(),
            packet.size(),
            0,
            reinterpret_cast<sockaddr*>(&clientAddr),
            sizeof(clientAddr)) < 0) {
            std::cerr << "Failed to send data to client " << clientID << std::endl;
            return false;
        }
#endif

        return true;
    }

    bool NetworkManager::SendToAll(PacketType type, const std::vector<uint8_t>& data, uint32_t excludeClientID) {
        if (!m_Initialized) {
            return false;
        }

        bool success = true;

        m_ClientsMutex.lock();
        for (const auto& pair : m_Clients) {
            if (pair.first != excludeClientID) {
                if (!SendToClient(pair.first, type, data)) {
                    success = false;
                }
            }
        }
        m_ClientsMutex.unlock();

        return success;
    }

    void NetworkManager::RegisterEventHandler(PacketType type, EventCallback callback) {
        m_EventHandlers[type].push_back(callback);
    }

    const ClientInfo* NetworkManager::GetClientInfo(uint32_t clientID) const {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);

        auto it = m_Clients.find(clientID);
        if (it == m_Clients.end()) {
            return nullptr;
        }

        return &it->second;
    }

    bool NetworkManager::DisconnectClient(uint32_t clientID, const std::string& reason) {
        const ClientInfo* client = GetClientInfo(clientID);
        if (!client) {
            return false;
        }

        // Send disconnect packet
        std::vector<uint8_t> reasonData(reason.begin(), reason.end());
        SendToClient(clientID, PacketType::DISCONNECT, reasonData);

        // Remove client
        m_ClientsMutex.lock();
        m_Clients.erase(clientID);
        m_ClientsMutex.unlock();

        std::cout << "Client " << clientID << " disconnected: " << reason << std::endl;
        return true;
    }

    void NetworkManager::ProcessIncomingPackets() {
        if (!m_Initialized) {
            return;
        }

        const int MAX_PACKET_SIZE = 4096;
        uint8_t buffer[MAX_PACKET_SIZE];

        sockaddr_in senderAddr;
        socklen_t senderAddrLen = sizeof(senderAddr);

        // Process up to 10 packets per frame
        for (int i = 0; i < 10; i++) {
#ifdef _WIN32
            int bytesReceived = recvfrom(reinterpret_cast<SOCKET>(m_Socket),
                reinterpret_cast<char*>(buffer),
                MAX_PACKET_SIZE,
                0,
                reinterpret_cast<sockaddr*>(&senderAddr),
                &senderAddrLen);

            if (bytesReceived == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
                    // No more data available
                    break;
                }
                std::cerr << "Error receiving data: " << error << std::endl;
                continue;
            }
#else
            int bytesReceived = recvfrom(reinterpret_cast<int>(m_Socket),
                buffer,
                MAX_PACKET_SIZE,
                0,
                reinterpret_cast<sockaddr*>(&senderAddr),
                &senderAddrLen);

            if (bytesReceived < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    // No more data available
                    break;
                }
                std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
                continue;
            }
#endif

            if (bytesReceived > 0) {
                // Get sender info
                char senderIP[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &senderAddr.sin_addr, senderIP, INET_ADDRSTRLEN);
                uint16_t senderPort = ntohs(senderAddr.sin_port);

                // Copy data to vector
                std::vector<uint8_t> packetData(buffer, buffer + bytesReceived);

                // Handle packet
                HandlePacket(senderIP, senderPort, packetData);
            }
        }
    }

    void NetworkManager::HandlePacket(const std::string& senderIP, uint16_t senderPort, const std::vector<uint8_t>& packetData) {
        if (packetData.empty()) {
            return;
        }

        // Extract packet type
        PacketType type = static_cast<PacketType>(packetData[0]);

        // Extract payload data (everything after the type byte)
        std::vector<uint8_t> payload;
        if (packetData.size() > 1) {
            payload.assign(packetData.begin() + 1, packetData.end());
        }

        // Handle connection request
        if (type == PacketType::CONNECT_REQUEST) {
            // Generate a new client ID
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint32_t> dist(1, UINT32_MAX);
            uint32_t clientID = dist(gen);

            // Add client
            ClientInfo client;
            client.clientID = clientID;
            client.ipAddress = senderIP;
            client.port = senderPort;
            client.authenticated = false;
            client.lastPingTime = std::time(nullptr);

            m_ClientsMutex.lock();
            m_Clients[clientID] = client;
            m_ClientsMutex.unlock();

            // Send connection accept message
            std::vector<uint8_t> clientIDBytes(4);
            clientIDBytes[0] = (clientID >> 0) & 0xFF;
            clientIDBytes[1] = (clientID >> 8) & 0xFF;
            clientIDBytes[2] = (clientID >> 16) & 0xFF;
            clientIDBytes[3] = (clientID >> 24) & 0xFF;

            SendToClient(clientID, PacketType::CONNECT_ACCEPT, clientIDBytes);

            std::cout << "New client connected: " << clientID << " from " << senderIP << ":" << senderPort << std::endl;
            return;
        }

        // For other packet types, find client by IP and port
        uint32_t clientID = 0;
        m_ClientsMutex.lock();
        for (const auto& pair : m_Clients) {
            if (pair.second.ipAddress == senderIP && pair.second.port == senderPort) {
                clientID = pair.first;
                break;
            }
        }
        m_ClientsMutex.unlock();

        if (clientID == 0) {
            std::cerr << "Received packet from unknown client: " << senderIP << ":" << senderPort << std::endl;
            return;
        }

        // Update last ping time
        m_ClientsMutex.lock();
        m_Clients[clientID].lastPingTime = std::time(nullptr);
        m_ClientsMutex.unlock();

        // Find event handlers for this packet type
        auto it = m_EventHandlers.find(type);
        if (it != m_EventHandlers.end()) {
            const ClientInfo& clientInfo = m_Clients[clientID];

            // Call all event handlers
            for (const auto& callback : it->second) {
                callback(clientInfo, payload);
            }
        }
    }
}
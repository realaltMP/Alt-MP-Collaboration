#pragma once

#include "../../sdk/SDK.h"
#include "NetworkManager.h"
#include "PlayerManager.h"
#include "VehicleManager.h"
#include "World.h"

namespace CustomMP {
    class Server {
    private:
        NetworkManager* m_NetworkManager;
        PlayerManager* m_PlayerManager;
        VehicleManager* m_VehicleManager;
        World* m_World;

        bool m_Running;

    public:
        Server();
        ~Server();

        bool Initialize(const std::string& configFile);
        void Shutdown();

        void Run();
        void Stop();

        NetworkManager* GetNetworkManager() const { return m_NetworkManager; }
        PlayerManager* GetPlayerManager() const { return m_PlayerManager; }
        VehicleManager* GetVehicleManager() const { return m_VehicleManager; }
        World* GetWorld() const { return m_World; }
    };
}
#include "Server.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace CustomMP {
    Server::Server()
        : m_NetworkManager(nullptr)
        , m_PlayerManager(nullptr)
        , m_VehicleManager(nullptr)
        , m_World(nullptr)
        , m_Running(false)
    {
    }

    Server::~Server() {
        Shutdown();
    }

    bool Server::Initialize(const std::string& configFile) {
        std::cout << "Initializing server with config: " << configFile << std::endl;

        // Create managers
        m_NetworkManager = new NetworkManager();
        m_PlayerManager = new PlayerManager();
        m_VehicleManager = new VehicleManager();
        m_World = new World();

        // Initialize network
        if (!m_NetworkManager->Initialize()) {
            std::cerr << "Failed to initialize network manager" << std::endl;
            return false;
        }

        std::cout << "Server initialized successfully" << std::endl;
        return true;
    }

    void Server::Shutdown() {
        std::cout << "Shutting down server..." << std::endl;

        // Stop server if running
        if (m_Running) {
            Stop();
        }

        // Clean up resources
        delete m_NetworkManager;
        m_NetworkManager = nullptr;

        delete m_PlayerManager;
        m_PlayerManager = nullptr;

        delete m_VehicleManager;
        m_VehicleManager = nullptr;

        delete m_World;
        m_World = nullptr;

        std::cout << "Server shutdown complete" << std::endl;
    }

    void Server::Run() {
        if (m_Running) {
            std::cerr << "Server is already running" << std::endl;
            return;
        }

        m_Running = true;
        std::cout << "Server running..." << std::endl;

        // Main server loop
        while (m_Running) {
            // Update network
            m_NetworkManager->Update();

            // Update world
            m_World->Update();

            // Sleep to avoid high CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void Server::Stop() {
        if (!m_Running) {
            std::cerr << "Server is not running" << std::endl;
            return;
        }

        m_Running = false;
        std::cout << "Server stopped" << std::endl;
    }
}
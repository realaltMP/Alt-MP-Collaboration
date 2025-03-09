#include "Server/Server.h"
#include <iostream>
#include <string>
#include <csignal>

using namespace CustomMP;

// Global server instance
Server* g_Server = nullptr;

// Signal handler for clean shutdown
void SignalHandler(int signal) {
    std::cout << "Received signal " << signal << ", shutting down..." << std::endl;

    if (g_Server) {
        g_Server->Shutdown();
        delete g_Server;
        g_Server = nullptr;
    }

    exit(signal);
}

int main(int argc, char** argv) {
    std::cout << "altmp" << std::endl;
    std::cout << "SDK Version: " << SDK_VERSION << std::endl;

    // Register signal handlers
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    // Parse command line arguments
    std::string configFile = "server.cfg";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--config" && i + 1 < argc) {
            configFile = argv[++i];
        }
    }

    // Create and initialize server
    g_Server = new Server();

    if (!g_Server->Initialize(configFile)) {
        std::cerr << "Failed to initialize server" << std::endl;
        delete g_Server;
        return 1;
    }

    // Run server
    g_Server->Run();

    // Cleanup
    g_Server->Shutdown();
    delete g_Server;

    return 0;
}
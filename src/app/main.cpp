#include "Application.hpp"

#include <csignal>
#include <iostream>

static csx::Application* g_app = nullptr;

void signalHandler(int signal) {
    (void)signal;
    if (g_app) {
        std::cout << "\nShutting down..." << std::endl;
        g_app->shutdown();
    }
    std::exit(0);
}

int main(int argc, char* argv[]) {
    std::cout << "=== Cyber Sentinel X v1.0.0 ===" << std::endl;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    csx::Application app;
    g_app = &app;

    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }

#ifdef CSX_BUILD_UI
    std::cout << "Launching tactical dashboard UI..." << std::endl;
    return app.runWithUi(argc, argv);
#else
    std::cout << "Console Mode (Qt UI disabled)" << std::endl;
    std::cout << "================================" << std::endl;
    std::cout << "Application initialized successfully" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    app.run();
    std::cout << "Shutting down..." << std::endl;
    app.shutdown();
    return 0;
#endif
}

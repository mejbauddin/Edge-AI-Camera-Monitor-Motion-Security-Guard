#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>

#include "config/ConfigService.hpp"
#include "events/EventBusFactory.hpp"

using csx::core::ConfigService;
using csx::core::createConfigService;
using csx::core::createEventBus;

TEST(ConfigServiceTest, LoadsValidJsonConfig) {
    const std::string path = "test_config_runtime.json";
    std::ofstream file(path);
    file << R"({
        "application": {"name": "Cyber Sentinel X", "target_fps": 60},
        "camera": {"width": 1920, "height": 1080, "fps": 30},
        "threat": {"levels": {"green_max": 20, "yellow_max": 40, "orange_max": 60, "red_max": 80}},
        "ui": {"boot_sequence": true}
    })";
    file.close();

    auto config = createConfigService();
    EXPECT_TRUE(config->load(path));
    EXPECT_EQ(config->getInt("camera.width", 0), 1920);
    EXPECT_EQ(config->getString("application.name", ""), "Cyber Sentinel X");
    EXPECT_TRUE(config->getBool("ui.boot_sequence", false));

    std::remove(path.c_str());
}

TEST(ConfigServiceTest, PublishesReloadEvent) {
    const std::string path = "test_config_reload.json";
    std::ofstream file(path);
    file << R"({
        "application": {"name": "CSX", "target_fps": 60},
        "camera": {"width": 1280, "height": 720, "fps": 60},
        "threat": {"levels": {"green_max": 20, "yellow_max": 40, "orange_max": 60, "red_max": 80}},
        "ui": {"boot_sequence": false}
    })";
    file.close();

    auto bus = createEventBus();
    int reloadCount = 0;
    bus->subscribe("config.reloaded", [&](const std::string&, const std::string&) { ++reloadCount; });

    auto config = createConfigService(bus);
    EXPECT_TRUE(config->load(path));
    EXPECT_TRUE(config->reload());
    EXPECT_EQ(reloadCount, 2);

    std::remove(path.c_str());
}

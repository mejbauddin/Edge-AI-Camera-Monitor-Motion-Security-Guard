#include <gtest/gtest.h>

#include "events/EventBusFactory.hpp"

using csx::core::createEventBus;

TEST(EventBusTest, DeliversPublishedEvents) {
    auto bus = createEventBus();
    int counter = 0;
    bus->subscribe("threat.elevated", [&](const std::string&, const std::string& payload) {
        if (payload == "ORANGE") {
            ++counter;
        }
    });

    bus->publish("threat.elevated", "ORANGE");
    bus->publish("threat.elevated", "GREEN");
    EXPECT_EQ(counter, 1);
}

TEST(EventBusTest, UnsubscribeClearsTopic) {
    auto bus = createEventBus();
    int counter = 0;
    bus->subscribe("defcon.changed", [&](const std::string&, const std::string&) { ++counter; });
    bus->unsubscribeAll("defcon.changed");
    bus->publish("defcon.changed", "DEFCON 3");
    EXPECT_EQ(counter, 0);
}

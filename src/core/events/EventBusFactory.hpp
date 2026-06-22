#pragma once

#include "events/EventBus.hpp"

#include <memory>

namespace csx::core {

std::shared_ptr<IEventBus> createEventBus();

}  // namespace csx::core

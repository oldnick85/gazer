#pragma once

#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time
#include <memory>

#include "logging.h"

constexpr bool DEBUG_MODE = false;
constexpr bool DEBUG_LOGS = DEBUG_MODE;
constexpr uint ACTOR_CYCLE_MS = (DEBUG_MODE) ? 1000 : 100;

class CApplication;
std::shared_ptr<CApplication> app();
class Configuration;
std::shared_ptr<Configuration> config();

#pragma once

#include <chrono>   // chrono::system_clock
#include <ctime>    // localtime
#include <iomanip>  // put_time
#include <memory>
#include <sstream>  // stringstream

#include "logging.h"

class CApplication;
std::shared_ptr<CApplication> app();
class Configuration;
std::shared_ptr<Configuration> config();

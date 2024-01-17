#pragma once

#include "appconfig.h"

#include <string>

class Configuration
{
    static inline const std::string TAG = "CNFG";
    
public:
    Configuration(const std::string &path);

    const sConfig &Get() const { return m_config; }
    bool Valid() const { return m_valid; }

private:
    bool Parse(const std::string &path);

private:
    sConfig m_config;
    bool m_valid = false;
};

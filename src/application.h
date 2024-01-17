#pragma once

#include "appconfig.h"

#include "actors/http/httpserver.h"
#include "actors/collector/collector.h"
#include "actors/data/data.h"

#include <memory>

class CApplication
{
    static inline const std::string TAG = "APP";

    static inline const std::string APP_VERSION = "1.0";

public:
    CApplication();
    ~CApplication();

    bool Run();
    void Stop();

private:
    std::unique_ptr<Collector>  m_collector = nullptr;
    DataPtr                     m_data = nullptr;
    std::unique_ptr<Server>     m_http = nullptr;

    std::atomic_bool            m_stop{false};
};

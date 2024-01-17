#pragma once

#include "common/logging.h"
#include "actors/data/data.h"
#include "config/config.h"

class Collector
{
    static inline const std::string TAG = "CLCT";
public:
    Collector(DataPtr data);
    ~Collector();

    std::thread Run();
    void Stop();

private:
    void Thread();
    void InitData();
    void CollectValue(Value& value, const sConfig::sValue& config_value) const;
    void AddValue(const float v, Value& value, const sConfig::sValue& config_value) const;
    static std::string ExecCommand(const std::string cmd, int& out_exitStatus);

private:
    DataPtr             m_data;
    std::atomic_bool    m_stop_thread{false};
};
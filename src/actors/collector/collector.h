#pragma once

#include "actors/data/data.h"
#include "common/logging.h"
#include "config/config.h"

class Collector
{
    static inline const std::string TAG = "CLCT";

  public:
    Collector(DataPtr& data);
    ~Collector();

    std::thread Run();
    void Stop();

  private:
    void Thread();
    void InitData();
    static void CollectValue(Value& value, const sConfig::sValue& config_value);
    static void AddValue(const float val, Value& value, const sConfig::sValue& config_value);
    static std::string ExecCommand(const std::string& cmd, int& out_exitStatus);

  private:
    DataPtr m_data;
    std::atomic_bool m_stop_thread{false};
    std::multimap<std::chrono::steady_clock::time_point, size_t> m_times;
};
#pragma once

#include "global.h"

namespace pulsar_statistic
{

using Time_t      = std::chrono::steady_clock;
using Timepoint_t = std::chrono::time_point<Time_t>;
using Duration_t  = std::chrono::duration<double>;

class CCounter
{
  public:
    void Add()
    {
        m_counter++;
        m_counter_sec++;
        CalcSpeed();
    }

    void CalcSpeed()
    {
        const auto now    = Time_t::now();
        const auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_second).count();
        if (dur_ms < 1000)
            return;
        m_speed       = (m_counter_sec * 1000) / dur_ms;
        m_counter_sec = 0;
        m_last_second = now;
    }

    uint Count() const { return m_counter; }

    uint Speed() const { return m_speed; }

    std::string to_str() const
    {
        std::string s = std::string("count=") + std::to_string(m_counter) + " speed=" + std::to_string(m_speed);
        return s;
    }

  private:
    uint m_counter     = 0;
    uint m_counter_sec = 0;
    uint m_speed       = 0;
    Timepoint_t m_last_second;
};

class CDuration
{
  public:
    void AddTime(const Duration_t d)
    {
        if (d < m_min)
            m_min = d;
        if (d > m_max)
            m_max = d;
        m_mean = (3 * m_mean + d) / 4;
        m_sigm = (3 * m_sigm + abs(m_mean - d)) / 4;
    }

    uint Min_usec() const { return std::chrono::duration_cast<std::chrono::microseconds>(m_min).count(); }

    uint Max_usec() const { return std::chrono::duration_cast<std::chrono::microseconds>(m_max).count(); }

    uint Mean_usec() const { return std::chrono::duration_cast<std::chrono::microseconds>(m_mean).count(); }

    uint Sigm_usec() const { return std::chrono::duration_cast<std::chrono::microseconds>(m_sigm).count(); }

    std::string to_str() const
    {
        std::string s =
            std::string("min=") + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(m_min).count()) +
            " " + "m(d)=" + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(m_mean).count()) + "" +
            "(" + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(m_sigm).count()) + ") " +
            "max=" + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(m_max).count());
        return s;
    }

  private:
    Duration_t m_max  = Duration_t::min();
    Duration_t m_min  = Duration_t::max();
    Duration_t m_mean = Duration_t::zero();
    Duration_t m_sigm = Duration_t::zero();
};

class CDurationWatch
{
  public:
    CDurationWatch(CDuration* d) : m_d(d) { m_start = Time_t::now(); }

    ~CDurationWatch() { m_d->AddTime(Time_t::now() - m_start); }

  private:
    CDuration* m_d = nullptr;
    Timepoint_t m_start;
};
using CDurationWatchPtr = std::shared_ptr<CDurationWatch>;

class C_HTTP
{
  public:
    void NewRegistry() { m_requests_registry.Add(); }

    void NewVerify() { m_requests_verify.Add(); }

    void RegistryDuration(const Duration_t d) { m_duration_registry.AddTime(d); }

    void VerifyDuration(const Duration_t d) { m_duration_verify.AddTime(d); }

    CDurationWatchPtr GetRegistryWatch()
    {
        auto dw = std::make_shared<CDurationWatch>(&m_duration_registry);
        return dw;
    }

    CDurationWatchPtr GetVerifyWatch()
    {
        auto dw = std::make_shared<CDurationWatch>(&m_duration_verify);
        return dw;
    }

    std::string to_str() const
    {
        std::string s = std::string("REGISTRY ") + m_requests_registry.to_str() + " " + m_duration_registry.to_str() +
                        " " + std::string("VERIFY ") + m_requests_verify.to_str() + " " + m_duration_verify.to_str() +
                        " ";
        return s;
    }

    std::string to_html() const
    {
        std::string s;
        s += "<p>";
        s += std::string("REGISTRY ") + m_requests_registry.to_str() + " " + m_duration_registry.to_str();
        s += "</p>";
        s += "<p>";
        s += std::string("VERIFY ") + m_requests_verify.to_str() + " " + m_duration_verify.to_str() + " ";
        s += "</p>";
        return s;
    }

    const CCounter& RegistryCounter() const { return m_requests_registry; }

    const CCounter& VerifyCounter() const { return m_requests_verify; }

    const CDuration& RegistryDuration() const { return m_duration_registry; }

    const CDuration& VerifyDuration() const { return m_duration_verify; }

  private:
    CCounter m_requests_registry;
    CCounter m_requests_verify;
    CDuration m_duration_registry;
    CDuration m_duration_verify;
};

}  // namespace pulsar_statistic
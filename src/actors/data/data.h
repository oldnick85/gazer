#pragma once

#include <list>
#include <regex>

#include "common/logging.h"

struct sHistorySample {
    std::chrono::time_point<std::chrono::system_clock> system_time;
    std::chrono::time_point<std::chrono::steady_clock> timepoint;
    float value;
};

using History_t = std::list<sHistorySample>;

class Chunk
{
  public:
    Chunk(size_t size) : m_size(size)
    {
        m_values.reserve(m_size);
        m_time = std::chrono::system_clock::now();
    }

    void AddValue(float v)
    {
        if (m_full)
            return;
        m_values.push_back(v);
        m_full = (m_values.size() >= m_size);
        Calc();
        if (m_full)
            m_values.clear();
    }

    bool Full() const { return m_full; }

    float Min() const { return m_min; }
    float Max() const { return m_max; }
    float Mean() const { return m_mean; }

    std::chrono::time_point<std::chrono::system_clock> Time() const { return m_time; }

  private:
    void Calc()
    {
        float min  = INFINITY;
        float max  = -INFINITY;
        float mean = 0.0;
        for (const auto v : m_values)
        {
            if (v < min)
                min = v;
            if (v > max)
                max = v;
            mean += v;
        }
        mean /= m_values.size();
        m_min  = min;
        m_max  = max;
        m_mean = mean;
    }

  private:
    size_t m_size = 0;
    std::vector<float> m_values;
    float m_min  = 0.0;
    float m_max  = 0.0;
    float m_mean = 0.0;
    bool m_full  = false;
    std::chrono::time_point<std::chrono::system_clock> m_time;
};

class Plot
{
  public:
    Plot(std::string title, std::string type, size_t chunk_size, size_t chunks_count)
        : m_title(title), m_type(type), m_chunk_size(chunk_size), m_chunks_count(chunks_count)
    {}

    void AddValue(float v)
    {
        if (m_chunks.empty() or m_chunks.back().Full())
        {
            Chunk chunk(m_chunk_size);
            m_chunks.push_back(chunk);
        }

        m_chunks.back().AddValue(v);

        if (m_chunks.size() > m_chunks_count)
            m_chunks.pop_front();
    }

    std::list<Chunk> GetChunks() const { return m_chunks; }

    std::string Type() const { return m_type; }
    std::string Title() const { return m_title; }

  private:
    std::string m_title;
    std::string m_type;
    size_t m_chunk_size   = 0;
    size_t m_chunks_count = 0;
    std::list<Chunk> m_chunks;
};

class Value
{
  public:
    Value() = default;

    Value(const Value& other)
    {
        name    = other.name;
        history = other.history;
        regex   = other.regex;
        plots   = other.plots;
    }

    History_t CopyHistory() const
    {
        const std::lock_guard<std::mutex> lock(mtx);
        return history;
    }

    void SetHistory(History_t& h)
    {
        const std::lock_guard<std::mutex> lock(mtx);
        history = std::move(h);
    }

    std::string name;
    std::chrono::time_point<std::chrono::steady_clock> last_proc;
    std::regex regex;
    std::vector<Plot> plots;

  private:
    mutable std::mutex mtx;
    History_t history;
};

class Data
{
    static inline const std::string TAG = "DATA";

  public:
    Data();
    ~Data();

    std::vector<Value> values;

  private:
};
using DataPtr = std::shared_ptr<Data>;
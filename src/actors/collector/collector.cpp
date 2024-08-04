#include "collector.h"

#include "global.h"

Collector::Collector(DataPtr& data) : m_data(data)
{
    spdlog::info("[{}] {}", TAG, __FUNCTION__);
    InitData();
}

Collector::~Collector()
{
    Stop();
}

void Collector::InitData()
{
    auto cnf        = config();
    size_t time_pos = 0;
    const auto now  = std::chrono::steady_clock::now();
    for (const auto& val : cnf->Get().values)
    {
        Value value;
        value.name  = val.name;
        value.regex = std::regex(val.output_regex);
        for (const auto& plt : val.plots)
        {
            Plot plot{plt.title, plt.type, plt.chunk_size, plt.chunks_count};
            value.plots.push_back(plot);
        }
        spdlog::info("[{}] add value {}", TAG, value.name);
        m_data->values.push_back(value);
        m_times.emplace(now, time_pos);
        ++time_pos;
    }
}

std::thread Collector::Run()
{
    std::thread collector_thread{&Collector::Thread, this};
    return collector_thread;
}

void Collector::Stop()
{
    m_stop_thread = true;
}

void Collector::Thread()
{
    spdlog::trace("[{}] {}", TAG, __FUNCTION__);
    pthread_setname_np(pthread_self(), "CLCT");
    while (not m_stop_thread)
    {
        spdlog::trace("[{}] {}", TAG, __FUNCTION__);

        {
            const auto now = std::chrono::steady_clock::now();
            auto times_it  = m_times.begin();
            while (times_it->first <= now)
            {
                const auto val_i         = times_it->second;
                const auto& config_value = config()->Get().values[val_i];
                auto& value              = m_data->values[val_i];
                const auto time          = times_it->first + std::chrono::seconds(config_value.collect_delay_sec);
                CollectValue(value, config_value);
                m_times.emplace(time, val_i);
                times_it = m_times.erase(times_it);
            }
        }

        const auto now = std::chrono::steady_clock::now();
        assert(m_times.begin()->first > now);
        const auto delay = m_times.begin()->first - now;
        std::this_thread::sleep_for(delay);
    }
    spdlog::trace("[{}] {} end", TAG, __FUNCTION__);
}

void Collector::CollectValue(Value& value, const sConfig::sValue& config_value)
{
    spdlog::debug("[{}] {} {}", TAG, __FUNCTION__, config_value.name);
    value.last_proc = std::chrono::steady_clock::now();
    int exit_status;
    auto str_out = ExecCommand(config_value.bash_command, exit_status);
    if (exit_status != 0)
    {
        spdlog::error("[{}] value {} command {} return nonzero exit code {}", TAG, config_value.name,
                      config_value.bash_command, exit_status);
        return;
    }

    if ((not str_out.empty()) and (str_out[str_out.length() - 1] == '\n'))
    {
        str_out.erase(str_out.length() - 1);
    }
    spdlog::trace("[{}] output: {}", TAG, str_out);

    float val = 0.0;
    if (config_value.output_regex.empty())
    {
        val = std::stof(str_out);
    }
    else
    {
        std::smatch output_match;
        if (std::regex_match(str_out, output_match, value.regex))
        {
            val = std::stof(output_match[1].str());
        }
        else
        {
            spdlog::error("[{}] cant parse regex:\n{}\noutput:\n{}", TAG, config_value.output_regex, str_out);
            return;
        }
    }
    AddValue(val, value, config_value);
}

void Collector::AddValue(const float val, Value& value, const sConfig::sValue& config_value)
{
    spdlog::trace("[{}] got value {}", TAG, value.name);
    auto history   = value.CopyHistory();
    const auto now = std::chrono::steady_clock::now();
    sHistorySample sample;
    sample.system_time = std::chrono::system_clock::now();
    sample.timepoint   = now;
    sample.value       = val;
    history.push_back(sample);
    if (history.size() > config_value.collect_count)
        history.pop_front();
    value.SetHistory(history);
    for (auto& plot : value.plots)
    {
        plot.AddValue(val);
    }
}

std::string Collector::ExecCommand(const std::string& cmd, int& out_exitStatus)
{
    out_exitStatus = 0;
    auto* pPipe    = ::popen(cmd.c_str(), "r");
    if (pPipe == nullptr)
    {
        throw std::runtime_error("Cannot open pipe");
    }

    constexpr uint buffer_size = 256;
    std::array<char, buffer_size> buffer;

    std::string result;

    while (std::feof(pPipe) == 0)
    {
        auto bytes = std::fread(buffer.data(), 1, buffer.size(), pPipe);
        result.append(buffer.data(), bytes);
    }

    auto res = ::pclose(pPipe);

    if (WIFEXITED(res))
    {
        out_exitStatus = WEXITSTATUS(res);
    }

    return result;
}

#include "collector.h"

#include "global.h"

Collector::Collector(DataPtr data)
    : m_data(data)
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
    auto cnf = config();
    size_t i = 0;
    const auto now = std::chrono::steady_clock::now();
    for (const auto& v : cnf->Get().values)
    {
        Value value;
        value.name = v.name;
        value.regex = std::regex(v.output_regex);
        for (const auto& p : v.plots)
        {
            Plot plot{p.title, p.type, p.chunk_size, p.chunks_count};
            value.plots.push_back(plot);
        }
        spdlog::info("[{}] add value {}", TAG, value.name);
        m_data->values.push_back(value);
        m_times.emplace(now, i);
        i++;
    }
}

std::thread Collector::Run()
{
    std::thread t{&Collector::Thread, this};
    return std::move(t);
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
            auto it = m_times.begin();
            while (it->first <= now)
            {
                const auto i = it->second;
                auto& config_value = config()->Get().values[i];
                auto& value = m_data->values[i];
                const auto tm = it->first + std::chrono::seconds(config_value.collect_delay_sec);
                CollectValue(value, config_value);
                m_times.emplace(tm, i);
                it = m_times.erase(it);
            }
        }

        const auto now = std::chrono::steady_clock::now();
        assert(m_times.begin()->first > now);
        const auto d = m_times.begin()->first - now;
        std::this_thread::sleep_for(d);
    }
    spdlog::trace("[{}] {} end", TAG, __FUNCTION__);
}

void Collector::CollectValue(Value& value, const sConfig::sValue& config_value) const
{
    spdlog::debug("[{}] {} {}", TAG, __FUNCTION__, config_value.name);
    value.last_proc = std::chrono::steady_clock::now();
    int exit_status;
    auto str_out = ExecCommand(config_value.bash_command, exit_status);
    if (exit_status != 0)
    {
        spdlog::error("[{}] value {} command {} return nonzero exit code {}", TAG, config_value.name, config_value.bash_command, exit_status);
        return;
    }

    if ((not str_out.empty()) and (str_out[str_out.length()-1] == '\n')) 
    {
        str_out.erase(str_out.length()-1);
    }
    spdlog::trace("[{}] output: {}", TAG, str_out);

    float v = 0.0;
    if (config_value.output_regex.empty())
    {
        v = std::stof(str_out);
    }
    else
    {
        std::smatch output_match;
        if (std::regex_match(str_out, output_match, value.regex))
        {
            v = std::stof(output_match[1].str());
        }
        else
        {
            spdlog::error("[{}] cant parse regex:\n{}\noutput:\n{}", TAG, config_value.output_regex, str_out);
            return;
        }
    }
    AddValue(v, value, config_value);
}

void Collector::AddValue(const float v, Value& value, const sConfig::sValue& config_value) const
{
    spdlog::trace("[{}] got value {}", TAG, v);
    auto history = value.CopyHistory();
    const auto now = std::chrono::steady_clock::now();
    sHistorySample sample;
    sample.system_time = std::chrono::system_clock::now();
    sample.timepoint = now;
    sample.value = v;
    history.push_back(sample);
    if (history.size() > config_value.collect_count)
        history.pop_front();
    value.SetHistory(history);
    for (auto& p : value.plots)
    {
        p.AddValue(v);
    }
}

std::string Collector::ExecCommand(const std::string cmd, int& out_exitStatus)
{
    out_exitStatus = 0;
    auto pPipe = ::popen(cmd.c_str(), "r");
    if (pPipe == nullptr)
    {
        throw std::runtime_error("Cannot open pipe");
    }

    std::array<char, 256> buffer;

    std::string result;

    while (not std::feof(pPipe))
    {
        auto bytes = std::fread(buffer.data(), 1, buffer.size(), pPipe);
        result.append(buffer.data(), bytes);
    }

    auto rc = ::pclose(pPipe);

    if (WIFEXITED(rc))
    {
        out_exitStatus = WEXITSTATUS(rc);
    }

    return result;
}

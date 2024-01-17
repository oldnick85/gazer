#include "config.h"

#include "global.h"

#include "nlohmann/json.hpp"

#include <fstream>

Configuration::Configuration(const std::string &path)
{
    Parse(path);
}

bool Configuration::Parse(const std::string &path)
{
    m_valid = false;
    std::ifstream file(path);
    if(file.is_open() == false) 
    {
        spdlog::error("[{}] File {} open error!", TAG, path);
        return false;
    }
    try
    {
        auto j = nlohmann::json::parse(file);
        if (j.contains("http_host"))
        {
            const auto http_host = j.at("http_host");
            m_config.http_host.ip = http_host.value("ip", "localhost");
            m_config.http_host.port = http_host.value("port", 16699);
        }
        if (j.contains("values"))
        {
            const auto values = j.at("values");
            for (const auto& v : values)
            {
                sConfig::sValue value;
                value.name = v.value("name", "");
                value.bash_command = v.value("bash_command", "");
                value.output_regex = v.value("output_regex", "");
                value.collect_delay_sec = v.value("collect_delay_sec", 1);
                value.collect_count = v.value("collect_count", 1);
                if (v.contains("plots"))
                {
                    const auto plots = v.at("plots");
                    for (const auto& p : plots)
                    {
                        sConfig::sValue::sPlot plot;
                        plot.title = p.value("title", "");
                        plot.type = p.value("type", "raw");
                        plot.chunk_size = p.value("chunk_size", 0);
                        plot.chunks_count = p.value("chunks_count", 0);
                        value.plots.push_back(plot);
                    }
                }
                m_config.values.push_back(value);
            }
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("[{}] File {} parse error!", TAG, path);
        file.close();
        return false;
    }
    spdlog::debug("[{}] File {} parse OK!", TAG, path);
    file.close();
    m_valid = true;
    return true;
}

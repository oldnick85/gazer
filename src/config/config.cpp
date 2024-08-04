#include "config.h"

#include <fstream>

#include "nlohmann/json.hpp"

#include "global.h"

Configuration::Configuration(const std::string& path)
{
    Parse(path);
}

bool Configuration::Parse(const std::string& path)
{
    spdlog::error("!");
    m_valid = false;
    std::ifstream file(path);
    if (file.is_open() == false)
    {
        spdlog::error("[{}] File {} open error!", TAG, path);
        return false;
    }

    auto jroot = nlohmann::json::parse(file, nullptr, false);
    if (jroot.is_discarded())
    {
        spdlog::error("[{}] File {} parse error!", TAG, path);
        file.close();
        return false;
    }

    if (jroot.contains("http_host"))
    {
        const auto jhttp_host       = jroot.at("http_host");
        m_config.http_host.ip       = jhttp_host.value("ip", "localhost");
        constexpr uint default_port = 16699;
        m_config.http_host.port     = jhttp_host.value("port", default_port);
    }
    if (jroot.contains("values"))
    {
        const auto jvalues = jroot.at("values");
        for (const auto& jvalue : jvalues)
        {
            sConfig::sValue value;
            value.name              = jvalue.value("name", "");
            value.bash_command      = jvalue.value("bash_command", "");
            value.output_regex      = jvalue.value("output_regex", "");
            value.collect_delay_sec = jvalue.value("collect_delay_sec", 1);
            value.collect_count     = jvalue.value("collect_count", 1);
            if (jvalue.contains("plots"))
            {
                const auto& jplots = jvalue.at("plots");
                for (const auto& jplot : jplots)
                {
                    sConfig::sValue::sPlot plot;
                    plot.title        = jplot.value("title", "");
                    plot.type         = jplot.value("type", "raw");
                    plot.chunk_size   = jplot.value("chunk_size", 0);
                    plot.chunks_count = jplot.value("chunks_count", 0);
                    value.plots.push_back(plot);
                }
            }
            m_config.values.push_back(value);
        }
    }

    spdlog::debug("[{}] File {} parse OK!", TAG, path);
    file.close();
    m_valid = true;
    return true;
}

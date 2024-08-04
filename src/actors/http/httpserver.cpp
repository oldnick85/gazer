#include "httpserver.h"

#include "global.h"
#include "config/config.h"

Server::Server(DataPtr& data) : m_data(data)
{
    m_ip   = config()->Get().http_host.ip;
    m_port = config()->Get().http_host.port;
    spdlog::info("[{}] {} {}:{}", TAG, __FUNCTION__, m_ip, m_port);
}

Server::~Server()
{
    Stop();
}

std::thread Server::RunInAnotherThread()
{
    auto server_thread = std::thread{&Server::Run, this};
    return server_thread;
}

void Server::Stop()
{
    m_svr.stop();
}

void Server::BindHandlers()
{
    handler_get("/");
    handler_get("/stat");
    handler_get("/stat/");
}

void Server::handler_get(const std::string& uri)
{
    m_svr.Get(uri, [this, uri](const httplib::Request& req, httplib::Response& res) {
        spdlog::debug("[{}] GET on {}", TAG, uri);
        spdlog::trace("[{}] req:{}", TAG, req.body);

        std::string respbody;
        respbody += "<!doctype html>\n";
        respbody += "<html><body>\n";

        const auto& config_values = config()->Get().values;
        for (size_t i = 0; i < config_values.size(); ++i)
        {
            auto& value = m_data->values[i];
            respbody += "<hr>\n<h2>" + value.name + "</h2>\n";

            if (value.plots.empty())
            {
                respbody += PlotPseudoRaw(value);
                continue;
            }

            for (const auto& plot : value.plots)
            {
                if (plot.Type() == "raw")
                {
                    respbody += PlotPseudoRaw(value);
                    continue;
                }
                respbody += "<h3>" + plot.Title() + "</h3>\n";
                const auto& chunks = plot.GetChunks();
                std::vector<std::chrono::time_point<std::chrono::system_clock>> plot_times;
                std::for_each(chunks.begin(), chunks.end(),
                              [&plot_times](const Chunk& chunk) { plot_times.push_back(chunk.Time()); });
                {
                    std::vector<float> plot_values;
                    std::for_each(chunks.begin(), chunks.end(),
                                  [&plot_values](const Chunk& chunk) { plot_values.push_back(chunk.Max()); });
                    respbody += PlotPseudo(plot_values, plot_times, "<b>MAX</b>");
                }
                {
                    std::vector<float> plot_values;
                    std::for_each(chunks.begin(), chunks.end(),
                                  [&plot_values](const Chunk& chunk) { plot_values.push_back(chunk.Mean()); });
                    respbody += PlotPseudo(plot_values, plot_times, "<b>MEAN</b>");
                }
                {
                    std::vector<float> plot_values;
                    std::for_each(chunks.begin(), chunks.end(),
                                  [&plot_values](const Chunk& chunk) { plot_values.push_back(chunk.Min()); });
                    respbody += PlotPseudo(plot_values, plot_times, "<b>MIN</b>");
                }
            }
        }

        respbody += "</body></html>";
        res.set_content(respbody, "text/html");
        spdlog::debug("[{}] response size {}", TAG, respbody.size());
        spdlog::trace("[{}] resp:{}", TAG, respbody);
    });
}

std::string Server::PlotPseudoRaw(const Value& value)
{
    const auto history = value.CopyHistory();
    if (history.empty())
        return "";
    std::vector<float> plot_values;
    std::for_each(history.begin(), history.end(),
                  [&plot_values](const sHistorySample& sample) { plot_values.push_back(sample.value); });
    std::vector<std::chrono::time_point<std::chrono::system_clock>> plot_times;
    std::for_each(history.begin(), history.end(),
                  [&plot_times](const sHistorySample& sample) { plot_times.push_back(sample.system_time); });
    return PlotPseudo(plot_values, plot_times, "");
}

std::string Server::PlotPseudo(const std::vector<float>& values,
                               std::vector<std::chrono::time_point<std::chrono::system_clock>> times,
                               const std::string& caption)
{
    std::string str;
    const auto [min_e, max_e] = std::minmax_element(values.begin(), values.end());
    const auto min            = *min_e;
    const auto max            = *max_e;
    const auto delta          = (max == min) ? 1.0 : max - min;
    str += "<p>";
    str += caption + "<br>\n";
    str += "max=" + std::to_string(max) + "<br>\n";
    auto time_it = times.begin();
    for (const auto value : values)
    {
        constexpr float y_values_count = 7.0;
        uint y_val                     = static_cast<uint>(round((value - min) * y_values_count / delta) + 1);
        const auto in_time_t           = std::chrono::system_clock::to_time_t(*time_it);
        std::stringstream sstr;
        sstr << std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%S%z");
        str += "<abbr title=\"" + sstr.str() + " : " + std::to_string(value) + "\">";
        str += "&#x258" + std::to_string(y_val) + ";";
        str += "</abbr>";
        time_it++;
    }
    str += "<br>\nmin=" + std::to_string(min);
    str += "</p>\n";
    return str;
}

void Server::Run()
{
    pthread_setname_np(pthread_self(), "PA_HTTP");
    BindHandlers();
    spdlog::info("[{}] Start http server {}:{}", TAG, m_ip, m_port);
    const auto listen_result = m_svr.listen(m_ip, m_port);
    spdlog::info("[{}] listen http server {}", TAG, (listen_result ? "OK" : "ERROR"));
}

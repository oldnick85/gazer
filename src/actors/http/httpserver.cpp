#include "httpserver.h"

#include "global.h"
#include "config/config.h"

Server::Server(DataPtr data)
    : m_data(data)
{
    m_ip = config()->Get().http_host.ip;
    m_port = config()->Get().http_host.port;
    spdlog::info("[{}] {} {}:{}", TAG, __FUNCTION__, m_ip, m_port);
}

Server::~Server()
{
    Stop();
}

std::thread Server::RunInAnotherThread()
{
    auto t = std::thread{&Server::Run, this};
    return std::move(t);
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

void 
Server::handler_get(const std::string &uri) 
{
    m_svr.Get(uri, [this, uri](const httplib::Request &req, httplib::Response &res) {
        
        spdlog::debug("[{}] GET on {}", TAG, uri);
        spdlog::trace("[{}] req:{}", TAG, req.body);

        std::string respbody;
        respbody += "<!doctype html>\n";
        respbody += "<html><body>\n";
        
        const auto& config_values = config()->Get().values;
        for (size_t i = 0; i < config_values.size(); ++i)
        {
            auto& config_value = config_values[i];
            auto& value = m_data->values[i];
            respbody += "<hr>\n<h2>" + value.name + "</h2>\n";

            if (value.plots.empty())
            {
                respbody += PlotPseudoRaw(value);
                continue;
            }
            
            for (const auto& p : value.plots)
            {
                if (p.Type() == "raw")
                {
                    respbody += PlotPseudoRaw(value);
                    continue;
                }
                respbody += "<h3>" + p.Title() + "</h3>\n";
                const auto& chunks = p.GetChunks();
                std::vector<std::chrono::time_point<std::chrono::system_clock>> plot_times;
                std::for_each(chunks.begin(), chunks.end(), [&plot_times](const Chunk& c) { plot_times.push_back(c.Time()); });
                {
                    std::vector<float> plot_values;
                    std::for_each(chunks.begin(), chunks.end(), [&plot_values](const Chunk& c) { plot_values.push_back(c.Max()); });
                    respbody += PlotPseudo(plot_values, plot_times, "<b>MAX</b>");
                }
                {
                    std::vector<float> plot_values;
                    std::for_each(chunks.begin(), chunks.end(), [&plot_values](const Chunk& c) { plot_values.push_back(c.Mean()); });
                    respbody += PlotPseudo(plot_values, plot_times, "<b>MEAN</b>");
                }
                {
                    std::vector<float> plot_values;
                    std::for_each(chunks.begin(), chunks.end(), [&plot_values](const Chunk& c) { plot_values.push_back(c.Min()); });
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
    std::for_each(history.begin(), history.end(), [&plot_values](const sHistorySample& s) { plot_values.push_back(s.value); });
    std::vector<std::chrono::time_point<std::chrono::system_clock>> plot_times;
    std::for_each(history.begin(), history.end(), [&plot_times](const sHistorySample& s) { plot_times.push_back(s.system_time); });
    return PlotPseudo(plot_values, plot_times, "");
}

std::string Server::PlotPseudo(const std::vector<float>& values, std::vector<std::chrono::time_point<std::chrono::system_clock>> times, const std::string caption)
{
    std::string s = "";
    const auto [min_e, max_e] = std::minmax_element(values.begin(), values.end());
    const auto min = *min_e;
    const auto max = *max_e;
    const auto d = (max == min) ? 1.0 : max - min;
    s += "<p>";
    s += caption + "<br>\n";
    s += "max=" + std::to_string(max) + "<br>\n";
    auto time_it = times.begin();
    for (const auto v : values)
    {
        uint n = round((v - min)*7/d) + 1;
        const auto in_time_t = std::chrono::system_clock::to_time_t(*time_it);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%S%z");
        s += "<abbr title=\"" + ss.str() + " : " + std::to_string(v) + "\">";
        s += "&#x258" + std::to_string(n) + ";";
        s += "</abbr>";
        time_it++;
    }
    s += "<br>\nmin=" + std::to_string(min);
    s += "</p>\n";
    return s;
}

void Server::Run()
{
    pthread_setname_np(pthread_self(), "PA_HTTP");
    BindHandlers();
    spdlog::info("[{}] Start http server {}:{}", TAG, m_ip, m_port);
    const auto listen_result = m_svr.listen(m_ip, m_port);
    spdlog::info("[{}] listen http server {}", TAG, (listen_result ? "OK" : "ERROR"));
}

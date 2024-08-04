#pragma once

#include <string>

#include "httplib.h"
#include "spdlog/spdlog.h"

#include "actors/data/data.h"

class Server
{
    static inline const std::string TAG = "HTTPSRV";

  public:
    static inline const std::string uri_call_register = "/call/registry";
    static inline const std::string uri_call_verify   = "/call/verifier";
    static inline const std::string uri_state         = "/state";
    static inline const std::string uri_overall       = "/overall";
    static inline const std::string uri_statistic     = "/statistic";

    Server(DataPtr& data);
    ~Server();

    std::thread RunInAnotherThread();
    void Run();
    void Stop();

  private:
    void BindHandlers();
    static std::string PlotPseudoRaw(const Value& value);
    static std::string PlotPseudo(const std::vector<float>& values,
                                  std::vector<std::chrono::time_point<std::chrono::system_clock>> times,
                                  const std::string& caption);

    // handlers
    void handler_get(const std::string& uri);

  private:
    DataPtr m_data;
    httplib::Server m_svr;
    std::string m_ip;
    uint16_t m_port = 0;
};
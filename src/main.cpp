#include <memory>

#include "global.h"
#include "application.h"
#include "config.h"

std::shared_ptr<CApplication> g_app     = nullptr;
std::shared_ptr<Configuration> g_config = nullptr;

void init_logger()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    auto logger       = std::make_shared<spdlog::logger>("gazer", console_sink);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    constexpr auto log_flush_period = std::chrono::milliseconds(100);
    spdlog::flush_every(log_flush_period);
}

int main(int argc, char **argv)
{
    init_logger();

    if (argc < 2)
    {
        spdlog::error("Must be at least 1 argument");
        std::exit(EXIT_FAILURE);
    }

    std::string config_path(argv[1]);

    g_config = std::make_shared<Configuration>(config_path);
    if (not g_config->Valid())
    {
        spdlog::error("Config parse error");
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("==== START GAZER ====");

    g_app = std::make_shared<CApplication>();
    if (not g_app->Run())
    {
        spdlog::error("App run error");
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}

#include "global.h"
#include "application.h"

extern std::shared_ptr<CApplication> g_app;
extern std::shared_ptr<Configuration> g_config;

std::shared_ptr<CApplication> app()
{
    return g_app;
}
std::shared_ptr<Configuration> config()
{
    return g_config;
}
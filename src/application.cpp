#include <execinfo.h>
#include "application.h"

#include "global.h"

void signal_handler(int signal)
{
    fprintf(stderr, "Error: signal %d:\n", signal);
    app()->Stop();
    constexpr uint wait_for_stop_usec = 1000000;
    usleep(wait_for_stop_usec);
}

CApplication::CApplication()
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

CApplication::~CApplication()
{
    Stop();
}

bool CApplication::Run()
{
    m_data      = std::make_shared<Data>();
    m_collector = std::make_unique<Collector>(m_data);
    m_http      = std::make_unique<Server>(m_data);

    std::thread collector_thread = m_collector->Run();
    m_http->Run();

    collector_thread.join();
    return true;
}

void CApplication::Stop()
{
    if (m_stop)
        return;

    m_stop = true;

    if (m_http)
        m_http->Stop();

    if (m_collector)
        m_collector->Stop();
}

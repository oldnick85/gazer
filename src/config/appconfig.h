#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct sConfig
{
    struct sHost
    {
        std::string ip;
        uint16_t    port;
    };
    sHost http_host;                         ///< ip:port on which Http server started

    struct sValue
    {
        std::string name;
        std::string bash_command;
        std::string output_regex;
        uint collect_delay_sec = 1;
        uint collect_count = 1;

        struct sPlot
        {
            std::string title;
            std::string type;
            uint        chunk_size = 0;
            uint        chunks_count = 0;  
        };
        
        std::vector<sPlot>  plots;
    };
    
    std::vector<sValue> values;
};
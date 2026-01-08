#pragma once

#include <cstring>
#include <string>
#include <fstream> 
#include <unordered_map>

#define CONFIG_FILE_PARSE_ERR 1

const std::string CONFIG_FILE{"config.yaml"};
const std::string YAML_ERROR_START{"yaml: syntax error on line \'"};
const std::string MISSING_COLON{"missing colon"};

struct Config {
    unsigned int port = 8080;
    unsigned int max_conn = 10;
    std::string host = "0.0.0.0";
    std::string parent_dir = "./";
    std::unordered_map<std::string, std::string> static_html_mappings{};

    Config()
    {
        // Config will search for the config.yaml file
        std::ifstream config_file{CONFIG_FILE};
        
        if (!config_file.is_open()) {
            printf("WARNING: config.yaml not found\n");
            return;
        }
        
        config_file.seekg(0, std::ios::end);
        size_t size = config_file.tellg();
        config_file.seekg(0);
        
        if (size == 0) {
            printf("WARNING: config.yaml is empty\n");
            return;
        }

        std::string configuration(size + 1, '\0');
        config_file.read(configuration.data(), size);
        configuration[size] = '\n';

        // Now begin reading the configuration
        if (!read_config(configuration))
            exit(CONFIG_FILE_PARSE_ERR);
    }

    ~Config() = default;

private:
    bool validate_token(const std::string& token) // validate key and value
    {
        if (memcmp("port", token.data(), token.size()) == 0);
    }

    bool read_config(const std::string& configuration)
    {
        size_t start = 0, end = configuration.find('\n');
        
        while (start < end) {
            std::string temp = configuration.substr(start, end - start);

            if (temp.find(':') == std::string::npos) {
                std::string error = YAML_ERROR_START + temp + '\'' + MISSING_COLON;
                perror(error.data());
                return false;
            }

        }

        return true;
    }
};

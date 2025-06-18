#include "../include/WebServ.hpp"

int main(int argc, char **argv) 
{
    if (argc == 1 || argc == 2) 
    {
        try
        {
            std::string config_path;

            if (argc == 1) {
                config_path = "config/default.config";
            } else {
                config_path = argv[1];
            }

            ReadConfig config_reader;

            config_reader.createServerGroup(config_path);

            std::vector<ServerSetUp> servers = config_reader.getServers();

            ServerManager manager;
            manager.setup(servers);
            manager.init();
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return (ERROR);
        }
    }
    else 
    {
        std::cerr << USAGE << std::endl;
        return (ERROR);
    }
    return (SUCCESS);
}

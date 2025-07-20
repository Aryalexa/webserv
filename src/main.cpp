#include "../include/WebServ.hpp"

int main(int argc, char **argv) 
{
    std::string config_path;
    ReadConfig config_reader;
    std::vector<ServerSetUp> serverGroup;
    ServerManager serverManager;

    if (argc > 2) {
        std::cerr << USAGE << std::endl;
        return (ERROR);
    }
    
    try {
        if (argc == 1)
            config_path = DEFAULT_CONFIG_FILE;
        else
            config_path = argv[1];
        
        config_reader.createServerGroup(config_path);

        serverGroup = config_reader.getServers();

        serverManager.setup(serverGroup);
        serverManager.init();

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return (ERROR);
    }
    
    return (SUCCESS);
}

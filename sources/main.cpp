#include "../headers/WebServ.hpp"

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

			// Esta parte es de Debug, no le hagan caso
            // config_reader.print_debug_parser();

            std::vector<ServerParser> servers = config_reader.getServers();

			// Esta parte también es de Debug, no le hagan caso

			/*
			for (size_t i = 0; i < servers.size(); ++i)
                servers[i].setUpServer();

			ServerParser &server = servers[0];

            std::cout << "Server listening on port: " << server.getPort() << std::endl;
			*/

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

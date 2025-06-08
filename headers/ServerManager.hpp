#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../headers/WebServ.hpp"

# define CONNECTION_TIMEOUT 5

class ServerParser;

class ServerManager {
    public:
        ServerManager();
        ~ServerManager();

        void setUpServers(const std::vector<ServerParser>& servers);
        void initializeSets();

    private:
        std::vector<ServerParser>           _servers;
        std::map<int,ServerParser>          _servers_map;
        fd_set                              _recv_fd_pool;
        fd_set                              _write_fd_pool;
        int                                 _biggest_fd;
};

#endif

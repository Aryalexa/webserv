#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "../headers/WebServ.hpp"

# define CONNECTION_TIMEOUT 5

class ServerSetUp;

class ServerManager {
    public:
        ServerManager();
        ~ServerManager();

        void setUpMultipleServers(const std::vector<ServerSetUp>& servers);
        void initializeSockets();

    private:
        std::vector<ServerSetUp>           _servers;
        std::map<int,ServerSetUp>          _servers_map;
        fd_set                              _recv_fd_pool;
        fd_set                              _write_fd_pool;
        int                                 _biggest_fd;
};

#endif

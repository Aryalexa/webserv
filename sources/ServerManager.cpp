#include "../headers/WebServ.hpp"

ServerManager::ServerManager()
  : _biggest_fd(0)
{
}

ServerManager::~ServerManager(){}

void ServerManager::setUpMultipleServers(const std::vector<ServerSetUp>& configs) {

    _servers = configs;
    
    for (size_t i = 0; i < _servers.size(); ++i) 
    {
        ServerSetUp &server = _servers[i];
        bool reused = false;

        for (size_t j = 0; j < i; ++j) {
            const ServerSetUp &prev = _servers[j];
            if (prev.getHost() == server.getHost() &&
                prev.getPort() == server.getPort())
            {
                server.setFd(prev.getFd());
                reused = true;
                break;
            }
        }

        if (!reused) {
            server.setUpIndividualServer();
        }

        _servers_map[server.getFd()] = server;

        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &server.getHost(), ipbuf, sizeof(ipbuf));
        Message::logMessage(
            "Server %s bound to %s: %d, fd = %d",
            server.getServerName().c_str(),
            ipbuf,
            server.getPort(),
            server.getFd()
        );
    }
}

void ServerManager::initializeSockets()
{

    FD_ZERO(&_recv_fd_pool);
    FD_ZERO(&_write_fd_pool);

    for (size_t i = 0; i < _servers.size(); ++i) 
    {
        int fd = _servers[i].getFd();

        if (listen(fd, 512) < 0) {
            Message::logMessage("listen(%d) failed: %s", fd, strerror(errno));
            exit(ERROR);
        }

        if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
            Message::logMessage("fcntl(%d, O_NONBLOCK) failed: %s", fd, strerror(errno));
            exit(ERROR);
        }

        FD_SET(fd, &_recv_fd_pool);

        if (fd > _biggest_fd)
            _biggest_fd = fd;
    }

    Message::logMessage("Initialized FDs: biggest_fd = %d", _biggest_fd);
}

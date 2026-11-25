#pragma once

#include "../includes.h"
#include "Client.h"

class Server{
	public :

		explicit Server(const std::vector<ServerConfig>& configs); // explicit cause 1 arg, making servers eggspensive
		~Server();
		Server(const Server&) = delete;
		Server& operator=(const Server&) = delete;
		void run();     // blocks until stop() is called (e.g. from a SIGINT handler)
		void stop();
	
	private : 
		const std::vector<ServerConfig>&                     m_Configs;
		int                                                  m_EpollFd;
		bool                                                 m_Running;

		// listening fd -> server blocks that listen on that host:port
		std::map<int, std::vector<const ServerConfig*>>      m_Listeners;
		// client fd -> client
		std::map<int, std::unique_ptr<Client>>               m_Clients;

		void    setupListeners();
		int     createListenSocket(const ListenAddress& addr);
		void    acceptClient(int listenFd);
		void    handleClientEvent(int fd, uint32_t events);
		void    closeClient(int fd);
		void    checkTimeouts();

		void    epollAdd(int fd, uint32_t events);
		void    epollMod(int fd, uint32_t events);
};

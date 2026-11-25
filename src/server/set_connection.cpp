#include "Server.h"

// ---------- construction ----------

Server::Server(const std::vector<ServerConfig>& configs)
	: m_Configs(configs), m_EpollFd(-1), m_Running(false)
{
	m_EpollFd = epoll_create(1);
	if (m_EpollFd == -1)
		throw std::runtime_error("epoll_create failed");
	setupListeners();
}

Server::~Server()
{
	for (auto& [fd, client] : m_Clients)
		close(fd);
	for (auto& [fd, configs] : m_Listeners)
		close(fd);
	if (m_EpollFd != -1)
		close(m_EpollFd);
}
// ---------- helpers ----------

static void setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl(O_NONBLOCK) failed");
	// So the fd isn't inherited by CGI children after fork()/execve()
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
		throw std::runtime_error("fcntl(FD_CLOEXEC) failed");
}

static std::string addrKey(const ListenAddress& a)
{
	return a.Host + ":" + std::to_string(a.Port);
}


// One socket per UNIQUE host:port. Several server blocks may share it.
void Server::setupListeners()
{
	std::map<std::string, int> opened;   // "host:port" -> listening fd
	
	for (const ServerConfig& cfg : m_Configs)
	{
		for (const ListenAddress& addr : cfg.Listens)
		{
			std::string key = addrKey(addr);
			auto it = opened.find(key);
			if (it == opened.end())
			{
				int fd = createListenSocket(addr);
				opened[key] = fd;
				m_Listeners[fd].push_back(&cfg);
				std::cout << "Listening on http://" << key << std::endl;
			}
			else
			{
				// Same address used by another server block: share the socket,
				// but don't add the same block twice (e.g. a duplicated listen line)
				auto& list = m_Listeners[it->second];
				if (list.back() != &cfg)
					list.push_back(&cfg);
			}
		}
	}
	if (m_Listeners.empty())
		throw std::runtime_error("no listen addresses in config");
}

int Server::createListenSocket(const ListenAddress& addr)
{
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* res = nullptr;
	std::string port = std::to_string(addr.Port);
	if (getaddrinfo(addr.Host.c_str(), port.c_str(), &hints, &res) != 0)
		throw std::runtime_error("invalid listen address: " + addrKey(addr));

	int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd == -1)
	{
		freeaddrinfo(res);
		throw std::runtime_error("socket() failed");
	}

	int yes = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	if (bind(fd, res->ai_addr, res->ai_addrlen) == -1)
	{
		freeaddrinfo(res);
		close(fd);
		throw std::runtime_error("bind() failed on " + addrKey(addr)
			+ " (port in use, or overlaps with a 0.0.0.0 listen?)");
	}
	freeaddrinfo(res);

	if (listen(fd, SOMAXCONN) == -1)
	{
		close(fd);
		throw std::runtime_error("listen() failed on " + addrKey(addr));
	}
	setNonBlocking(fd);
	epollAdd(fd, EPOLLIN);
	return fd;
}

// ---------- main loop ----------

void Server::run()
{
	struct epoll_event events[MAX_EVENTS];
	m_Running = true;

	while (m_Running)
	{
		// 1s timeout so checkTimeouts() runs even when nothing happens
		int n = epoll_wait(m_EpollFd, events, MAX_EVENTS, 1000);

		for (int i = 0; i < n; ++i)
		{
			int fd = events[i].data.fd;
			if (m_Listeners.count(fd))
				acceptClient(fd);
			else
				handleClientEvent(fd, events[i].events);
		}
		checkTimeouts();
	}
	std::cout << "\nServer stopped." << std::endl;
}

void Server::stop()
{
	m_Running = false;
}

void Server::acceptClient(int listenFd)
{
	int fd = accept(listenFd, nullptr, nullptr);
	if (fd == -1)
		return;   // nothing to accept after all; don't check errno, just move on

	try {
		setNonBlocking(fd);
		epollAdd(fd, EPOLLIN);
	} catch (const std::exception&) {
		close(fd);
		return;
	}
	m_Clients[fd] = std::make_unique<Client>(fd, m_Listeners[listenFd]);
}

void Server::handleClientEvent(int fd, uint32_t events)
{
	auto it = m_Clients.find(fd);
	if (it == m_Clients.end())
		return;   // already closed earlier in this batch of events
	Client& client = *it->second;

	if (events & (EPOLLERR | EPOLLHUP))
		return closeClient(fd);

	if ((events & EPOLLIN) && client.getState() == Client::READING)
	{
		if (!client.onReadable())
			return closeClient(fd);
		if (client.getState() == Client::WRITING)
			epollMod(fd, EPOLLOUT);   // request complete: switch to writing
	}
	else if ((events & EPOLLOUT) && client.getState() == Client::WRITING)
	{
		if (!client.onWritable())
			return closeClient(fd);
	}

	if (client.getState() == Client::DONE)
		closeClient(fd);
}

void Server::closeClient(int fd)
{
	epoll_ctl(m_EpollFd, EPOLL_CTL_DEL, fd, nullptr);
	close(fd);
	m_Clients.erase(fd);
}

void Server::checkTimeouts()
{
	time_t now = std::time(nullptr);
	std::vector<int> toClose;

	for (auto& [fd, client] : m_Clients)
	{
		if (now - client->getLastActivity() < CLIENT_TIMEOUT)
			continue;
		if (client->getState() == Client::READING)
		{
			// Client went quiet mid-request: answer 408, then close once it's sent.
			// (Swap this body for your ErrorPage code later.)
			std::string body = "<html><body><h1>408 Request Timeout</h1></body></html>\n";
			client->setResponse(
				"HTTP/1.0 408 Request Timeout\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: " + std::to_string(body.size()) + "\r\n"
				"\r\n" + body);
			epollMod(fd, EPOLLOUT);
		}
		else
			toClose.push_back(fd);   // stuck while writing: just drop it
	}
	for (int fd : toClose)
		closeClient(fd);
}

// ---------- epoll wrappers ----------

void Server::epollAdd(int fd, uint32_t events)
{
	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
		throw std::runtime_error("epoll_ctl(ADD) failed");
}

void Server::epollMod(int fd, uint32_t events)
{
	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.fd = fd;
	epoll_ctl(m_EpollFd, EPOLL_CTL_MOD, fd, &ev);
}

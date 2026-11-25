#pragma once

#include "../includes.h"

class Client
{
public:
	enum State
	{
		READING,  // waiting for (more of) the request
		WRITING,  // sending the response
		DONE      // response fully sent, connection can be closed
	};

	Client(int fd, const std::vector<const ServerConfig*>& configs);

	int                     getFd() const;
	State                   getState() const;
	time_t                  getLastActivity() const;
	const ServerConfig&     getConfig() const;

	// Called by the Server when epoll says the fd is ready.
	// Each does exactly ONE recv()/send(). Return false if the client must be dropped.
	bool                    onReadable();
	bool                    onWritable();

	// Used for timeouts / errors: replace whatever we were doing with this response
	void                    setResponse(const std::string& response);

private:
	int                                 m_Fd;
	std::vector<const ServerConfig*>    m_Configs;   // server blocks for the socket that accepted us
	State                               m_State;
	time_t                              m_LastActivity;

	std::string                         m_ReadBuffer;
	std::string                         m_WriteBuffer;
	size_t                              m_BytesSent;

	void                                handleRequest();
};

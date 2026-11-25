#include "includes.h"

Client::Client(int fd, const std::vector<const ServerConfig*>& configs)
	: m_Fd(fd), m_Configs(configs), m_State(READING),
	  m_LastActivity(std::time(nullptr)), m_BytesSent(0)
{
}

int Client::getFd() const{ 
	return m_Fd; 
}

Client::State Client::getState() const{ 
	return m_State; 
}

time_t Client::getLastActivity() const{ 
	return m_LastActivity; 
}	

// No virtual hosts: the first server block for this address is the default one
const ServerConfig& Client::getConfig() const{ 
	return *m_Configs.front(); 
}

bool Client::onReadable(){
	char buf[8192];
	ssize_t n = recv(m_Fd, buf, sizeof(buf), 0);

	// 0 = client closed the connection, -1 = error.
	// The subject forbids checking errno here, so both mean "drop this client".
	if (n <= 0)
		return false;

	m_LastActivity = std::time(nullptr);
	m_ReadBuffer.append(buf, n);

	// TODO: replace this with your HTTP parser:
	//   parser.feed(buf, n);
	//   if (parser.isComplete()) handleRequest();
	//   if (parser.hasError())   setResponse(errorResponse(parser.errorCode()));
	if (m_ReadBuffer.find("\r\n\r\n") != std::string::npos)
		handleRequest();

	return true;
}

bool Client::onWritable()
{
	ssize_t n = send(m_Fd, m_WriteBuffer.data() + m_BytesSent,
	                 m_WriteBuffer.size() - m_BytesSent, MSG_NOSIGNAL);
	if (n <= 0)
		return false;

	m_LastActivity = std::time(nullptr);
	m_BytesSent += n;
	if (m_BytesSent == m_WriteBuffer.size())
		m_State = DONE;   // HTTP/1.0: close after the response
	return true;
}

void Client::setResponse(const std::string& response)
{
	m_WriteBuffer = response;
	m_BytesSent = 0;
	m_State = WRITING;
}

// Temporary: echoes the request back so you can see what arrived.
// This is where routing / GET / POST / DELETE / CGI will plug in.
void Client::handleRequest()
{
	const ServerConfig& cfg = getConfig();
	std::string body =
		"Hello from webserv!\n"
		"Server root: " + cfg.Root + "\n"
		"\n--- Request received ---\n" + m_ReadBuffer;

	setResponse(
		"HTTP/1.0 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: " + std::to_string(body.size()) + "\r\n"
		"\r\n" + body);
}

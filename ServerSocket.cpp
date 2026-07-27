#include "ServerSocket.hpp"

static std::string	socketSystemError(const std::string& callName)
{
	return (callName + " failed: " + std::strerror(errno));
}

AcceptedClient::AcceptedClient()
	: fd(-1), ip("")
{
}

AcceptedClient::AcceptedClient(int clientFd, const std::string& clientIp)
	: fd(clientFd), ip(clientIp)
{
}

ServerSocket::ServerSocket()
	: _fd(-1), _port(0)
{
}

ServerSocket::~ServerSocket()
{
	closeSocket();
}

void	ServerSocket::createListeningSocket(int port, int backlog)
{
	_port = port;
	_createSocket();
	try
	{
		_configureReuseAddr();
		setNonBlocking(_fd);
		_bindSocket();
		_startListening(backlog);
	}
	catch (...)
	{
		closeSocket();
		throw;
	}
}

bool	ServerSocket::acceptClient(AcceptedClient& out)
{
	struct sockaddr_in	clientAddr;
	socklen_t			clientLen;
	int					clientFd;
	std::string			clientIp;

	std::memset(&clientAddr, 0, sizeof(clientAddr));
	clientLen = sizeof(clientAddr);
	clientFd = accept(_fd, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientLen);
	if (clientFd == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (false);
		throw std::runtime_error(socketSystemError("accept"));
	}
	try
	{
		setNonBlocking(clientFd);
	}
	catch (...)
	{
		closeFd(clientFd);
		throw;
	}
	clientIp = inet_ntoa(clientAddr.sin_addr);
	out = AcceptedClient(clientFd, clientIp);
	return (true);
}

void	ServerSocket::closeSocket()
{
	if (_fd != -1)
	{
		close(_fd);
		_fd = -1;
	}
}

int	ServerSocket::getFd() const
{
	return (_fd);
}

int	ServerSocket::getPort() const
{
	return (_port);
}

bool	ServerSocket::isOpen() const
{
	return (_fd != -1);
}

void	ServerSocket::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error(socketSystemError("fcntl(F_SETFL, O_NONBLOCK)"));
}

void	ServerSocket::closeFd(int fd)
{
	if (fd != -1)
		close(fd);
}

void	ServerSocket::_createSocket()
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw std::runtime_error(socketSystemError("socket"));
}

void	ServerSocket::_configureReuseAddr()
{
	int	optval;

	optval = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		throw std::runtime_error(socketSystemError("setsockopt(SO_REUSEADDR)"));
}

void	ServerSocket::_bindSocket()
{
	struct sockaddr_in	addr;

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);
	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1)
		throw std::runtime_error(socketSystemError("bind"));
}

void	ServerSocket::_startListening(int backlog)
{
	if (listen(_fd, backlog) == -1)
		throw std::runtime_error(socketSystemError("listen"));
}

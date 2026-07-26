#include "AyoubSocket.hpp"

static std::string	ayoubSystemError(const std::string& callName)
{
	return (callName + " failed: " + std::strerror(errno));
}

AyoubAcceptedClient::AyoubAcceptedClient()
	: fd(-1), ip("")
{
}

AyoubAcceptedClient::AyoubAcceptedClient(int clientFd, const std::string& clientIp)
	: fd(clientFd), ip(clientIp)
{
}

AyoubSocket::AyoubSocket()
	: _fd(-1), _port(0)
{
}

AyoubSocket::~AyoubSocket()
{
	closeSocket();
}

void	AyoubSocket::createListeningSocket(int port, int backlog)
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

bool	AyoubSocket::acceptClient(AyoubAcceptedClient& out)
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
		throw std::runtime_error(ayoubSystemError("accept"));
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
	out = AyoubAcceptedClient(clientFd, clientIp);
	return (true);
}

void	AyoubSocket::closeSocket()
{
	if (_fd != -1)
	{
		close(_fd);
		_fd = -1;
	}
}

int	AyoubSocket::getFd() const
{
	return (_fd);
}

int	AyoubSocket::getPort() const
{
	return (_port);
}

bool	AyoubSocket::isOpen() const
{
	return (_fd != -1);
}

void	AyoubSocket::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error(ayoubSystemError("fcntl(F_SETFL, O_NONBLOCK)"));
}

void	AyoubSocket::closeFd(int fd)
{
	if (fd != -1)
		close(fd);
}

void	AyoubSocket::_createSocket()
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw std::runtime_error(ayoubSystemError("socket"));
}

void	AyoubSocket::_configureReuseAddr()
{
	int	optval;

	optval = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		throw std::runtime_error(ayoubSystemError("setsockopt(SO_REUSEADDR)"));
}

void	AyoubSocket::_bindSocket()
{
	struct sockaddr_in	addr;

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);
	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1)
		throw std::runtime_error(ayoubSystemError("bind"));
}

void	AyoubSocket::_startListening(int backlog)
{
	if (listen(_fd, backlog) == -1)
		throw std::runtime_error(ayoubSystemError("listen"));
}

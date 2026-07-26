#include "AyoubIo.hpp"

static std::string	ayoubIoError(const std::string& callName)
{
	return (callName + " failed: " + std::strerror(errno));
}

ssize_t	AyoubIo::recvChunk(int fd, std::string& chunk,
			bool& closed, bool& wouldBlock)
{
	char	buffer[4096];
	ssize_t	bytes;

	chunk.clear();
	closed = false;
	wouldBlock = false;
	bytes = recv(fd, buffer, sizeof(buffer), 0);
	if (bytes > 0)
	{
		chunk.assign(buffer, static_cast<size_t>(bytes));
		return (bytes);
	}
	if (bytes == 0)
	{
		closed = true;
		return (0);
	}
	if (errno == EAGAIN || errno == EWOULDBLOCK)
	{
		wouldBlock = true;
		return (0);
	}
	throw std::runtime_error(ayoubIoError("recv"));
}

ssize_t	AyoubIo::sendFromBuffer(int fd, std::string& outputBuffer,
			bool& wouldBlock)
{
	ssize_t	sent;

	wouldBlock = false;
	if (outputBuffer.empty())
		return (0);
	sent = send(fd, outputBuffer.c_str(), outputBuffer.size(), 0);
	if (sent > 0)
	{
		outputBuffer.erase(0, static_cast<size_t>(sent));
		return (sent);
	}
	if (sent == 0)
		return (0);
	if (errno == EAGAIN || errno == EWOULDBLOCK)
	{
		wouldBlock = true;
		return (0);
	}
	throw std::runtime_error(ayoubIoError("send"));
}

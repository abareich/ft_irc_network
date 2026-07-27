#ifndef NETWORK_IO_HPP
# define NETWORK_IO_HPP

# include <string>
# include <stdexcept>
# include <cstring>
# include <cerrno>
# include <sys/types.h>
# include <sys/socket.h>

class NetworkIo
{
	public:
		static ssize_t	recvChunk(int fd, std::string& chunk,
							bool& closed, bool& wouldBlock);
		static ssize_t	sendFromBuffer(int fd, std::string& outputBuffer,
							bool& wouldBlock);

	private:
		NetworkIo();
		NetworkIo(const NetworkIo& other);
		NetworkIo& operator=(const NetworkIo& other);
		~NetworkIo();
};

#endif

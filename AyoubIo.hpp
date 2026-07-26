#ifndef AYOUB_IO_HPP
# define AYOUB_IO_HPP

# include <string>
# include <stdexcept>
# include <cstring>
# include <cerrno>
# include <sys/types.h>
# include <sys/socket.h>

class AyoubIo
{
	public:
		static ssize_t	recvChunk(int fd, std::string& chunk,
							bool& closed, bool& wouldBlock);
		static ssize_t	sendFromBuffer(int fd, std::string& outputBuffer,
							bool& wouldBlock);

	private:
		AyoubIo();
		AyoubIo(const AyoubIo& other);
		AyoubIo& operator=(const AyoubIo& other);
		~AyoubIo();
};

#endif

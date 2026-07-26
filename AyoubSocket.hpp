#ifndef AYOUB_SOCKET_HPP
# define AYOUB_SOCKET_HPP

# include <string>
# include <stdexcept>
# include <cstring>
# include <cerrno>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <unistd.h>

struct AyoubAcceptedClient
{
	int			fd;
	std::string	ip;

	AyoubAcceptedClient();
	AyoubAcceptedClient(int clientFd, const std::string& clientIp);
};

class AyoubSocket
{
	public:
		AyoubSocket();
		~AyoubSocket();

		void	createListeningSocket(int port, int backlog);
		bool	acceptClient(AyoubAcceptedClient& out);
		void	closeSocket();

		int		getFd() const;
		int		getPort() const;
		bool	isOpen() const;

		static void	setNonBlocking(int fd);
		static void	closeFd(int fd);

	private:
		AyoubSocket(const AyoubSocket& other);
		AyoubSocket& operator=(const AyoubSocket& other);

		void	_createSocket();
		void	_configureReuseAddr();
		void	_bindSocket();
		void	_startListening(int backlog);

		int	_fd;
		int	_port;
};

#endif

#ifndef SERVER_SOCKET_HPP
# define SERVER_SOCKET_HPP

# include <string>
# include <stdexcept>
# include <cstring>
# include <cerrno>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <unistd.h>

struct AcceptedClient
{
	int			fd;
	std::string	ip;

	AcceptedClient();
	AcceptedClient(int clientFd, const std::string& clientIp);
};

class ServerSocket
{
	public:
		ServerSocket();
		~ServerSocket();

		void	createListeningSocket(int port, int backlog);
		bool	acceptClient(AcceptedClient& out);
		void	closeSocket();

		int		getFd() const;
		int		getPort() const;
		bool	isOpen() const;

		static void	setNonBlocking(int fd);
		static void	closeFd(int fd);

	private:
		ServerSocket(const ServerSocket& other);
		ServerSocket& operator=(const ServerSocket& other);

		void	_createSocket();
		void	_configureReuseAddr();
		void	_bindSocket();
		void	_startListening(int backlog);

		int	_fd;
		int	_port;
};

#endif

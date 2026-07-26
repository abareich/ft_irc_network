#ifndef AYOUB_POLL_MANAGER_HPP
# define AYOUB_POLL_MANAGER_HPP

# include <string>
# include <vector>
# include <stdexcept>
# include <cstring>
# include <cerrno>
# include <poll.h>

class AyoubPollManager
{
	public:
		typedef std::vector<struct pollfd> PollList;

		AyoubPollManager();
		~AyoubPollManager();

		void					addFd(int fd, short events);
		void					removeFd(int fd);
		void					setEvents(int fd, short events);
		void					enableWrite(int fd);
		void					disableWrite(int fd);
		bool					hasFd(int fd) const;

		int						wait(int timeoutMs);
		void					clearRevents();

		size_t					size() const;
		const struct pollfd&	at(size_t index) const;
		short					getRevents(int fd) const;

	private:
		AyoubPollManager(const AyoubPollManager& other);
		AyoubPollManager& operator=(const AyoubPollManager& other);

		int	_findIndex(int fd) const;

		PollList	_fds;
};

#endif

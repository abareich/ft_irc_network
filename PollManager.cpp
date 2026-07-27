#include "PollManager.hpp"

static std::string	pollSystemError(const std::string& callName)
{
	return (callName + " failed: " + std::strerror(errno));
}

PollManager::PollManager()
{
}

PollManager::~PollManager()
{
}

void	PollManager::addFd(int fd, short events)
{
	struct pollfd	pfd;

	if (hasFd(fd))
	{
		setEvents(fd, events);
		return ;
	}
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	_fds.push_back(pfd);
}

void	PollManager::removeFd(int fd)
{
	int	index;

	index = _findIndex(fd);
	if (index == -1)
		return ;
	_fds.erase(_fds.begin() + index);
}

void	PollManager::setEvents(int fd, short events)
{
	int	index;

	index = _findIndex(fd);
	if (index == -1)
		return ;
	_fds[index].events = events;
}

void	PollManager::enableWrite(int fd)
{
	int	index;

	index = _findIndex(fd);
	if (index == -1)
		return ;
	_fds[index].events = static_cast<short>(_fds[index].events | POLLOUT);
}

void	PollManager::disableWrite(int fd)
{
	int	index;

	index = _findIndex(fd);
	if (index == -1)
		return ;
	_fds[index].events = static_cast<short>(_fds[index].events & ~POLLOUT);
}

bool	PollManager::hasFd(int fd) const
{
	return (_findIndex(fd) != -1);
}

int	PollManager::wait(int timeoutMs)
{
	int	ready;

	if (_fds.empty())
		return (0);
	ready = poll(&_fds[0], static_cast<nfds_t>(_fds.size()), timeoutMs);
	if (ready == -1)
	{
		if (errno == EINTR)
			return (0);
		throw std::runtime_error(pollSystemError("poll"));
	}
	return (ready);
}

void	PollManager::clearRevents()
{
	for (size_t i = 0; i < _fds.size(); ++i)
		_fds[i].revents = 0;
}

size_t	PollManager::size() const
{
	return (_fds.size());
}

const struct pollfd&	PollManager::at(size_t index) const
{
	return (_fds.at(index));
}

short	PollManager::getRevents(int fd) const
{
	int	index;

	index = _findIndex(fd);
	if (index == -1)
		return (0);
	return (_fds[index].revents);
}

int	PollManager::_findIndex(int fd) const
{
	for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == fd)
			return (static_cast<int>(i));
	}
	return (-1);
}

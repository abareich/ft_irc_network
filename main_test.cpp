#include "AyoubSocket.hpp"
#include "AyoubPollManager.hpp"
#include "AyoubIo.hpp"
#include <iostream>
#include <map>
#include <cstdlib>
#include <csignal>

static bool g_running = true;

static void handleSignal(int sig)
{
	(void)sig;
	g_running = false;
	std::cout << "\n[SERVER] Signal de fermeture reçu. Arrêt du serveur..." << std::endl;
}

struct ClientContext
{
	int			fd;
	std::string	ip;
	std::string	inBuffer;
	std::string	outBuffer;
};

int main(int argc, char **argv)
{
	int port = 6667;

	if (argc >= 2)
		port = std::atoi(argv[1]);

	std::signal(SIGINT, handleSignal);
	std::signal(SIGTERM, handleSignal);

	std::cout << "==========================================" << std::endl;
	std::cout << "   TEST SERVEUR NETWORK LAYER (AYOUB)    " << std::endl;
	std::cout << "==========================================" << std::endl;
	std::cout << "[INFO] Port d'écoute : " << port << std::endl;

	AyoubSocket serverSocket;
	AyoubPollManager poller;
	std::map<int, ClientContext> clients;

	try
	{
		serverSocket.createListeningSocket(port, 10);
		poller.addFd(serverSocket.getFd(), POLLIN);
		std::cout << "[SUCCESS] Listening socket créé avec succès (FD: " 
		          << serverSocket.getFd() << ")" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[ERROR] Échec lors de la création du socket d'écoute : " 
		          << e.what() << std::endl;
		return (1);
	}

	std::cout << "[SERVER] En attente de connexions... (Appuyez sur Ctrl+C pour quitter)\n" << std::endl;

	while (g_running)
	{
		try
		{
			int readyCount = poller.wait(1000);
			if (readyCount <= 0)
				continue;

			for (size_t i = 0; i < poller.size(); ++i)
			{
				const struct pollfd& pfd = poller.at(i);
				int fd = pfd.fd;
				short revents = pfd.revents;

				if (revents == 0)
					continue;

				// 1. Événement sur le socket d'écoute (Nouveau Client)
				if (fd == serverSocket.getFd())
				{
					if (revents & POLLIN)
					{
						AyoubAcceptedClient newClient;
						if (serverSocket.acceptClient(newClient))
						{
							poller.addFd(newClient.fd, POLLIN);
							ClientContext ctx;
							ctx.fd = newClient.fd;
							ctx.ip = newClient.ip;
							clients[newClient.fd] = ctx;

							std::cout << "[+ CONNECT] Nouveau client connecté | FD: " 
							          << newClient.fd << " | IP: " << newClient.ip << std::endl;

							// Message de bienvenue IRC / Echo
							std::string welcome = ":server 001 Client :Bienvenue sur le serveur de test IRC (FD: ";
							welcome += static_cast<char>('0' + (newClient.fd % 10));
							welcome += ")\r\n";
							clients[newClient.fd].outBuffer += welcome;
							poller.enableWrite(newClient.fd);
						}
					}
				}
				// 2. Événement sur un socket Client
				else if (clients.count(fd) > 0)
				{
					ClientContext& client = clients[fd];

					// Lecture des données (POLLIN)
					if (revents & (POLLIN | POLLHUP | POLLERR))
					{
						std::string chunk;
						bool closed = false;
						bool wouldBlock = false;

						ssize_t bytesRead = AyoubIo::recvChunk(fd, chunk, closed, wouldBlock);
						if (bytesRead > 0)
						{
							std::cout << "[RECV] FD " << fd << " (" << client.ip << ") : " << chunk;
							// Echo des données reçues en retour au client
							client.outBuffer += "[ECHO] " + chunk;
							poller.enableWrite(fd);
						}

						if (closed)
						{
							std::cout << "[- DISCONNECT] Client déconnecté | FD: " << fd << std::endl;
							poller.removeFd(fd);
							AyoubSocket::closeFd(fd);
							clients.erase(fd);
							continue;
						}
					}

					// Écriture des données (POLLOUT)
					if (revents & POLLOUT)
					{
						if (!client.outBuffer.empty())
						{
							bool wouldBlock = false;
							ssize_t bytesSent = AyoubIo::sendFromBuffer(fd, client.outBuffer, wouldBlock);
							if (bytesSent > 0)
							{
								std::cout << "[SEND] Transmis " << bytesSent << " octets au FD " << fd << std::endl;
							}
						}

						if (client.outBuffer.empty())
						{
							poller.disableWrite(fd);
						}
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "[LOOP ERROR] Exception attrapée dans la boucle principale : " << e.what() << std::endl;
		}
	}

	// Nettoyage final
	std::cout << "\n[CLEANUP] Fermeture de tous les sockets..." << std::endl;
	for (std::map<int, ClientContext>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		poller.removeFd(it->first);
		AyoubSocket::closeFd(it->first);
	}
	clients.clear();
	serverSocket.closeSocket();
	std::cout << "[SERVER] Serveur arrêté proprement." << std::endl;

	return (0);
}

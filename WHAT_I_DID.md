# Network Layer Code Documentation

Ce dossier contient la couche réseau événementielle non-bloquante du projet IRC.

## Files

- **`NetworkIo.hpp`**
  Déclaration des helpers pour `recv()` et `send()`.

- **`NetworkIo.cpp`**
  Implémentation de la réception par chunks et de l'envoi depuis le buffer de sortie avec support des envois partiels.

- **`PollManager.hpp`**
  Déclaration de la classe de gestion du vecteur de `pollfd`.

- **`PollManager.cpp`**
  Implémentation de `addFd`, `removeFd`, `wait`, `enableWrite`, `disableWrite`.

- **`ServerSocket.hpp`**
  Déclaration de la classe créant le listening socket et acceptant les clients.

- **`ServerSocket.cpp`**
  Implémentation de `socket()`, `setsockopt()`, `fcntl()`, `bind()`, `listen()`, `accept()`, `close()`.

## Fonctionnalités couvertes

1. Création d'un TCP listening socket.
2. Configuration de `SO_REUSEADDR`.
3. Passage des sockets en mode non-bloquant avec `fcntl(fd, F_SETFL, O_NONBLOCK)`.
4. Liaison (`bind`) du socket à un port.
5. Démarrage de l'écoute (`listen()`).
6. Acceptation des nouveaux clients (`accept()`) et récupération de leur FD + IP.
7. Gestion des FDs avec `poll()`.
8. Activation/désactivation dynamique de `POLLOUT` selon la présence de données dans le buffer d'émission.
9. Lecture via `recv()` lors de la détection de `POLLIN`.
10. Émission via `send()` lors de la détection de `POLLOUT`.
11. Interception sécurisée de `EAGAIN` / `EWOULDBLOCK`.
12. Gestion des envois partiels avec rétention des octets restants dans le buffer.

## Utilisation

Au démarrage du serveur :

```cpp
ServerSocket socket;
PollManager poller;

socket.createListeningSocket(port, 10);
poller.addFd(socket.getFd(), POLLIN);
```

Lorsqu'un événement est détecté sur le socket serveur :

```cpp
AcceptedClient accepted;

if (socket.acceptClient(accepted))
    server.addClient(accepted.fd, accepted.ip);
```

Lorsqu'un événement `POLLIN` est détecté sur le socket d'un client :

```cpp
std::string chunk;
bool closed;
bool wouldBlock;

NetworkIo::recvChunk(fd, chunk, closed, wouldBlock);
```

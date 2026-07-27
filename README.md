# 📡 FT_IRC - Module Network Layer (C++98)

![C++98](https://img.shields.io/badge/Language-C%2B%2B98-blue.svg)
![POSIX](https://img.shields.io/badge/API-POSIX_Sockets-orange.svg)
![Multiplexing](https://img.shields.io/badge/Multiplexing-poll()-green.svg)
![Status](https://img.shields.io/badge/Status-Completed-brightgreen.svg)

---

## 📌 Présentation

Ce dépôt contient l'implémentation de la **couche réseau non-bloquante** pour le projet **`ft_irc`** (École 42). 

Cette architecture repose sur un modèle d'**Entrées/Sorties Multiplexées (I/O Multiplexing)** utilisant l'appel système POSIX `poll()`. Elle permet au serveur IRC de gérer simultanément un grand nombre de clients connectés au sein d'un unique fil d'exécution (**single-threaded event loop**), sans jamais bloquer le thread principal et sans consommer inutilement de ressources processeur (pas de boucle d'attente active / busy waiting).

---

## 🏛️ Architecture & Structure des Classes C++

Le module est découpé en trois composants principaux respectant les normes C++98 et les principes d'encapsulation orientée objet (RAII) :

```mermaid
graph TD
    Main["main_test.cpp<br/>(Boucle d'Événements & Signaux)"]
    Socket["ServerSocket<br/>(Socket IPv4, Non-blocking, Bind, Listen, Accept)"]
    Poll["PollManager<br/>(Vecteur de struct pollfd, Masques d'événements)"]
    Io["NetworkIo<br/>(I/O non-bloquant recv & send, Partial-sends)"]
    ClientCtx["ClientContext / AcceptedClient<br/>(FD, IP, Tampons In/Out)"]

    Main --> Socket
    Main --> Poll
    Main --> Io
    Main --> ClientCtx
    Socket --> Io
```

### 🧩 Composants du Projet

| Classe / Module | Fichiers Source | Description & Rôle Technique |
| :--- | :--- | :--- |
| **`ServerSocket`** | [`ServerSocket.hpp`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/ServerSocket.hpp)<br/>[`ServerSocket.cpp`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/ServerSocket.cpp) | Encapsule la création du socket TCP IPv4, la configuration de l'option `SO_REUSEADDR`, l'activation du mode non-bloquant (`O_NONBLOCK` via `fcntl`), l'association au port (`bind`), le passage en écoute (`listen`) et l'acceptation des clients (`accept`). |
| **`PollManager`** | [`PollManager.hpp`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/PollManager.hpp)<br/>[`PollManager.cpp`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/PollManager.cpp) | Gère la collection dynamique `std::vector<struct pollfd>`. Offre des méthodes d'activation/désactivation dynamique des masques d'événements (`POLLIN`, `POLLOUT`) et encapsule l'attente synchrone via `poll()`. |
| **`NetworkIo`** | [`NetworkIo.hpp`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/NetworkIo.hpp)<br/>[`NetworkIo.cpp`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/NetworkIo.cpp) | Classe statique utilitaire offrant les fonctions de lecture (`recvChunk`) et d'écriture (`sendFromBuffer`). Intercepte les interruptions réseau (`EAGAIN` / `EWOULDBLOCK`) et conserve le reliquat des données lors des envois partiels. |
| **`main_test`** | [`main_test.cpp`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/main_test.cpp) | Point d'entrée de l'application de test : interception des signaux POSIX (`SIGINT`, `SIGTERM`), exécution de la boucle événementielle et gestion du cycle de vie des clients. |

---

## ⚡ Caractéristiques Réseau Clés

- **Sockets Non-Bloquants (`O_NONBLOCK`)** : Tous les descripteurs de fichiers (socket d'écoute et sockets clients) sont marqués non-bloquants via `fcntl()`. Aucun appel réseau ne bloque le serveur.
- **Réutilisation de Port (`SO_REUSEADDR`)** : Permet de relancer le serveur immédiatement sans attendre l'expiration du délai `TIME_WAIT` du noyau TCP.
- **Gestion Dynamique de `POLLOUT`** : Le drapeau d'écriture `POLLOUT` n'est activé dans le `PollManager` que lorsqu'un client possède des données en attente d'émission dans son buffer de sortie (`outBuffer`), optimisant l'usage processeur.
- **Gestion des Envois Partiels (Partial Sends)** : Si l'appel système `send()` ne transmet qu'une fraction des données (buffer noyau plein), le reliquat reste dans le buffer applicatif pour être envoyé au tour suivant de `poll()`.
- **Fermeture Propre (Graceful Shutdown)** : Interception des signaux `SIGINT` (Ctrl+C) et `SIGTERM` pour libérer tous les sockets et structures de données sans fuite mémoire ni fuite de descripteurs de fichiers.

---

## 🛠️ Compilation & Utilisation

### Compilation

Le projet utilise un `Makefile` compatible C++98 strict (`-Wall -Wextra -Werror -std=c++98`) :

```bash
# Compilation du binaire de test
make

# Nettoyage des fichiers objets (.o)
make clean

# Re-compilation complète
make re
```

L'exécutable généré est nommé **`irc_server_test`**.

### Lancement du Serveur

```bash
# Lancement sur le port 6667 par défaut
./irc_server_test

# Lancement sur un port spécifique (ex: 8080)
./irc_server_test 8080
```

---

## 🧪 Test & Connexion Client

### Connexion via Netcat (`nc`)

Ouvrez un terminal secondaire et lancez la commande :

```bash
nc 127.0.0.1 6667
```

**Comportement attendu :**
1. Le client reçoit un message d'accueil IRC (`:server 001 Client :Bienvenue...`).
2. Dans le terminal du serveur, le log confirme la connexion :
   ```text
   [+ CONNECT] Nouveau client connecté | FD: 4 | IP: 127.0.0.1
   ```
3. Tapez une commande (ex: `NICK alex\r\n`) $\rightarrow$ Le serveur la lit sans bloquer et renvoie un écho.

### Connexion via Client IRC (`irssi`)

```bash
irssi -c 127.0.0.1 -p 6667
```

---

## 🔁 Deroulement Synoptique (Sequence Diagram)

```mermaid
sequenceDiagram
    autonumber
    actor Client as Client TCP (nc / irssi)
    participant Server as Serveur main (Event Loop)
    participant Socket as ServerSocket (FD 3)
    participant Poller as PollManager
    participant Io as NetworkIo

    Note over Server,Socket: 1. Initialisation
    Server->>Socket: createListeningSocket(6667, 10)
    Socket->>Socket: socket(), setsockopt(), fcntl(O_NONBLOCK), bind(), listen()
    Server->>Poller: addFd(FD 3, POLLIN)

    Note over Server,Client: 2. Connexion d'un Client
    Client->>Socket: TCP Handshake
    Poller-->>Server: poll() débloque (POLLIN sur FD 3)
    Server->>Socket: acceptClient() -> Retourne FD 4 (Non-bloquant)
    Server->>Poller: addFd(FD 4, POLLIN) & enableWrite(FD 4)

    Note over Server,Client: 3. Émission & Lecture
    Poller-->>Server: poll() débloque (POLLOUT sur FD 4)
    Server->>Io: sendFromBuffer(FD 4, welcome)
    Server->>Poller: disableWrite(FD 4)
    Client->>Io: Envoi données
    Poller-->>Server: poll() débloque (POLLIN sur FD 4)
    Server->>Io: recvChunk(FD 4)

    Note over Server,Client: 4. Déconnexion
    Client->>Socket: Envoi paquet FIN
    Poller-->>Server: poll() débloque (POLLHUP / POLLIN)
    Server->>Io: recvChunk(FD 4) -> 0 octets (closed = true)
    Server->>Poller: removeFd(FD 4) & closeFd(FD 4)
```

---

## 📚 Documentation Technique Complémentaire

Pour une analyse approfondie et une explication pas à pas adaptée à la soutenance ou à **NotebookLM** :

- 📄 **[`file.md`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/file.md) / [`SIMULATION_EXECUTION.md`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/SIMULATION_EXECUTION.md)** : Simulation complète d'exécution étape par étape, explications exhaustives de tous les appels système POSIX et des classes C++.
- 📄 **[`TESTING_GUIDE.md`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/TESTING_GUIDE.md)** : Guide de test pas à pas couvrant 6 scénarios de validation réseau.
- 📄 **[`WHAT_I_DID.md`](file:///Users/ayoubbareich/Desktop/ft_irc_git/ayoub_code/WHAT_I_DID.md)** : Résumé fonctionnel et guide d'intégration.

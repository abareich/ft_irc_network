# Guide de Test et Validation - Module Network Layer (`ayoub_code`)

Ce document explique pas à pas comment compiler, exécuter et tester le module réseau développé dans `ayoub_code/` (`AyoubSocket`, `AyoubPollManager`, `AyoubIo`). 

Chaque étape de test détaille **comment procéder** et **pourquoi ce test est effectué**.

---

## 1. Compilation du Projet

Pour compiler le binaire de test :

```bash
cd ayoub_code
make
```

- **Ce qui est généré** : Un fichier exécutable nommé `ayoub_server_test`.
- **Pourquoi cette étape ?** : Vérifier que le code respecte la norme C++98 (`-std=c++98`) et compile sans avertissements ni erreurs (`-Wall -Wextra -Werror`).

---

## 2. Étapes de Test Détaillées

### Test 1 : Démarrage du Serveur et Création du Socket d'Écoute

**Commande à exécuter :**
```bash
./ayoub_server_test 6667
```

**Pourquoi fait-on ce test ?**
1. **Validation de `socket()` & `setsockopt()`** : Vérifie que le socket TCP IPv4 (`AF_INET`, `SOCK_STREAM`) est correctement instancié et que l'option `SO_REUSEADDR` est activée pour réutiliser immédiatement le port sans attendre l'état `TIME_WAIT`.
2. **Validation de `fcntl()` (Non-blocking)** : S'assure que le socket d'écoute est mis en mode non-bloquant (`O_NONBLOCK`).
3. **Validation de `bind()` & `listen()`** : Associe le socket au port (6667) et lance l'écoute avec le backlog configuré.

---

### Test 2 : Connexion d'un Premier Client (`nc` / `telnet`)

**Commande à exécuter (dans un deuxième terminal) :**
```bash
nc 127.0.0.1 6667
```

**Résultat attendu :**
- Le client reçoit un message d'accueil IRC (`:server 001 Client :Bienvenue...`).
- Le serveur affiche dans son terminal : `[+ CONNECT] Nouveau client connecté | FD: 4 | IP: 127.0.0.1`.

**Pourquoi fait-on ce test ?**
1. **Validation d'`acceptClient()`** : Vérifie que `accept()` récupère le nouveau client sans bloquer le serveur.
2. **Validation du socket client non-bloquant** : S'assure que le socket du client est immédiatement passé en `O_NONBLOCK` via `setNonBlocking()`.
3. **Validation de l'enregistrement `poll()`** : Vérifie que le nouveau descripteur de fichier (FD) est ajouté avec succès dans `AyoubPollManager`.

---

### Test 3 : Connexions Simultanées (Multi-clients)

**Commande à exécuter :**
Ouvrir 3 terminaux différents et lancer dans chacun :
```bash
nc 127.0.0.1 6667
```

**Pourquoi fait-on ce test ?**
1. **Validation du multiplexage E/S avec `poll()`** : Garantit que le serveur gère plusieurs connexions simultanées sans bloquer sur un client lent ou inactif.
2. **Gestion dynamique du vecteur `pollfd`** : S'assure que `AyoubPollManager` redimensionne et gère la liste des FDs (`std::vector<struct pollfd>`) en toute sécurité.

---

### Test 4 : Échange de Données (Lecture et Écho - `POLLIN` / `POLLOUT`)

**Action :**
Dans le terminal où `nc 127.0.0.1 6667` est ouvert, tapez un message (ex: `PING :123456`) et appuyez sur Entrée.

**Résultat attendu :**
- Le serveur reçoit les octets et affiche : `[RECV] FD 4 (127.0.0.1) : PING :123456`.
- Le client reçoit en réponse : `[ECHO] PING :123456`.

**Pourquoi fait-on ce test ?**
1. **Validation d'`AyoubIo::recvChunk()`** : Teste la lecture par morceaux de 4096 octets et vérifie qu'aucune erreur `EAGAIN` / `EWOULDBLOCK` ne fait crasher le serveur.
2. **Validation d'`AyoubIo::sendFromBuffer()`** : Vérifie l'envoi non-bloquant du buffer de sortie.
3. **Validation de l'activation/désactivation dynamique de `POLLOUT`** :
   - `enableWrite(fd)` est appelé **uniquement** quand le buffer de sortie contient des données.
   - `disableWrite(fd)` est appelé dès que le buffer devient vide pour éviter de consommer du CPU inutilement avec `POLLOUT`.

---

### Test 5 : Déconnexion Propre d'un Client

**Action :**
Fermer l'un des terminaux clients avec `Ctrl+C` ou tapez `Ctrl+D`.

**Résultat attendu :**
- Le serveur détection la fermeture et affiche : `[- DISCONNECT] Client déconnecté | FD: 4`.

**Pourquoi fait-on ce test ?**
1. **Validation de la détection EOF (`bytes == 0`)** : S'assure que `recvChunk()` retourne `closed = true` quand le client ferme le socket.
2. **Prévention des fuites de File Descriptors** : Vérifie que `removeFd(fd)` retire le FD de la liste de `poll()` et que `closeFd(fd)` libère la ressource système.

---

### Test 6 : Arrêt Propre du Serveur (`SIGINT` / `Ctrl+C`)

**Action :**
Dans le terminal du serveur, appuyer sur `Ctrl+C`.

**Résultat attendu :**
- Le serveur intercepte le signal, ferme tous les sockets clients restants, ferme le listening socket, et affiche `[SERVER] Serveur arrêté proprement.`

**Pourquoi fait-on ce test ?**
1. **Validation de la libération des ressources** : S'assure qu'aucun socket ne reste ouvert à l'arrêt du programme, évitant le blocage du port lors des redémarrages futurs.

---

## Synthèse des Tests

| Test # | Description | Composant Testé | Raison / Obectif principal |
| :---: | :--- | :--- | :--- |
| **01** | Initialisation & Listen | `AyoubSocket` | Valider `socket`, `bind`, `listen`, `SO_REUSEADDR` & non-blocage |
| **02** | Connexion Client | `AyoubSocket` + `AyoubPollManager` | Valider `acceptClient()` non-bloquant et l'ajout au `poll` |
| **03** | Multi-clients | `AyoubPollManager` | Valider le multiplexage sans blocage de l'Event Loop |
| **04** | Communication (E/S) | `AyoubIo` + `POLLOUT` | Valider `recvChunk`, `sendFromBuffer` et le toggle dynamique de `POLLOUT` |
| **05** | Déconnexion Client | `AyoubPollManager` + `AyoubSocket` | Valider le nettoyage des FDs et éviter les fuites de ressources |
| **06** | Fermeture Serveur | Global (`AyoubSocket`) | Valider le shutdown propre sur signal `SIGINT` |

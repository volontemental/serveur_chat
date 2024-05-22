# Serveur IRC - Documentation

## Introduction

Bienvenue dans la documentation du serveur IRC (Internet Relay Chat). Ce document vise à expliquer de manière détaillée le fonctionnement du code du serveur IRC, la logique derrière son implémentation, ainsi que les concepts et définitions importants. Il est conçu pour être compréhensible même par quelqu'un qui n'a jamais codé.

## Qu'est-ce qu'un Serveur IRC ?

Un serveur IRC est une application qui permet à plusieurs utilisateurs de discuter ensemble dans des canaux (ou salles) de discussion en temps réel. Chaque utilisateur se connecte au serveur avec un client IRC, peut rejoindre des canaux, envoyer des messages privés, et interagir avec d'autres utilisateurs.

## Fonctionnement Général

### Initialisation

Le programme commence dans `main.cpp`, où il effectue les étapes suivantes :

1. **Configuration du serveur** : Lit le port et le mot de passe du serveur à partir des arguments de la ligne de commande.
2. **Création du socket serveur** : Initialise un socket pour écouter les connexions entrantes.
3. **Gestion des connexions** : Utilise `poll` pour surveiller les connexions entrantes et les messages des clients.

### Gestion des Clients

Lorsqu'un client se connecte :

1. Le serveur accepte la connexion et crée un objet `Client` pour représenter le nouvel utilisateur.
2. Le client doit s'authentifier en envoyant une commande `PASS` avec le mot de passe correct.
3. Une fois authentifié, le client peut envoyer diverses commandes IRC (par exemple, `NICK`, `JOIN`, `PRIVMSG`).

### Gestion des Commandes IRC

Les commandes IRC sont traitées dans `irc_commands.cpp` :

1. **PING/PONG** : Utilisé pour vérifier si le client est toujours connecté.
2. **PRIVMSG** : Envoie un message privé à un utilisateur ou à un canal.
3. **JOIN** : Permet à un utilisateur de rejoindre un canal.
4. **KICK** : Expulse un utilisateur d'un canal.
5. **INVITE** : Invite un utilisateur à rejoindre un canal.
6. **TOPIC** : Définit ou affiche le sujet d'un canal.
7. **MODE** : Change les modes d'un canal ou d'un utilisateur.
8. **LIST** : Affiche la liste des canaux actifs.
9. **NAMES** : Affiche la liste des utilisateurs dans un canal.
10. **WHOIS** : Affiche des informations sur un utilisateur.

### Gestion des Canaux

Les canaux sont représentés par la classe `Channel` dans `channel.cpp` :

1. Chaque canal a un nom, un sujet, une liste d'utilisateurs, et des modes (par exemple, invitation uniquement).
2. Les utilisateurs peuvent rejoindre ou quitter des canaux, et envoyer des messages à tous les utilisateurs du canal.

### Fonctionnalités Utilitaires

Les fonctionnalités utilitaires sont implémentées dans `utils.cpp` :

1. **Envoi de messages** : `sendToClient` envoie un message à un client via son socket.
2. **Manipulation de chaînes** : `trim`, `ltrim`, `rtrim` suppriment les espaces en début et en fin de chaînes.

## Concepts Clés et Définitions

- **Socket** : Un point de communication pour envoyer et recevoir des données entre deux machines. Il agit comme un canal par lequel les données peuvent être envoyées et reçues.
- **Canal** : Une salle de discussion où plusieurs utilisateurs peuvent envoyer et recevoir des messages. Chaque canal est identifié par un nom unique.
- **Ping/Pong** : Mécanisme utilisé pour vérifier la connectivité entre le client et le serveur. Le serveur envoie un PING et attend une réponse PONG du client pour s'assurer que la connexion est toujours active.
- **Opérateur** : Utilisateur ayant des privilèges spéciaux dans un canal, comme la capacité de changer le sujet ou d'expulser des utilisateurs. Les opérateurs sont souvent responsables de la modération des discussions.
- **Mode d'invitation uniquement** : Mode où seuls les utilisateurs invités peuvent rejoindre le canal. Cela permet de restreindre l'accès à un groupe sélectionné d'utilisateurs.
- **Clé du canal** : Mot de passe requis pour rejoindre un canal. Cela ajoute une couche de sécurité en restreignant l'accès aux utilisateurs qui connaissent la clé.
- **Sujet** : Le sujet de discussion d'un canal, visible par tous les utilisateurs du canal. Le sujet peut être défini par les opérateurs et donne une idée du thème de discussion du canal.

## Points Importants

- **Gestion des erreurs** : Le serveur gère diverses erreurs, comme les mots de passe incorrects, les commandes mal formées, ou les permissions insuffisantes. Chaque erreur est signalée au client avec un message explicatif.
- **Sécurité** : Utilisation d'un mot de passe pour l'authentification des utilisateurs, et possibilité de protéger les canaux avec des clés.
- **Robustesse** : Gestion des déconnexions inattendues et des pannes de réseau. Le serveur utilise des mécanismes comme `poll` pour surveiller les connexions et détecter les clients inactifs ou déconnectés.

## Conclusion

Ce projet implémente un serveur IRC de base en suivant les principes fondamentaux des protocoles de communication réseau. Il est conçu pour être extensible et robuste, permettant d'ajouter facilement de nouvelles fonctionnalités ou de modifier les existantes. Nous espérons que cette documentation vous aidera à comprendre le fonctionnement et la logique derrière ce serveur IRC.

---


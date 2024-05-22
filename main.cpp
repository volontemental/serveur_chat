/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:46:22 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:46:23 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <ctime>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <fcntl.h>
#include "channel.hpp"
#include "irc_commands.hpp"
#include "client.hpp"
#include "utils.hpp"

// Constantes pour la taille du buffer, l'intervalle de ping et le timeout du pong
const int BUFFER_SIZE = 1025;
const int PING_INTERVAL = 500000; // Intervalle en microsecondes pour envoyer des PING aux clients
const int PONG_TIMEOUT = 2400000; // Temps en microsecondes avant de considérer qu'un client ne répond pas

// Map globale des clients et des canaux
std::map<int, Client> clients; // Stocke les clients connectés avec leur socket comme clé
std::map<std::string, Channel> channels; // Stocke les canaux avec leur nom comme clé

// Variable pour contrôler l'exécution du serveur
bool running = true; // Indicateur pour savoir si le serveur est en cours d'exécution

// Gestionnaire de signal pour arrêter le serveur proprement
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") reçu. Arrêt du serveur." << std::endl;
    running = false;
}

// Fonction pour configurer un socket en mode non-bloquant
int setNonBlocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) {
        perror("Erreur : fcntl(F_GETFL)");
        return -1;
    }
    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Erreur : fcntl(F_SETFL)");
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // Vérification des arguments de la ligne de commande
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);  // Récupération du port
    std::string password = argv[2]; // Récupération du mot de passe

    // Attachement du gestionnaire de signal pour SIGINT
    signal(SIGINT, signalHandler);

    int server_fd;  // Descripteur de fichier pour le socket du serveur
    struct sockaddr_in address;  // Structure d'adresse pour le socket
    int opt = 1;  // Option pour setsockopt
    int addrlen = sizeof(address);  // Taille de l'adresse

    // Création du socket du serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erreur : Échec de la création du socket");
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket créé avec succès." << std::endl;

    // Configuration des options du socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        perror("Erreur : Échec de setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Options de socket définies avec succès." << std::endl;

    // Configuration de l'adresse du socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Liaison du socket à l'adresse et au port spécifiés
    if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1) {
        perror("Erreur : Échec de bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Bind réussi sur le port " << port << "." << std::endl;

    // Mise en écoute du socket
    if (listen(server_fd, 10) == -1) {
        perror("Erreur : Échec de listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Serveur en écoute sur le port " << port << "." << std::endl;

    // Configuration du socket du serveur en mode non-bloquant
    if (setNonBlocking(server_fd) == -1) {
        std::cerr << "Erreur : Échec de la configuration du socket en mode non-bloquant" << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket du serveur configuré en mode non-bloquant." << std::endl;

    struct pollfd pfds[30];  // Tableau de structures pollfd pour gérer les événements sur les sockets
    int nfds = 1;  // Nombre de descripteurs de fichiers suivis
    pfds[0].fd = server_fd;  // Ajouter le socket du serveur à la liste
    pfds[0].events = POLLIN;  // Surveiller les événements de lecture

    time_t lastPingTime = time(NULL);  // Heure du dernier ping envoyé

    while (running) {
        // Utilisation de poll pour surveiller les événements sur les sockets
        int poll_count = poll(pfds, nfds, 1000);

        if (poll_count == -1) {
            perror("Erreur : poll");
            running = false;
            break;
        }

        for (int i = 0; i < nfds; i++) {
            // Vérifier les événements de lecture
            if (pfds[i].revents & POLLIN) {
                // Si l'événement concerne le socket du serveur, accepter une nouvelle connexion
                if (pfds[i].fd == server_fd) {
                    int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                    if (new_socket == -1) {
                        perror("Erreur : Échec de accept");
                        continue;
                    }

                    std::cout << "Nouvelle connexion, socket fd est " << new_socket
                              << ", ip est: " << inet_ntoa(address.sin_addr)
                              << ", port: " << ntohs(address.sin_port) << std::endl;

                    // Configurer le nouveau socket en mode non-bloquant
                    if (setNonBlocking(new_socket) == -1) {
                        std::cerr << "Erreur : Échec de la configuration du socket client en mode non-bloquant" << std::endl;
                        close(new_socket);
                        continue;
                    }

                    // Ajouter le nouveau client à la liste des clients
                    clients.insert(std::make_pair(new_socket, Client(new_socket, "")));
                    pfds[nfds].fd = new_socket;
                    pfds[nfds].events = POLLIN;
                    nfds++;

                    // Envoyer un message de bienvenue au nouveau client
                    sendWelcomeMessage(new_socket);
                } else {
                    // Lire les données du client
                    char buffer[BUFFER_SIZE];
                    int valread = read(pfds[i].fd, buffer, BUFFER_SIZE - 1);
                    if (valread > 0) {
                        buffer[valread] = '\0';
                        Client& client = clients[pfds[i].fd];
                        client.appendToCommandBuffer(std::string(buffer));

                        std::string& commandBuffer = client.getCommandBuffer();
                        size_t pos;
                        while ((pos = commandBuffer.find('\n')) != std::string::npos) {
                            std::string fullCommand = commandBuffer.substr(0, pos);
                            commandBuffer.erase(0, pos + 1);

                            // Traiter les commandes du client
                            if (!client.isAuthenticated()) {
                                if (fullCommand.find("PASS ") == 0) {
                                    handleCommand(pfds[i].fd, fullCommand, clients, channels, password);
                                    client.setSentAuthError(false);
                                } else if (!client.hasSentAuthError()) {
                                    sendToClient(pfds[i].fd, "ERROR :Vous devez d'abord vous authentifier avec PASS\r\n");
                                    client.setSentAuthError(true);
                                }
                            } else {
                                handleCommand(pfds[i].fd, fullCommand, clients, channels, password);
                            }
                        }
                    } else if (valread == 0) {
                        // Si la lecture retourne 0, le client a fermé la connexion
                        std::cout << "Hôte déconnecté, socket fd est " << pfds[i].fd << std::endl;
                        close(pfds[i].fd);
                        clients.erase(pfds[i].fd);
                        for (int j = i; j < nfds - 1; j++) {
                            pfds[j] = pfds[j + 1];
                        }
                        nfds--;
                    } else {
                        // Gestion des erreurs de lecture
                        if (errno != EWOULDBLOCK && errno != EAGAIN) {
                            perror("Erreur lors de la lecture du socket client");
                            close(pfds[i].fd);
                            clients.erase(pfds[i].fd);
                            for (int j = i; j < nfds - 1; j++) {
                                pfds[j] = pfds[j + 1];
                            }
                            nfds--;
                        }
                    }
                }
            }
        }

        // Vérifier si l'intervalle de ping est dépassé
        time_t currentTime = time(NULL);
        if (difftime(currentTime, lastPingTime) >= PING_INTERVAL) {
            for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ) {
                // Envoyer un ping aux clients
                sendToClient(it->first, "PING :server");

                // Vérifier si le client a répondu au ping
                if (difftime(currentTime, it->second.getLastPong()) >= PONG_TIMEOUT) {
                    std::cout << "Le client " << it->first << " n'a pas répondu au PING. Déconnexion." << std::endl;
                    close(it->first);
                    std::map<int, Client>::iterator toErase = it++;
                    clients.erase(toErase);

                    for (int j = 0; j < nfds; j++) {
                        if (pfds[j].fd == toErase->first) {
                            for (int k = j; k < nfds - 1; k++) {
                                pfds[k] = pfds[k + 1];
                            }
                            nfds--;
                            break;
                        }
                    }
                } else {
                    ++it;
                }
            }
            lastPingTime = currentTime;
        }
    }

    // Fermer tous les descripteurs de fichiers avant de quitter
    for (int i = 0; i < nfds; i++) {
        if (pfds[i].fd >= 0) close(pfds[i].fd);
    }

    std::cout << "Serveur arrêté." << std::endl;
    return 0;
}

/*
Explications détaillées :
- **signalHandler** : Cette fonction gère les signaux d'interruption (comme Ctrl+C) pour arrêter le serveur proprement. Elle définit la variable `running` sur false, ce qui provoque l'arrêt de la boucle principale.
- **setNonBlocking** : Configure un socket en mode non-bloquant, ce qui signifie que les appels de fonction qui seraient normalement bloquants, comme `read` ou `accept`, retourneront immédiatement s'ils ne peuvent pas être exécutés.
- **main** : Fonction principale du programme. Voici un résumé détaillé de ses actions :
  - Vérifie que les arguments de la ligne de commande sont corrects (un port et un mot de passe).
  - Configure le gestionnaire de signal pour arrêter le serveur proprement en cas d'interruption.
  - Crée le socket du serveur, configure les options du socket, lie le socket à l'adresse spécifiée et le met en écoute pour les connexions entrantes.
  - Configure le socket du serveur en mode non-bloquant.
  - Initialise une structure `pollfd` pour surveiller les événements sur le socket du serveur.
  - Boucle principale : utilise `poll` pour surveiller les événements sur les sockets :
    - Accepte les nouvelles connexions et configure les nouveaux sockets en mode non-bloquant.
    - Lit les données des clients, traite les commandes et gère les déconnexions.
    - Envoie des pings aux clients et déconnecte ceux qui ne répondent pas.
  - Ferme tous les descripteurs de fichiers avant de quitter le programme.

Définitions des termes importants :
- **Socket** : Un point de communication entre deux machines pour envoyer et recevoir des données.
- **Non-bloquant** : Mode d'opération où les appels de fonction ne bloquent pas l'exécution du programme en attendant une opération, mais retournent immédiatement.
- **poll** : Une fonction qui surveille plusieurs descripteurs de fichiers pour voir s'ils sont prêts pour une opération d'E/S.
- **PING/PONG** : Mécanisme utilisé pour vérifier si un client est toujours connecté. Le serveur envoie un PING et attend une réponse PONG du client.
- **SIGINT** : Signal d'interruption envoyé par le système d'exploitation, généralement en appuyant sur Ctrl+C dans le terminal.
- **bind** : Fonction qui assigne une adresse spécifique à un socket.
- **listen** : Fonction qui met le socket en mode écoute pour accepter les connexions entrantes.
- **accept** : Fonction qui accepte une nouvelle connexion sur un socket en écoute.
- **fcntl** : Fonction utilisée pour manipuler les descripteurs de fichiers, comme configurer un socket en mode non-bloquant.
- **POLLIN** : Événement qui indique qu'il y a des données à lire sur le socket.

*/


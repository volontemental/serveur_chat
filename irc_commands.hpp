/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc_commands.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:46:54 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:46:56 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_COMMANDS_HPP
#define IRC_COMMANDS_HPP

#include <string>
#include <map>
#include "client.hpp"
#include "channel.hpp"

// Fonction pour traiter une commande reçue d'un client
void handleCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels, const std::string& serverPassword);

// Fonctions pour traiter les commandes spécifiques
void handlePing(int clientSocket, const std::string& message);
void handlePong(int clientSocket);
void handlePrivmsgCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels);
void handleListCommand(int clientSocket, const std::map<std::string, Channel>& channels);
void handleNamesCommand(int clientSocket, const std::string& channelName, const std::map<std::string, Channel>& channels, const Client& client);
void handleWhoisCommand(int clientSocket, const std::string& nickname, const std::map<int, Client>& clients);
void handleKickCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels);
void handleInviteCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels);
void handleTopicCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels);
void handleModeCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels);

// Fonction pour envoyer un message de bienvenue à un client
void sendWelcomeMessage(int clientSocket);

// Fonction pour traiter la commande JOIN
void handleJoinCommand(int clientSocket, const std::string& channelName, const std::string& key, std::map<int, Client>& clients, std::map<std::string, Channel>& channels);

// Fonction pour vérifier si un nom de canal est valide
bool isValidChannelName(const std::string& channelName);

#endif // IRC_COMMANDS_HPP

/*
Explications détaillées :
- **handleCommand** : Fonction principale pour traiter les commandes reçues d'un client. Elle décode la commande et appelle la fonction appropriée pour la traiter.
- **handlePing, handlePong** : Fonctions pour traiter les commandes PING et PONG utilisées pour vérifier la connectivité entre le client et le serveur.
- **handlePrivmsgCommand** : Traite la commande PRIVMSG qui envoie un message privé à un utilisateur ou à un canal.
- **handleListCommand** : Traite la commande LIST qui renvoie la liste des canaux actifs.
- **handleNamesCommand** : Traite la commande NAMES qui renvoie la liste des utilisateurs dans un canal.
- **handleWhoisCommand** : Traite la commande WHOIS qui renvoie des informations sur un utilisateur spécifique.
- **handleKickCommand** : Traite la commande KICK qui expulse un utilisateur d'un canal.
- **handleInviteCommand** : Traite la commande INVITE qui invite un utilisateur à rejoindre un canal.
- **handleTopicCommand** : Traite la commande TOPIC qui permet de définir ou de consulter le sujet d'un canal.
- **handleModeCommand** : Traite la commande MODE qui permet de définir ou de consulter les modes d'un canal ou d'un utilisateur.
- **sendWelcomeMessage** : Envoie un message de bienvenue à un client lorsqu'il se connecte au serveur.
- **handleJoinCommand** : Traite la commande JOIN qui permet à un utilisateur de rejoindre un canal.
- **isValidChannelName** : Vérifie si un nom de canal est valide, c'est-à-dire s'il ne contient pas d'espaces.
*/


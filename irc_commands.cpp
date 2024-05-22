/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc_commands.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:46:11 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:46:13 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc_commands.hpp"
#include "utils.hpp"
#include "client.hpp"
#include "channel.hpp"
#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <unistd.h>
#include <sstream>

// Déclaration des variables externes
extern std::map<int, Client> clients;
extern std::string serverPassword;

// Vérifie si un nom de canal est valide (ne contient pas d'espaces)
bool isValidChannelName(const std::string& channelName) {
    return channelName.find(' ') == std::string::npos;
}

// Traite une commande PING en envoyant une réponse PONG au client
void handlePing(int clientSocket, const std::string& message) {
    std::cout << "Réception d'un PING de la part du client " << clientSocket << " avec le message: " << message << std::endl;
    std::string response = "PONG " + message + "\r\n";
    if (send(clientSocket, response.c_str(), response.size(), 0) < 0) {
        std::cerr << "Erreur : Échec de l'envoi du PONG au client " << clientSocket << ": " << strerror(errno) << std::endl;
    } else {
        std::cout << "PONG envoyé au client " << clientSocket << std::endl;
    }
}

// Traite une commande PONG en mettant à jour l'heure du dernier PONG du client
void handlePong(int clientSocket) {
    std::cout << "Réception d'un PONG de la part du client " << clientSocket << std::endl;
    std::map<int, Client>::iterator it = clients.find(clientSocket);
    if (it != clients.end()) {
        it->second.updateLastPong();
        std::cout << "Heure du dernier PONG mise à jour pour le client " << clientSocket << std::endl;
    }
}

// Traite la commande KICK pour expulser un utilisateur d'un canal
void handleKickCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels) {
    std::cout << "Traitement de la commande KICK de la part du client " << clientSocket << ": " << command << std::endl;
    size_t firstSpace = command.find(' ');
    size_t secondSpace = command.find(' ', firstSpace + 1);
    size_t thirdSpace = command.find(' ', secondSpace + 1);
    if (firstSpace != std::string::npos && secondSpace != std::string::npos && thirdSpace != std::string::npos) {
        std::string channelName = command.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        std::string userToKick = command.substr(secondSpace + 1, thirdSpace - secondSpace - 1);
        std::string reason = command.substr(thirdSpace + 1);
        trim(channelName);
        trim(userToKick);
        trim(reason);

        if (channels.find(channelName) != channels.end()) {
            Channel& channel = channels[channelName];
            if (channel.hasUser(userToKick)) {
                if (channel.isOperator(clients[clientSocket].getNickname())) {
                    std::cout << "Utilisateur " << userToKick << " trouvé dans le canal " << channelName << ". Procédure de kick en cours..." << std::endl;
                    channel.removeUser(userToKick);
                    std::map<int, Client>::iterator it = std::find_if(clients.begin(), clients.end(), NicknameEquals(userToKick));
                    if (it != clients.end()) {
                        it->second.leaveChannel(channelName);
                        std::string message = ":" + clients[clientSocket].getNickname() + " KICK " + channelName + " " + userToKick + " :" + reason + "\r\n";
                        send(it->first, message.c_str(), message.size(), 0);
                        std::cout << "Utilisateur " << userToKick << " a été kické du canal " << channelName << " pour la raison: " << reason << std::endl;
                    }
                } else {
                    std::string response = ":server 482 " + channelName + " :You're not channel operator\r\n";
                    send(clientSocket, response.c_str(), response.size(), 0);
                    std::cout << "Erreur: Le client " << clientSocket << " n'est pas opérateur du canal " << channelName << std::endl;
                }
            } else {
                std::cout << "Utilisateur " << userToKick << " n'est pas dans le canal " << channelName << std::endl;
            }
        } else {
            std::cout << "Canal " << channelName << " non trouvé." << std::endl;
        }
    } else {
        std::cout << "Commande KICK mal formée reçue: " << command << std::endl;
    }
}

// Traite la commande INVITE pour inviter un utilisateur à rejoindre un canal
void handleInviteCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels) {
    std::cout << "Traitement de la commande INVITE de la part du client " << clientSocket << ": " << command << std::endl;
    size_t firstSpace = command.find(' ');
    size_t secondSpace = command.find(' ', firstSpace + 1);
    if (firstSpace != std::string::npos && secondSpace != std::string::npos) {
        std::string userToInvite = command.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        std::string channelName = command.substr(secondSpace + 1);
        trim(userToInvite);
        trim(channelName);
        
        if (channels.find(channelName) != channels.end()) {
            std::map<int, Client>::iterator it = std::find_if(clients.begin(), clients.end(), NicknameEquals(userToInvite));
            if (it != clients.end()) {
                Channel& channel = channels[channelName];
                channel.addInvitedUser(userToInvite);
                std::string message = ":" + clients[clientSocket].getNickname() + " INVITE " + userToInvite + " " + channelName + "\r\n";
                send(it->first, message.c_str(), message.size(), 0);
                std::cout << "Utilisateur " << userToInvite << " invité au canal " << channelName << std::endl;
            }
        }
    }
}

// Traite la commande TOPIC pour définir ou obtenir le sujet d'un canal
void handleTopicCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels) {
    std::cout << "Traitement de la commande TOPIC de la part du client " << clientSocket << ": " << command << std::endl;
    size_t firstSpace = command.find(' ');
    size_t secondSpace = command.find(' ', firstSpace + 1);
    if (firstSpace != std::string::npos && secondSpace != std::string::npos) {
        std::string channelName = command.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        std::string topic = command.substr(secondSpace + 1);
        trim(channelName);
        trim(topic);
        
        if (channels.find(channelName) != channels.end()) {
            Channel& channel = channels[channelName];
            if (!channel.isTopicProtected() || channel.isOperator(clients[clientSocket].getNickname())) {
                channel.setTopic(topic);
                std::string message = ":" + clients[clientSocket].getNickname() + " TOPIC " + channelName + " :" + topic + "\r\n";
                channel.broadcast(message, clients, clientSocket);
                std::cout << "Nouveau sujet du canal " << channelName << ": " << topic << std::endl;
            } else {
                std::string response = ":server 482 " + channelName + " :You do not have permission to change the topic\r\n";
                send(clientSocket, response.c_str(), response.size(), 0);
                std::cout << "Erreur: Le client " << clientSocket << " n'a pas la permission de changer le sujet du canal " << channelName << std::endl;
            }
        }
    }
}

// Traite la commande MODE pour définir ou obtenir les modes d'un canal
void handleModeCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels) {
    std::cout << "Traitement de la commande MODE de la part du client " << clientSocket << ": " << command << std::endl;
    size_t firstSpace = command.find(' ');
    size_t secondSpace = command.find(' ', firstSpace + 1);
    size_t thirdSpace = command.find(' ', secondSpace + 1);

    if (firstSpace != std::string::npos && secondSpace != std::string::npos) {
        std::string target = command.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        std::string mode;
        std::string user;
        bool add = true;

        if (thirdSpace != std::string::npos) {
            mode = command.substr(secondSpace + 1, thirdSpace - secondSpace - 1);
            user = command.substr(thirdSpace + 1);
        } else {
            mode = command.substr(secondSpace + 1);
        }

        trim(target);
        trim(mode);
        trim(user);

        if (mode[0] == '-') {
            add = false;
            mode = mode.substr(1);
        } else if (mode[0] == '+') {
            mode = mode.substr(1);
        }

        if (channels.find(target) != channels.end()) {
            Channel& channel = channels[target];
            std::string message = ":" + clients[clientSocket].getNickname() + " MODE " + target + " " + (add ? "+" : "-") + mode + (user.empty() ? "" : " " + user) + "\r\n";

            if (mode == "o") {
                if (add) {
                    channel.addOperator(user);
                } else {
                    channel.removeOperator(user);
                }
                channel.broadcast(message, clients, clientSocket);
                std::cout << "Utilisateur " << user << (add ? " est maintenant opérateur " : " n'est plus opérateur ") << "du canal " << target << std::endl;
            } else if (mode == "i") {
                channel.setInviteOnly(add);
                channel.broadcast(message, clients, clientSocket);
                std::cout << "Mode " << (add ? "ajouté" : "retiré") << " pour le canal " << target << ": " << mode << std::endl;
            } else if (mode == "t") {
                channel.setTopicProtection(add);
                channel.broadcast(message, clients, clientSocket);
                std::cout << "Mode " << (add ? "ajouté" : "retiré") << " pour le canal " << target << ": " << mode << std::endl;
            } else if (mode == "k") {
                if (add) {
                    channel.setKey(user);
                    channel.broadcast(message, clients, clientSocket);
                    std::cout << "Clé du canal " << target << " définie à: " << user << std::endl;
                } else {
                    channel.setKey("");
                    channel.broadcast(message, clients, clientSocket);
                    std::cout << "Clé du canal " << target << " retirée" << std::endl;
                }
            } else if (mode == "l") {
                if (add) {
                    std::istringstream iss(user);
                    size_t limit;
                    iss >> limit;
                    channel.setUserLimit(limit);
                    channel.broadcast(message, clients, clientSocket);
                    std::cout << "Limite d'utilisateurs du canal " << target << " définie à: " << limit << std::endl;
                } else {
                    channel.setUserLimit(0);
                    channel.broadcast(message, clients, clientSocket);
                    std::cout << "Limite d'utilisateurs du canal " << target << " retirée" << std::endl;
                }
            } else {
                std::cout << "Mode non reconnu: " << mode << std::endl;
            }
        } else {
            std::map<int, Client>::iterator it = clients.find(clientSocket);
            if (it != clients.end()) {
                std::string response = ":server 403 " + it->second.getNickname() + " " + target + " :No such channel\r\n";
                send(clientSocket, response.c_str(), response.size(), 0);
                std::cout << "Erreur: Canal " << target << " non trouvé." << std::endl;
            }
        }
    }
}

// Traite la commande LIST pour renvoyer la liste des canaux actifs
void handleListCommand(int clientSocket, const std::map<std::string, Channel>& channels) {
    std::cout << "Traitement de la commande LIST de la part du client " << clientSocket << std::endl;
    std::string response;
    for (std::map<std::string, Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
        std::ostringstream oss;
        oss << it->second.userCount();
        response = ":server 322 " + it->first + " :" + oss.str() + "\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Liste des canaux envoyée au client " << clientSocket << std::endl;
    }
    response = ":server 323 :End of LIST\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
    std::cout << "Fin de la liste des canaux envoyée au client " << clientSocket << std::endl;
}

// Traite la commande NAMES pour renvoyer la liste des utilisateurs dans un canal
void handleNamesCommand(int clientSocket, const std::string& channelName, const std::map<std::string, Channel>& channels, const Client& client) {
    std::cout << "Traitement de la commande NAMES de la part du client " << clientSocket << std::endl;
    std::map<std::string, Channel>::const_iterator it = channels.find(channelName);
    if (it != channels.end()) {
        std::string response = ":server 353 " + client.getNickname() + " = " + channelName + " :";
        std::set<std::string> users = it->second.listUsers();
        for (std::set<std::string>::const_iterator userIt = users.begin(); userIt != users.end(); ++userIt) {
            response += *userIt + " ";
        }
        response += "\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Liste des utilisateurs du canal " << channelName << " envoyée au client " << clientSocket << std::endl;
        response = ":server 366 " + client.getNickname() + " " + channelName + " :End of NAMES list\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Fin de la liste des utilisateurs du canal " << channelName << " envoyée au client " << clientSocket << std::endl;
    } else {
        std::string response = ":server 403 " + channelName + " :No such channel\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Canal " << channelName << " non trouvé. Message d'erreur envoyé au client " << clientSocket << std::endl;
    }
}

// Traite la commande WHOIS pour renvoyer des informations sur un utilisateur spécifique
void handleWhoisCommand(int clientSocket, const std::string& nickname, const std::map<int, Client>& clients) {
    std::cout << "Traitement de la commande WHOIS de la part du client " << clientSocket << " pour le pseudonyme " << nickname << std::endl;
    std::map<int, Client>::const_iterator it = std::find_if(clients.begin(), clients.end(), NicknameEquals(nickname));
    if (it != clients.end()) {
        std::string response = ":server 311 " + it->second.getNickname() + " " + it->second.getNickname() + " " + it->second.getNickname() + " * :" + it->second.getNickname() + "\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Informations WHOIS envoyées pour le pseudonyme " << nickname << " au client " << clientSocket << std::endl;
        response = ":server 318 " + it->second.getNickname() + " :End of WHOIS list\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Fin de la liste WHOIS envoyée pour le pseudonyme " << nickname << " au client " << clientSocket << std::endl;
    } else {
        std::string response = ":server 401 " + nickname + " :No such nick/channel\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Pseudonyme " << nickname << " non trouvé. Message d'erreur envoyé au client " << clientSocket << std::endl;
    }
}

// Traite la commande PRIVMSG pour envoyer un message privé à un utilisateur ou à un canal
void handlePrivmsgCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels) {
    size_t spacePos = command.find(' ');
    size_t colonPos = command.find(':', spacePos + 1);

    if (spacePos != std::string::npos && colonPos != std::string::npos) {
        std::string recipient = command.substr(spacePos + 1, colonPos - spacePos - 1);
        std::string message = command.substr(colonPos + 1);

        trim(recipient);
        trim(message);

        std::string sender_nick = clients[clientSocket].getNickname();
        std::string sender_user = sender_nick;
        std::string sender_host = "localhost";

        std::cout << "Traitement de PRIVMSG de " << sender_nick << " à " << recipient << " avec le message: " << message << std::endl;

        if (recipient.empty() || message.empty()) {
            std::cerr << "Erreur : Destinataire ou message vide." << std::endl;
            sendToClient(clientSocket, ":server 411 :No recipient given (PRIVMSG)\r\n");
            return;
        }

        std::string formattedMessage = ":" + sender_nick + "!" + sender_user + "@" + sender_host + " PRIVMSG " + recipient + " :" + message + "\r\n";

        std::cout << "Message formaté : " << formattedMessage << std::endl;

        if (recipient[0] == '#') {
            // Message à un canal
            std::map<std::string, Channel>::iterator channelIt = channels.find(recipient);
            if (channelIt != channels.end()) {
                Channel& channel = channelIt->second;
                if (channel.hasUser(sender_nick)) {
                    std::cout << "Envoi du message au canal " << recipient << std::endl;
                    channel.broadcast(formattedMessage, clients, clientSocket);
                } else {
                    std::cerr << "Erreur : Utilisateur non présent dans le canal" << std::endl;
                    sendToClient(clientSocket, ":server 442 " + recipient + " :You're not on that channel\r\n");
                }
            } else {
                std::cerr << "Erreur : Canal introuvable" << std::endl;
                sendToClient(clientSocket, ":server 403 " + recipient + " :No such channel\r\n");
            }
        } else {
            // Message privé à un utilisateur
            std::map<int, Client>::iterator clientIt = std::find_if(clients.begin(), clients.end(), NicknameEquals(recipient));
            if (clientIt != clients.end()) {
                std::cout << "Envoi du message privé à " << recipient << " (Socket: " << clientIt->first << ")" << std::endl;
                if (send(clientIt->first, formattedMessage.c_str(), formattedMessage.size(), 0) < 0) {
                    std::cerr << "Erreur lors de l'envoi du message au client " << clientIt->first << ": " << strerror(errno) << std::endl;
                } else {
                    std::cout << "Message privé envoyé à " << recipient << " (Socket: " << clientIt->first << ")" << std::endl;
                }
            } else {
                std::cerr << "Erreur : Utilisateur introuvable" << std::endl;
                sendToClient(clientSocket, ":server 401 " + recipient + " :No such nick/channel\r\n");
            }
        }
    } else {
        std::cerr << "Erreur : Commande PRIVMSG mal formée" << std::endl;
        sendToClient(clientSocket, ":server 461 PRIVMSG :Not enough parameters\r\n");
    }
}

// Envoie un message de bienvenue à un client lorsqu'il se connecte
void sendWelcomeMessage(int clientSocket) {
    std::string welcomeMessage = ":server 001 " + clients[clientSocket].getNickname() + " :Welcome to the IRC server!\r\n";
    if (sendToClient(clientSocket, welcomeMessage) == -1) {
        std::cerr << "Erreur : Échec de l'envoi du message de bienvenue au client " << clientSocket << std::endl;
    }
}

// Traite la commande JOIN pour permettre à un utilisateur de rejoindre un canal
void handleJoinCommand(int clientSocket, const std::string& channelName, const std::string& key, std::map<int, Client>& clients, std::map<std::string, Channel>& channels) {
    std::string channel = channelName;
    trim(channel);

    if (channel.empty()) {
        std::cout << "Erreur: Nom de canal vide dans la commande JOIN." << std::endl;
        return;
    }
    if (!isValidChannelName(channel)) {
        sendToClient(clientSocket, "ERROR :Invalid channel name\r\n");
        std::cout << "Erreur: Nom de canal invalide (contient des espaces): " << channel << std::endl;
        return;
    }
    if (channel[0] != '#') {
        channel = "#" + channel;
    }

    std::map<std::string, Channel>::iterator it = channels.find(channel);
    if (it == channels.end()) {
        channels.insert(std::make_pair(channel, Channel(channel)));
        std::cout << "Nouveau canal créé: " << channel << std::endl;
        it = channels.find(channel);
    }

    // Vérifie si le canal a une clé définie et si la clé fournie correspond
    if (!it->second.getKey().empty() && it->second.getKey() != key) {
        sendToClient(clientSocket, "ERROR :Incorrect channel key\r\n");
        std::cout << "Erreur: Clé incorrecte pour le canal " << channel << std::endl;
        return;
    }

    // Vérifie si le canal est en mode invitation uniquement et si l'utilisateur est invité
    if (it->second.isInviteOnly() && !it->second.isInvited(clients[clientSocket].getNickname())) {
        sendToClient(clientSocket, "ERROR :Cannot join channel (+i)\r\n");
        std::cout << "Erreur: Canal sur invitation seulement et utilisateur non invité pour le canal " << channel << std::endl;
        return;
    }

    std::string memberNick = clients[clientSocket].getNickname();
    if (!memberNick.empty()) {
        it->second.addUser(memberNick);
        if (it->second.hasUser(memberNick)) {
            clients[clientSocket].joinChannel(channel);
            std::cout << "Client " << memberNick << " a rejoint le canal " << channel << std::endl;

            std::string response = ":" + clients[clientSocket].getNickname() + "!" + clients[clientSocket].getNickname() + "@" + "localhost" + " JOIN " + channel + "\r\n";
            send(clientSocket, response.c_str(), response.size(), 0);

            response = ":server 353 " + clients[clientSocket].getNickname() + " = " + channel + " :" + memberNick + "\r\n";
            send(clientSocket, response.c_str(), response.size(), 0);

            response = ":server 366 " + clients[clientSocket].getNickname() + " " + channel + " :End of NAMES list\r\n";
            send(clientSocket, response.c_str(), response.size(), 0);

            sendWelcomeMessage(clientSocket);
        }
    } else {
        std::cout << "Erreur: Pseudonyme non défini pour le client " << clientSocket << std::endl;
    }
    Channel::displayChannels(channels);
}

// Fonction principale pour traiter les commandes reçues d'un client
void handleCommand(int clientSocket, const std::string& command, std::map<int, Client>& clients, std::map<std::string, Channel>& channels, const std::string& serverPassword) {
    const size_t MAX_COMMAND_LENGTH = 512;

    // Vérification de la longueur de la commande
    if (command.length() > MAX_COMMAND_LENGTH) {
        sendToClient(clientSocket, "ERROR :Command too long\r\n");
        return;
    }

    std::cout << "Traitement de la commande: " << command << " du client " << clientSocket << std::endl;

    Client& client = clients[clientSocket];

    if (command.find("PASS ") == 0) {
        std::string pass = command.substr(5);
        trim(pass);
        if (pass == serverPassword) {
            client.authenticate();
            sendToClient(clientSocket, "Vous êtes maintenant authentifié\r\n");
            std::cout << "Client " << clientSocket << " authentifié avec succès." << std::endl;

            // Envoyer un message confirmant l'authentification avec le pseudonyme si déjà défini
            if (!client.getNickname().empty()) {
                std::string response = ":server NOTICE " + client.getNickname() + " :Vous êtes authentifié en tant que " + client.getNickname() + "\r\n";
                send(clientSocket, response.c_str(), response.size(), 0);
                std::cout << "Message de confirmation d'authentification envoyé au client " << clientSocket << std::endl;
            }

            client.setSentAuthError(false);
        } else {
            sendToClient(clientSocket, "ERROR :Mot de passe incorrect\r\n");
            close(clientSocket);
            clients.erase(clientSocket);
            std::cout << "Client " << clientSocket << " déconnecté à cause d'un mot de passe incorrect." << std::endl;
            return;
        }
        return;
    } else if (!client.isAuthenticated()) {
        std::cout << "Client " << clientSocket << " tente d'exécuter une commande sans s'authentifier." << std::endl;
        sendToClient(clientSocket, "ERROR :Vous devez d'abord vous authentifier avec PASS\r\n");
        return;
    }

    if (command.find("NICK ") == 0) {
        std::string newNick = command.substr(5);
        trim(newNick);

        if (newNick.empty()) {
            sendToClient(clientSocket, "ERROR :No nickname given\r\n");
            std::cout << "Erreur: Aucun pseudonyme fourni par le client " << clientSocket << std::endl;
        } else if (Client::containsInvalidCharacters(newNick)) {
            sendToClient(clientSocket, "ERROR :Erroneous nickname\r\n");
            std::cout << "Erreur: Pseudonyme erroné fourni par le client " << clientSocket << ": " << newNick << std::endl;
        } else if (Client::isNicknameAlreadyInUse(clients, newNick)) {
            sendToClient(clientSocket, "ERROR :Nickname is already in use\r\n");
            std::cout << "Erreur: Pseudonyme déjà utilisé: " << newNick << std::endl;
        } else {
            client.setNickname(newNick);
            std::cout << "Pseudo défini pour le client " << clientSocket << ": " << newNick << std::endl;
            sendToClient(clientSocket, "Nickname changed to " + newNick + "\r\n");

            // Envoyer un message confirmant le pseudonyme
            std::string response = ":server NOTICE " + newNick + " :Vous êtes maintenant connu sous le pseudonyme " + newNick + "\r\n";
            send(clientSocket, response.c_str(), response.size(), 0);
            std::cout << "Message de confirmation de pseudonyme envoyé au client " << clientSocket << std::endl;

            // Si le client est déjà authentifié, envoyer un message de confirmation d'authentification
            if (client.isAuthenticated()) {
                response = ":server NOTICE " + newNick + " :Vous êtes authentifié en tant que " + newNick + "\r\n";
                send(clientSocket, response.c_str(), response.size(), 0);
                std::cout << "Message de confirmation d'authentification envoyé au client " << clientSocket << std::endl;
            }
        }
    } else if (command.find("USER ") == 0) {
        std::string response = ":server 001 " + client.getNickname() + " :Welcome to the IRC server\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Message de bienvenue envoyé au client " << clientSocket << std::endl;

        // Envoyer un message confirmant le pseudonyme
        response = ":server NOTICE " + client.getNickname() + " :Vous êtes maintenant connu sous le pseudonyme " + client.getNickname() + "\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Message de confirmation de pseudonyme envoyé au client " << clientSocket << std::endl;

        // Si le client est déjà authentifié, envoyer un message de confirmation d'authentification
        if (client.isAuthenticated()) {
            response = ":server NOTICE " + client.getNickname() + " :Vous êtes authentifié en tant que " + client.getNickname() + "\r\n";
            send(clientSocket, response.c_str(), response.size(), 0);
            std::cout << "Message de confirmation d'authentification envoyé au client " << clientSocket << std::endl;
        }
    } else if (command.find("JOIN ") == 0) {
        size_t spacePos = command.find(' ');
        size_t secondSpacePos = command.find(' ', spacePos + 1);
        std::string channelName;
        std::string key;

        if (spacePos != std::string::npos) {
            channelName = command.substr(spacePos + 1, secondSpacePos - (spacePos + 1));
            if (secondSpacePos != std::string::npos) {
                key = command.substr(secondSpacePos + 1);
            }
        }

        handleJoinCommand(clientSocket, channelName, key, clients, channels);
    } else if (command.find("PART ") == 0) {
        std::string channelName = command.substr(5);
        trim(channelName);
        if (channelName[0] != '#') {
            channelName = "#" + channelName;
        }
        channels[channelName].removeUser(client.getNickname());
        client.leaveChannel(channelName);
        std::cout << "Client " << client.getNickname() << " a quitté le canal " << channelName << std::endl;
        Channel::displayChannels(channels);
    } else if (command.find("PRIVMSG ") == 0) {
        handlePrivmsgCommand(clientSocket, command, clients, channels);
    } else if (command.find("PING ") == 0) {
        handlePing(clientSocket, command.substr(5));
    } else if (command.find("PONG ") == 0) {
        handlePong(clientSocket);
    } else if (command.find("KICK ") == 0) {
        handleKickCommand(clientSocket, command, clients, channels);
    } else if (command.find("INVITE ") == 0) {
        handleInviteCommand(clientSocket, command, clients, channels);
    } else if (command.find("TOPIC ") == 0) {
        handleTopicCommand(clientSocket, command, clients, channels);
    } else if (command.find("MODE ") == 0) {
        handleModeCommand(clientSocket, command, clients, channels);
    } else if (command.find("LIST") == 0) {
        handleListCommand(clientSocket, channels);
    } else if (command.find("NAMES ") == 0) {
        handleNamesCommand(clientSocket, command.substr(6), channels, client);
    } else if (command.find("WHOIS ") == 0) {
        handleWhoisCommand(clientSocket, command.substr(6), clients);
    } else {
        std::string response = ":server 421 " + command + " :Unknown command\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        std::cout << "Commande inconnue reçue: " << command << std::endl;
    }
}

/*
Explications détaillées :
- **handlePing** : Traite une commande PING en envoyant une réponse PONG au client.
- **handlePong** : Traite une commande PONG en mettant à jour l'heure du dernier PONG du client.
- **handleKickCommand** : Traite la commande KICK pour expulser un utilisateur d'un canal.
- **handleInviteCommand** : Traite la commande INVITE pour inviter un utilisateur à rejoindre un canal.
- **handleTopicCommand** : Traite la commande TOPIC pour définir ou obtenir le sujet d'un canal.
- **handleModeCommand** : Traite la commande MODE pour définir ou obtenir les modes d'un canal.
- **handleListCommand** : Traite la commande LIST pour renvoyer la liste des canaux actifs.
- **handleNamesCommand** : Traite la commande NAMES pour renvoyer la liste des utilisateurs dans un canal.
- **handleWhoisCommand** : Traite la commande WHOIS pour renvoyer des informations sur un utilisateur spécifique.
- **handlePrivmsgCommand** : Traite la commande PRIVMSG pour envoyer un message privé à un utilisateur ou à un canal.
- **sendWelcomeMessage** : Envoie un message de bienvenue à un client lorsqu'il se connecte.
- **handleJoinCommand** : Traite la commande JOIN pour permettre à un utilisateur de rejoindre un canal.
- **handleCommand** : Fonction principale pour traiter les commandes reçues d'un client. Elle décode la commande et appelle la fonction appropriée pour la traiter.

Définitions des termes importants :
- **Socket** : Un point de communication entre deux machines pour envoyer et recevoir des données.
- **Ping/Pong** : Mécanisme utilisé pour vérifier si un client est toujours connecté. Le serveur envoie un PING et attend une réponse PONG du client.
- **Canal** : Une salle de discussion où plusieurs utilisateurs peuvent envoyer et recevoir des messages.
- **Sujet** : Le sujet de discussion du canal, visible par tous les utilisateurs du canal.
- **Mode d'invitation uniquement** : Mode où seuls les utilisateurs invités peuvent rejoindre le canal.
- **Protection du sujet** : Mode où seuls les opérateurs peuvent changer le sujet du canal.
- **Clé du canal** : Mot de passe requis pour rejoindre le canal.
- **Opérateur** : Utilisateur ayant des privilèges spéciaux dans le canal, comme la capacité de changer le sujet ou de bannir des utilisateurs.
- **Diffusion** : Envoi d'un message à tous les utilisateurs du canal.
*/


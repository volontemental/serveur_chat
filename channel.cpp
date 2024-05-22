/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:45:12 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:45:15 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "channel.hpp"
#include <algorithm>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <cerrno>

// Constructeur par défaut, initialise le canal avec des paramètres par défaut
Channel::Channel() : inviteOnly(false), topicRestricted(false), userLimit(0) {
    std::cout << "Canal créé avec les paramètres par défaut." << std::endl;
}

// Constructeur avec nom, initialise le canal avec le nom spécifié
Channel::Channel(const std::string& name) : name(name), inviteOnly(false), topicRestricted(false), userLimit(0) {
    std::cout << "Canal " << name << " créé." << std::endl;
}

// Ajoute un utilisateur au canal
void Channel::addUser(const std::string& user) {
    users.insert(user);
    std::cout << "Utilisateur " << user << " ajouté au canal " << name << "." << std::endl;
}

// Retire un utilisateur du canal
void Channel::removeUser(const std::string& user) {
    users.erase(user);
    operators.erase(user); // Supprime également l'utilisateur de la liste des opérateurs s'il en fait partie
    std::cout << "Utilisateur " << user << " retiré du canal " << name << "." << std::endl;
}

// Vérifie si un utilisateur est dans le canal
bool Channel::hasUser(const std::string& user) const {
    return users.find(user) != users.end();
}

// Retourne la liste des utilisateurs du canal
std::set<std::string> Channel::listUsers() const {
    return users;
}

// Retourne le nombre d'utilisateurs dans le canal
size_t Channel::userCount() const {
    return users.size();
}

// Définit le sujet du canal
void Channel::setTopic(const std::string& topic) {
    this->topic = topic;
    std::cout << "Sujet du canal " << name << " mis à jour: " << topic << "." << std::endl;
}

// Retourne le sujet du canal
std::string Channel::getTopic() const {
    return topic;
}

// Définit un mode pour le canal (comme inviteOnly ou topicRestricted)
void Channel::setMode(const std::string& mode, bool add) {
    if (mode == "i") {
        inviteOnly = add;
        std::cout << "Mode inviteOnly " << (add ? "activé" : "désactivé") << " pour le canal " << name << "." << std::endl;
    } else if (mode == "t") {
        topicRestricted = add;
        std::cout << "Mode topicRestricted " << (add ? "activé" : "désactivé") << " pour le canal " << name << "." << std::endl;
    } else {
        std::cout << "Mode inconnu: " << mode << " pour le canal " << name << "." << std::endl;
    }
}

// Retourne les modes activés pour le canal
std::string Channel::getMode() const {
    std::string mode;
    if (inviteOnly) mode += "i";
    if (topicRestricted) mode += "t";
    return mode;
}

// Définit le mode d'invitation uniquement pour le canal
void Channel::setInviteOnly(bool inviteOnly) {
    this->inviteOnly = inviteOnly;
    std::cout << "Mode inviteOnly " << (inviteOnly ? "activé" : "désactivé") << " pour le canal " << name << "." << std::endl;
}

// Vérifie si le canal est en mode d'invitation uniquement
bool Channel::isInviteOnly() const {
    return inviteOnly;
}

// Ajoute un utilisateur en tant qu'opérateur du canal
void Channel::addOperator(const std::string& user) {
    if (hasUser(user)) {
        operators.insert(user);
        std::cout << "Utilisateur " << user << " ajouté en tant qu'opérateur du canal " << name << "." << std::endl;
    }
}

// Retire un utilisateur de la liste des opérateurs du canal
void Channel::removeOperator(const std::string& user) {
    operators.erase(user);
    std::cout << "Utilisateur " << user << " retiré de la liste des opérateurs du canal " << name << "." << std::endl;
}

// Vérifie si un utilisateur est opérateur du canal
bool Channel::isOperator(const std::string& user) const {
    return operators.find(user) != operators.end();
}

// Définit la clé du canal
void Channel::setKey(const std::string& key) {
    this->key = key;
    std::cout << "Clé du canal " << name << " définie à: " << key << "." << std::endl;
}

// Retourne la clé du canal
std::string Channel::getKey() const {
    return key;
}

// Vérifie si une clé fournie correspond à la clé du canal
bool Channel::checkKey(const std::string& key) const {
    return this->key == key;
}

// Définit la limite d'utilisateurs pour le canal
void Channel::setUserLimit(size_t limit) {
    this->userLimit = limit;
    std::cout << "Limite d'utilisateurs pour le canal " << name << " définie à: " << limit << "." << std::endl;
}

// Retourne la limite d'utilisateurs du canal
size_t Channel::getUserLimit() const {
    return userLimit;
}

// Retourne le nom du canal
std::string Channel::getName() const {
    return name;
}

// Diffuse un message à tous les utilisateurs du canal
void Channel::broadcast(const std::string& message, std::map<int, Client>& clients, int senderSocket) {
    std::cout << "Diffusion du message dans le canal " << name << " : " << message << std::endl;
    for (std::set<std::string>::const_iterator it = users.begin(); it != users.end(); ++it) {
        std::map<int, Client>::iterator clientIt = std::find_if(clients.begin(), clients.end(), NicknameEquals(*it));
        if (clientIt != clients.end() && clientIt->first != senderSocket) {
            if (send(clientIt->first, message.c_str(), message.size(), 0) < 0) {
                std::cerr << "Erreur lors de l'envoi du message au client " << clientIt->first << ": " << strerror(errno) << std::endl;
            } else {
                std::cout << "Message envoyé à " << *it << " (Socket: " << clientIt->first << ")" << std::endl;
            }
        }
    }
}

// Ajoute un utilisateur à la liste des invités du canal
void Channel::addInvitedUser(const std::string& user) {
    invitedUsers.insert(user);
    std::cout << "Utilisateur " << user << " ajouté à la liste des invités du canal " << name << "." << std::endl;
}

// Vérifie si un utilisateur est invité dans le canal
bool Channel::isInvited(const std::string& user) const {
    return invitedUsers.find(user) != invitedUsers.end();
}

// Définit la protection du sujet pour le canal
void Channel::setTopicProtection(bool add) {
    topicRestricted = add;
    std::cout << "Protection du sujet " << (add ? "activée" : "désactivée") << " pour le canal " << name << "." << std::endl;
}

// Vérifie si le sujet du canal est protégé
bool Channel::isTopicProtected() const {
    return topicRestricted;
}

// Ajoute un utilisateur au canal avec vérifications supplémentaires
void Channel::addUserToChannel(const std::string& user) {
    std::cout << "Tentative d'ajout de l'utilisateur " << user << " au canal " << getName() << std::endl;
    if (userCount() >= getUserLimit() && getUserLimit() > 0) {
        std::cout << "Erreur : limite d'utilisateurs atteinte pour le canal " << getName() << std::endl;
        return;
    }
    if (!hasUser(user)) {
        addUser(user);
        std::cout << "Utilisateur " << user << " ajouté au canal " << getName() << std::endl;
    } else {
        std::cout << "Utilisateur " << user << " est déjà dans le canal " << getName() << std::endl;
    }
}

// Retire un utilisateur du canal avec vérifications supplémentaires
void Channel::removeUserFromChannel(const std::string& user) {
    std::cout << "Tentative de retrait de l'utilisateur " << user << " du canal " << getName() << std::endl;
    if (hasUser(user)) {
        removeUser(user);
        std::cout << "Utilisateur " << user << " retiré du canal " << getName() << std::endl;
    } else {
        std::cout << "Utilisateur " << user << " n'est pas dans le canal " << getName() << std::endl;
    }
}

// Affiche la liste des canaux actifs et leurs membres
void Channel::displayChannels(const std::map<std::string, Channel>& channels) {
    std::cout << "Canaux actuellement actifs:" << std::endl;
    for (std::map<std::string, Channel>::const_iterator channelPair = channels.begin(); channelPair != channels.end(); ++channelPair) {
        std::cout << "Canal: " << channelPair->first << " | Membres: ";
        std::set<std::string> users = channelPair->second.listUsers();
        for (std::set<std::string>::const_iterator member = users.begin(); member != users.end(); ++member) {
            std::cout << *member << " ";
        }
        std::cout << std::endl;
    }
}

/*
Explications détaillées :
- **Channel** : Classe représentant un canal de discussion IRC. Elle gère les informations et les actions associées à chaque canal.
- **Canal** : Une salle de discussion où plusieurs utilisateurs peuvent envoyer et recevoir des messages.
- **Sujet** : Le sujet de discussion du canal, visible par tous les utilisateurs du canal.
- **Mode d'invitation uniquement** : Mode où seuls les utilisateurs invités peuvent rejoindre le canal.
- **Protection du sujet** : Mode où seuls les opérateurs peuvent changer le sujet du canal.
- **Clé du canal** : Mot de passe requis pour rejoindre le canal.
- **Opérateur** : Utilisateur ayant des privilèges spéciaux dans le canal, comme la capacité de changer le sujet ou de bannir des utilisateurs.
- **Diffusion** : Envoi d'un message à tous les utilisateurs du canal.
- **Méthodes statiques** : Méthodes qui appartiennent à la classe plutôt qu'à une instance spécifique de la classe.
*/


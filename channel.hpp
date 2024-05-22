/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:47:25 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:47:27 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include "client.hpp"

// Déclaration de la classe Channel qui représente un canal de discussion IRC
class Channel {
public:
    // Constructeurs
    Channel();
    Channel(const std::string& name);

    // Méthodes pour ajouter et retirer des utilisateurs du canal
    void addUser(const std::string& user);
    void removeUser(const std::string& user);
    bool hasUser(const std::string& user) const;
    std::set<std::string> listUsers() const;
    size_t userCount() const;

    // Méthodes pour gérer le sujet du canal
    void setTopic(const std::string& topic);
    std::string getTopic() const;

    // Méthodes pour gérer les modes du canal
    void setMode(const std::string& mode, bool add);
    std::string getMode() const;
    void setInviteOnly(bool inviteOnly);
    bool isInviteOnly() const;

    // Méthodes pour gérer les opérateurs du canal
    void addOperator(const std::string& user);
    void removeOperator(const std::string& user);
    bool isOperator(const std::string& user) const;

    // Méthodes pour gérer la clé et la limite d'utilisateurs du canal
    void setKey(const std::string& key);
    std::string getKey() const;
    bool checkKey(const std::string& key) const;
    void setUserLimit(size_t limit);
    size_t getUserLimit() const;

    // Méthode pour obtenir le nom du canal
    std::string getName() const;

    // Méthode pour diffuser un message à tous les utilisateurs du canal
    void broadcast(const std::string& message, std::map<int, Client>& clients, int senderSocket);

    // Méthodes pour gérer les utilisateurs invités
    void addInvitedUser(const std::string& user);
    bool isInvited(const std::string& user) const;

    // Méthodes pour gérer la protection du sujet
    void setTopicProtection(bool add);
    bool isTopicProtected() const;

    // Méthodes pour ajouter et retirer des utilisateurs du canal
    void addUserToChannel(const std::string& user);
    void removeUserFromChannel(const std::string& user);

    // Méthode statique pour afficher les canaux actifs
    static void displayChannels(const std::map<std::string, Channel>& channels);

private:
    std::string name; // Nom du canal
    std::set<std::string> users; // Ensemble des utilisateurs du canal
    std::set<std::string> operators; // Ensemble des opérateurs du canal
    std::set<std::string> invitedUsers; // Ensemble des utilisateurs invités au canal
    std::string topic; // Sujet du canal
    std::string key; // Clé du canal (si le canal est protégé par une clé)
    bool inviteOnly; // Indicateur de mode d'invitation uniquement
    bool topicRestricted; // Indicateur de restriction de modification du sujet
    size_t userLimit; // Limite d'utilisateurs dans le canal
};

#endif // CHANNEL_HPP

/*
Explications détaillées :
- **Channel** : Classe représentant un canal de discussion IRC. Elle gère les informations et les actions associées à chaque canal.
- **Canal** : Une salle de discussion où plusieurs utilisateurs peuvent envoyer et recevoir des messages.
- **Sujet** : Le sujet de discussion du canal, visible par tous les utilisateurs du canal.
- **Mode d'invitation uniquement** : Mode où seuls les utilisateurs invités peuvent rejoindre le canal.
- **Protection du sujet** : Mode où seuls les opérateurs peuvent changer le sujet du canal.
- **Clé du canal** : Mot de passe requis pour rejoindre le canal.
- **Opérateur** : Utilisateur ayant des privilèges spéciaux dans le canal, comme la capacité de changer le sujet ou de bannir des utilisateurs.
*/


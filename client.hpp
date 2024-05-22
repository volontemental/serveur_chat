/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:47:10 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:47:12 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include <map>

// Déclaration de la classe Client qui représente un client connecté au serveur IRC
class Client {
public:
    // Constructeurs
    Client();
    Client(int socket, const std::string& nickname);

    // Méthodes pour obtenir et définir le pseudonyme du client
    std::string getNickname() const;
    void setNickname(const std::string& nickname);

    // Méthode pour obtenir le descripteur de fichier du socket du client
    int getSocket() const;

    // Méthodes pour gérer l'authentification du client
    void authenticate();
    bool isAuthenticated() const;

    // Méthodes pour gérer les canaux que le client a rejoint
    void joinChannel(const std::string& channelName);
    void leaveChannel(const std::string& channelName);
    bool isInChannel(const std::string& channelName) const;
    std::set<std::string> getChannels() const;

    // Méthodes pour gérer le temps du dernier PONG reçu
    void updateLastPong();
    time_t getLastPong() const;

    // Méthodes pour gérer le buffer de commande
    std::string& getCommandBuffer();
    void appendToCommandBuffer(const std::string& fragment);
    void clearCommandBuffer();

    // Méthodes pour gérer les erreurs d'authentification
    bool hasSentAuthError() const;
    void setSentAuthError(bool value);

    // Méthodes statiques pour vérifier les pseudonymes
    static bool isNicknameAlreadyInUse(const std::map<int, Client>& clients, const std::string& nickname);
    static bool containsInvalidCharacters(const std::string& nickname);

private:
    int _socket; // Descripteur de fichier du socket du client
    std::string _nickname; // Pseudonyme du client
    bool _authenticated; // Indicateur d'authentification du client
    time_t _lastPong; // Temps du dernier PONG reçu du client
    std::string _commandBuffer; // Buffer de commande pour stocker les commandes partielles
    bool _sentAuthError; // Indicateur d'erreur d'authentification envoyée
    std::set<std::string> _channels; // Ensemble des canaux que le client a rejoint
};

// Structure utilitaire pour comparer les pseudonymes des clients
struct NicknameEquals {
    NicknameEquals(const std::string& nickname) : _nickname(nickname) {}
    bool operator()(const std::pair<const int, Client>& client) const {
        return client.second.getNickname() == _nickname;
    }
private:
    std::string _nickname;
};

#endif // CLIENT_HPP

/*
Explications détaillées :
- **Client** : Classe représentant un client connecté au serveur IRC. Elle gère les informations et les actions associées à chaque client.
- **Socket** : Un point de communication entre deux machines pour envoyer et recevoir des données.
- **Pseudonyme** : Nom unique utilisé par le client pour s'identifier sur le serveur IRC.
- **Authentification** : Processus par lequel le client prouve son identité au serveur.
- **Buffer de commande** : Zone de stockage temporaire pour les commandes partielles reçues du client.
- **PONG** : Réponse à un PING envoyé par le serveur pour vérifier si le client est toujours connecté.
- **Méthodes statiques** : Méthodes qui appartiennent à la classe plutôt qu'à une instance spécifique de la classe.
*/


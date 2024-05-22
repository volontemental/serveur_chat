/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:45:58 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:46:00 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"
#include <ctime>
#include <iostream>
#include <algorithm>

// Constructeur par défaut, initialise le socket à -1 et l'heure du dernier PONG à l'heure actuelle
Client::Client()
    : _socket(-1), _nickname(""), _authenticated(false), _lastPong(std::time(NULL)), _sentAuthError(false) {
    std::cout << "Client par défaut créé, socket: " << _socket << std::endl;
}

// Constructeur avec paramètres, initialise le socket, le pseudonyme et l'heure du dernier PONG à l'heure actuelle
Client::Client(int socket, const std::string& nickname)
    : _socket(socket), _nickname(nickname), _authenticated(false), _lastPong(std::time(NULL)), _sentAuthError(false) {
    std::cout << "Client créé avec socket: " << _socket << " et nickname: " << _nickname << std::endl;
}

// Retourne le pseudonyme du client
std::string Client::getNickname() const {
    return _nickname;
}

// Définit un nouveau pseudonyme pour le client, vérifie s'il est valide
void Client::setNickname(const std::string& nickname) {
    if (nickname.empty() || containsInvalidCharacters(nickname) || nickname == "Welcome to the IRC server!") {
        std::cerr << "Erreur : Pseudonyme invalide fourni." << std::endl;
        return;
    }
    _nickname = nickname;
    std::cout << "Nickname mis à jour pour le client avec socket: " << _socket << " à: " << _nickname << std::endl;
}

// Retourne le descripteur de fichier du socket du client
int Client::getSocket() const {
    return _socket;
}

// Authentifie le client et réinitialise le statut d'erreur d'authentification
void Client::authenticate() {
    _authenticated = true;
    _sentAuthError = false; // Réinitialise le statut d'erreur après une authentification réussie
    std::cout << "Client avec socket: " << _socket << " authentifié." << std::endl;
}

// Vérifie si le client est authentifié
bool Client::isAuthenticated() const {
    return _authenticated;
}

// Ajoute le client à un canal
void Client::joinChannel(const std::string& channelName) {
    _channels.insert(channelName);
    std::cout << "Client avec socket: " << _socket << " a rejoint le canal: " << channelName << std::endl;
}

// Retire le client d'un canal
void Client::leaveChannel(const std::string& channelName) {
    _channels.erase(channelName);
    std::cout << "Client avec socket: " << _socket << " a quitté le canal: " << channelName << std::endl;
}

// Vérifie si le client est dans un canal
bool Client::isInChannel(const std::string& channelName) const {
    return _channels.find(channelName) != _channels.end();
}

// Retourne l'ensemble des canaux que le client a rejoint
std::set<std::string> Client::getChannels() const {
    return _channels;
}

// Met à jour l'heure du dernier PONG reçu à l'heure actuelle
void Client::updateLastPong() {
    _lastPong = std::time(NULL);
    std::cout << "Dernier PONG mis à jour pour le client avec socket: " << _socket << std::endl;
}

// Retourne l'heure du dernier PONG reçu
time_t Client::getLastPong() const {
    return _lastPong;
}

// Retourne le buffer de commande
std::string& Client::getCommandBuffer() {
    return _commandBuffer;
}

// Ajoute un fragment de commande au buffer
void Client::appendToCommandBuffer(const std::string& fragment) {
    _commandBuffer += fragment;
    std::cout << "Buffer de commande mis à jour pour le client avec socket: " << _socket << ", contenu actuel: " << _commandBuffer << std::endl;
}

// Efface le buffer de commande
void Client::clearCommandBuffer() {
    _commandBuffer.clear();
    std::cout << "Buffer de commande effacé pour le client avec socket: " << _socket << std::endl;
}

// Vérifie si une erreur d'authentification a été envoyée au client
bool Client::hasSentAuthError() const {
    return _sentAuthError;
}

// Définit le statut d'erreur d'authentification envoyée
void Client::setSentAuthError(bool value) {
    _sentAuthError = value;
    std::cout << "Statut d'erreur d'authentification mis à jour pour le client avec socket: " << _socket << " à: " << _sentAuthError << std::endl;
}

// Vérifie si un pseudonyme est déjà utilisé par un autre client
bool Client::isNicknameAlreadyInUse(const std::map<int, Client>& clients, const std::string& nickname) {
    std::cout << "Vérification de l'existence du pseudonyme: " << nickname << std::endl;
    std::map<int, Client>::const_iterator it = std::find_if(clients.begin(), clients.end(), NicknameEquals(nickname));
    if (it != clients.end()) {
        std::cout << "Le client avec le pseudonyme '" << nickname << "' a été trouvé. Socket: " << it->first << std::endl;
        return true;
    } else {
        std::cout << "Le client avec le pseudonyme '" << nickname << "' n'a pas été trouvé." << std::endl;
        return false;
    }
}

// Vérifie si un pseudonyme contient des caractères invalides
bool Client::containsInvalidCharacters(const std::string& nickname) {
    std::cout << "Vérification des caractères invalides dans le pseudonyme: " << nickname << std::endl;
    std::string invalidChars = " ,*?!@."; 
    if (nickname.empty()) return true;
    if (nickname[0] == '$' || nickname[0] == ':' || nickname[0] == '#') {
        return true; 
    }
    for (size_t i = 0; i < nickname.size(); i++) {
        if (invalidChars.find(nickname[i]) != std::string::npos) {
            return true; 
        }
    }
    return false;
}

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


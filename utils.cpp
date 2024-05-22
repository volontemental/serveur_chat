/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:46:33 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:46:34 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"
#include <iostream>
#include <cstring> // Pour strerror
#include <cerrno>  // Pour errno
#include <sys/socket.h> // Pour send
#include <algorithm> // Pour std::find_if

// Envoie un message à un client via son socket
int sendToClient(int clientSocket, const std::string& message) {
    ssize_t sent = send(clientSocket, message.c_str(), message.length(), 0);
    if (sent == -1) {
        std::cerr << "Erreur : Échec de l'envoi du message au client " << clientSocket << ": " << strerror(errno) << std::endl;
        return -1;
    }
    std::cout << "Message envoyé au client " << clientSocket << ": " << message << std::endl;
    return 0;
}

// Vérifie si un caractère n'est pas un espace
bool isNotSpace(unsigned char ch) {
    return !std::isspace(ch);
}

// Supprime les espaces en début de chaîne de caractères
void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), isNotSpace));
}

// Supprime les espaces en fin de chaîne de caractères
void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), isNotSpace).base(), s.end());
}

// Supprime les espaces en début et en fin de chaîne de caractères
void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// Vérifie si un pseudonyme contient des caractères invalides
bool containsInvalidCharacters(const std::string& nickname) {
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
- **sendToClient** : Envoie un message à un client via son socket. Utilise la fonction `send` pour envoyer les données et gère les erreurs potentielles.
- **isNotSpace** : Vérifie si un caractère donné n'est pas un espace en utilisant la fonction `isspace`.
- **ltrim** : Supprime les espaces au début d'une chaîne de caractères en utilisant `std::find_if` pour trouver le premier caractère non espace.
- **rtrim** : Supprime les espaces à la fin d'une chaîne de caractères en utilisant `std::find_if` et `rbegin`/`rend` pour parcourir la chaîne à l'envers.
- **trim** : Combine `ltrim` et `rtrim` pour supprimer les espaces des deux côtés d'une chaîne de caractères.
- **containsInvalidCharacters** : Vérifie si un pseudonyme contient des caractères invalides en utilisant une liste de caractères non autorisés et en s'assurant que le pseudonyme ne commence pas par des caractères spéciaux.
*/


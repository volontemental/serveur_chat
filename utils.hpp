/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cvu <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 02:46:43 by cvu               #+#    #+#             */
/*   Updated: 2024/05/22 02:46:45 by cvu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

// Fonction pour envoyer un message à un client via son socket
int sendToClient(int clientSocket, const std::string& message);

// Fonctions pour trimmer les espaces dans les chaînes de caractères
bool isNotSpace(unsigned char ch);
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);

// Fonction pour vérifier si un pseudonyme contient des caractères invalides
bool containsInvalidCharacters(const std::string& nickname);

#endif // UTILS_HPP

/*
Explications détaillées :
- **sendToClient** : Fonction qui envoie un message à un client via son socket.
- **Trimming** : Suppression des espaces en début et en fin de chaîne de caractères.
- **isNotSpace** : Fonction qui vérifie si un caractère n'est pas un espace.
- **ltrim, rtrim, trim** : Fonctions pour trimmer les espaces à gauche, à droite et des deux côtés d'une chaîne de caractères.
- **containsInvalidCharacters** : Fonction qui vérifie si un pseudonyme contient des caractères non valides.
*/


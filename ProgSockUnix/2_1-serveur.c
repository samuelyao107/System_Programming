/*
 * Auteur(s): Samuel YAO
 * Description : Serveur itératif utilisant des sockets UNIX locaux (AF_UNIX) 
 * en mode SOCK_SEQPACKET (orienté connexion, avec préservation 
 * des limites de messages).
 */

#define _GNU_SOURCE  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/un.h>

// Le chemin sur le système de fichiers qui servira de point de rencontre
#define SOCKET_PATH "/tmp/seqpacket.sock"
// La taille du tampon de lecture
#define BUFSIZE 512

int main(int argc, char **argv) {
    int server_fd, client_fd;
    socklen_t fromlen;
    
    // sockaddr_storage est une structure générique assez grande pour 
    // contenir n'importe quel type d'adresse (IPv4, IPv6, UNIX...)
    struct sockaddr_storage from;
    
    // sockaddr_un est la structure spécifique pour les adresses locales (AF_UNIX)
    struct sockaddr_un name;
    
    char buf[BUFSIZE];
    ssize_t nread, nwrite;
    
    char *message = "Message a envoyer: ";

    // 1. CRÉATION DU SOCKET
    // AF_UNIX : Communication locale sur la même machine.
    // SOCK_SEQPACKET : Fiable, bidirectionnel, et préserve les limites des messages.
    server_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (server_fd == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // 2. PRÉPARATION DE L'ADRESSE
    // On met la structure à zéro par sécurité, puis on définit la famille et le chemin
    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_PATH, sizeof(name.sun_path) - 1);

    // 3. NETTOYAGE PRÉALABLE
    // Si le serveur a planté précédemment, le fichier socket existe peut-être encore.
    // Le supprimer évite l'erreur "Address already in use" lors du bind.
    unlink(SOCKET_PATH);

    // 4. ATTACHEMENT (BIND)
    // On associe notre socket (server_fd) au fichier physique (/tmp/seqpacket.sock)
    if (bind(server_fd, (const struct sockaddr *) &name, sizeof(name)) == -1) {
        perror("Erreur lors du bind");
        exit(EXIT_FAILURE);
    }

    // 5. MISE EN ÉCOUTE (LISTEN)
    // On indique au système d'accepter les connexions entrantes. 
    // '5' est la taille de la file d'attente pour les connexions en instance.
    if (listen(server_fd, 5) == -1) {
        perror("Erreur lors du listen");
        exit(EXIT_FAILURE);
    }
    printf("Serveur en attente de connexion sur %s...\n", SOCKET_PATH);
  
    // BOUCLE PRINCIPALE DU SERVEUR (Serveur itératif : traite un client à la fois)
    for (;;) {
        
        // 6. ACCEPTATION D'UN CLIENT
        // Le programme se bloque ici jusqu'à ce qu'un client se connecte.
        fromlen = sizeof(from);
        client_fd = accept(server_fd, (struct sockaddr *)&from, &fromlen);
        if (client_fd == -1) {
            perror("Erreur lors de l'acceptation (accept)");
            exit(EXIT_FAILURE);
        }

        printf("Nouveau client connecté!\n");

        // BOUCLE DE COMMUNICATION AVEC LE CLIENT ACTUEL
        for (;;) {
            
            // On envoie une requête/un prompt au client
            nwrite = write(client_fd, message, strlen(message));
            if (nwrite < 0) {
                perror("Erreur d'écriture (write)");
                close(client_fd);
                break; // On sort de la boucle interne pour passer au client suivant
            }
            
            // On attend la réponse du client. 
            // On lit au maximum BUFSIZE-1 pour garder de la place pour le caractère de fin de chaîne '\0'
            nread = read(client_fd, buf, BUFSIZE - 1);
            
            if (nread == 0) {
                // read() renvoie 0 si le client a fermé la connexion proprement (EOF)
                printf("Fin de la communication avec le client.\n");
                close(client_fd);
                break;
            } else if (nread < 0) {
                // read() renvoie une valeur négative en cas d'erreur
                perror("Erreur de lecture (read)");
                close(client_fd);
                break;
            }
            
            // On ajoute le caractère de fin de chaîne pour pouvoir utiliser printf("%s") en toute sécurité
            buf[nread] = '\0';
            printf("Message recu : '%s'\n", buf);
        }
    }
}
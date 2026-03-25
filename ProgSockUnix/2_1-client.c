/*
 * Auteur(s): Samuel YAO
 */
#define _GNU_SOURCE  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/seqpacket.sock"
#define BUFSIZE 512

int main(int argc, char **argv) {
    int sock_fd;
    struct sockaddr_un name;
    char buf[BUFSIZE], *pt;
    ssize_t nread, nwrite;

    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sock_fd == -1) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_PATH, sizeof(name.sun_path) - 1);

    if (connect(sock_fd, (const struct sockaddr *) &name, sizeof(name)) == -1) {
        perror("Erreur de connexion au serveur (est-il démarré ?)");
        close(sock_fd); // Bonne pratique : fermer avant de quitter
        exit(EXIT_FAILURE);
    }
    printf("Connecté au serveur avec succès.\n");

    /* Boucle de communication */
    for (;;) {
        /* Lecture socket */
        nread = read(sock_fd, buf, BUFSIZE - 1);
        if (nread == 0) {
            printf("\nConnexion rompue par le serveur\n");
            close(sock_fd); // Ajouté ici
            exit(EXIT_SUCCESS);
        } else if (nread < 0) {
            perror("read");
            close(sock_fd); // Ajouté ici
            exit(EXIT_FAILURE);
        }
        buf[nread] = '\0';
        
        /* Affichage ecran du message lu sur la socket */
        printf("Message recu '%s'", buf);
        fflush(stdout); /* Force l'affichage immédiat du texte à l'écran */

        /* Lecture clavier. Si on tape <Control-D>, fgets() rend NULL
         * <Control-D> symbolise la fin de fichier, ici la terminaison
         * du client */
        pt = fgets(buf, BUFSIZE, stdin);
        if (pt == NULL) {
            printf("\nSortie du client\n");
            close(sock_fd);
            exit(EXIT_SUCCESS);
        }
        
        nwrite = write(sock_fd, buf, strlen(buf));
        if (nwrite < 0) {
            perror("write");
            close(sock_fd); // Ajouté ici
            exit(EXIT_FAILURE);
        }
    }
    
    return 0; // Bonne pratique à la fin du main (bien que jamais atteint ici)
}
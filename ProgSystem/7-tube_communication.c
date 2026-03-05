/* 
 * Auteur(s): Samuel YAO
*/

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int fd[2];      // fd[0] = lecture, fd[1] = écriture
    pid_t pid;
    char buf[256];
    int n;

    /*Création du tube */
    if (pipe(fd) < 0) {
        perror("Erreur lors de la création du tube");
        exit(EXIT_FAILURE);
    }

    /*Création du fils */
    pid = fork();
    if (pid < 0) {
        perror("Erreur lors de la création du fils");
        exit(EXIT_FAILURE);
    }

    /* -------------------------------------------------- */
    /*                      FILS                          */
    /* -------------------------------------------------- */
    if (pid == 0) {

        /* Rediriger stdin → fd[0] (lecture du tube) */
        dup2(fd[0], STDIN_FILENO);

        /* Fermer les descripteurs inutiles */
        close(fd[0]);
        close(fd[1]);

        /* Lire stdin (= le tube) et afficher */
        while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, "[FILS reçoit] : ", 16);
            write(STDOUT_FILENO, buf, n);
        }

        exit(EXIT_SUCCESS);
    }

    /* -------------------------------------------------- */
    /*                      PÈRE                          */
    /* -------------------------------------------------- */

    /* Rediriger stdout → fd[1] (écriture dans le tube) */
    dup2(fd[1], STDOUT_FILENO);

    /* Fermer les descripteurs inutiles */
    close(fd[0]);
    close(fd[1]);

    /* Lire le clavier et écrire dans stdout (= le tube) */
    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, n);   // stdout = tube → le fils reçoit
    }

    /* Attendre la fin du fils */
    wait(NULL);
    exit(EXIT_SUCCESS);
}

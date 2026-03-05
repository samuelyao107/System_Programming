#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <sys/time.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 64

char *c_key =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

/* ------------------------------------------------------------------ */
/* Question 1 — Générer une clé de 2 caractères aléatoires            */
/* ------------------------------------------------------------------ */
void getkey(char key[3]) {
    struct timeval tp;

    /* Graine basée sur les microsecondes → très variable */
    gettimeofday(&tp, NULL);
    srandom((unsigned int) tp.tv_usec);

    /* Deux tirages aléatoires entre 0 et 63 */
    key[0] = c_key[random() % 64];
    key[1] = c_key[random() % 64];
    key[2] = '\0';
}

/* ------------------------------------------------------------------ */
/* Question 3 — Désactiver l'écho du terminal                         */
/* ------------------------------------------------------------------ */
int setsilent(struct termios *initial_term) {
    struct termios term;

    /* Sauvegarder l'état actuel */
    if (tcgetattr(STDIN_FILENO, initial_term) < 0) {
        perror("tcgetattr");
        return -1;
    }

    term = *initial_term;   // copie pour modification

    /* Question 3 — désactiver l'écho */
    term.c_lflag &= ~ECHO;

    /* Question 4 — mode non-canonique : lire caractère par caractère */
    term.c_lflag &= ~ICANON;
    term.c_cc[VMIN]  = 1;   // lire dès qu'1 caractère est disponible
    term.c_cc[VTIME] = 0;   // pas de timeout

    /* Appliquer immédiatement */
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* Question 5 — Restaurer le terminal dans son état initial           */
/* ------------------------------------------------------------------ */
int restaure_term(struct termios *initial_term) {
    if (tcsetattr(STDIN_FILENO, TCSANOW, initial_term) < 0) {
        perror("tcsetattr restaure");
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Question 4 — Lire le mot de passe caractère par caractère          */
/*              Afficher '*' pour chaque caractère tapé               */
/* ------------------------------------------------------------------ */
char *get_pass() {
    static char buf[BUFSIZE];
    char c;
    int i = 0;

    while ((c = getchar()) != '\n' && i < BUFSIZE - 1) {
        buf[i++] = c;
        putchar('*');       // afficher '*' à la place du caractère
        fflush(stdout);     // forcer l'affichage immédiat
    }

    buf[i] = '\0';
    putchar('\n');
    return buf;
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */
int main() {
    char buf[BUFSIZE], *pwd;
    char key[3];
    struct termios initial_term;

    /* Question 1 — générer et afficher la clé */
    getkey(key);
    printf("key = %s\n", key);

    /* Question 3+4 — passer en mode silencieux + non-canonique */
    printf("Tapez votre mot de passe : ");
    fflush(stdout);
    setsilent(&initial_term);

    /* Question 2 — lire le mot de passe */
    pwd = get_pass();

    /* Question 5 — restaurer le terminal avant d'afficher */
    restaure_term(&initial_term);

    /* Question 2 — chiffrer et afficher */
    printf("Mot de passe chiffre : %s\n", crypt(pwd, key));

    exit(EXIT_SUCCESS);
}

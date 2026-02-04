/*
 * Auteur(s): Samuel YAO
 *
 * Cet programme refait ce que fait la commande "ls". Il donne des
 * informnations sur les caracteristiques de fichiers dont le nom est passe
 * en parametre.
 *
 * Utilisation de la primitive stat(2) et de la fonction getpwuid(3).
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

/* Petite fonction qui se charge d'envoyer les messages d'erreur
   et qui ensuite "suicide" le processus. */

void erreur_grave(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

/* Fonction principale (fournie avec erreur(s?)) */

int main(int argc, char **argv) {
  struct stat status, *buffer;
  int r;
  struct passwd *pw, *another_pw;

  /*Correction le pointeur buffer n'avait pas l'adresse de status*/
  buffer = &status;
  pw = getpwuid(buffer->st_uid);
  //le deuxieme appel écrase le resultat du premier
 // another_pw = getpwuid(1000); 
  if(pw == NULL ) {
    erreur_grave("getpwuid");
  }
  
  r = stat(argv[1], buffer);
  if (r < 0)
    erreur_grave("Stat");

  printf("Fichier %s:  mode: %X  Taille: %ld  Proprietaire: %s\n",
	argv[1], buffer->st_mode, buffer->st_size, pw->pw_name);

  exit(EXIT_SUCCESS);
}
/* L'appelant doit fournir la mémoire pour stat alors que getpwuid fournir l'adresse de sa mémoire interne. C'est la raison pour laquelle après 2 appels successifs le resultat du 1er appel est écrasé*/
/* 
 * Auteur(s): Samuel YAO
 *
 * Programme a appeler avec ou sans l'option "true"
 * Lancer "top" auparavant (pour terminer taper "q").
 *
 * Avec l'option "true", le flag O_NONBLOCK est positionne ce qui rend le
 * read non bloquant (il rend -1 et errno est positionne a EAGAIN).  On
 * constate alors que la charge cpu monte...  On n'arrete pas d'appeler
 * read.
 * 
 * Sans l'option "true", la charge cpu n'augmente pas, le read est bloquant. 
 * Le process s'endort en attendant que des caracteres soient tapes au
 * clavier.
 *
 * On peut aussi lancer ce programme avec ou sans l'option "true" a l'aide
 * de la commande strace : strace read_on_delay [true].
 * 
 * Sous Solaris on utilisera truss au lieu de strace
*/
#define _POSIX_C_SOURCE 200809L 
#include <signal.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void hdl_block(int n){
  int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
  if(flag < 0) {
    perror("Fcntl F_GETFL");
    exit(EXIT_FAILURE);
  }

  flag = flag & ~O_NONBLOCK;
  if(fcntl(STDIN_FILENO, F_SETFL, flag) < 0) {
    perror("Fcntl F_SETFL");
    exit(EXIT_FAILURE);
  }

  printf("Mode BLOQUANT\n", n);
}

void hdl_non_block(int n){
  int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
  if(flag < 0) {
    perror("Could not get flags");
    exit(EXIT_FAILURE);
  }

  flag = flag | O_NONBLOCK;
  if(fcntl(STDIN_FILENO, F_SETFL, flag) < 0) {
    perror("Could not set flags");
    exit(EXIT_FAILURE);
  }

  printf("Mode NON-BLOQUANT\n", n);
}



int main(int argc, char **argv) {
  int flag, r;
  char buf[10];

  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sa.sa_handler = hdl_block;
  sigaction(SIGINT, &sa, NULL);

  sa.sa_handler = hdl_non_block;
  sigaction(SIGQUIT, &sa, NULL);

  if ((argc > 1) && (strcmp(argv[1], "true") == 0)) {
    flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flag < 0) {
      perror("Fcntl F_GETFL");
      exit(EXIT_FAILURE);
    }
    /* positionner maintenant le flag avec O_NONBLOCK */
    flag = fcntl(STDIN_FILENO, F_SETFL, flag | O_NONBLOCK);

    if (flag < 0) {
      perror("Fcntl F_SETFL");
      exit(EXIT_FAILURE);
    }
  }

  for (;;) {
    r = read(STDIN_FILENO, buf, 10);
    if ((r > 0) && (strncmp(buf, "quit", 4) == 0)) {
      exit(EXIT_SUCCESS);
    }
  }

}

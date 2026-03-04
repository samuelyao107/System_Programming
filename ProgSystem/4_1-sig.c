/* 
 * Auteur(s): Samuel YAO
 */
#define _POSIX_C_SOURCE 200809L 

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

void (*sig_avant)(int);		/* pour la question 4.3 */
struct sigaction sa_int, sa_quit;		/* pour la question 4.4 */

void hdl_sys1(int n) {
  printf("hdl_sys1: Signal recu: %d\n", n);

}

void hdl_msg1(int n){
  printf("Message 1 \n");
}

void hdl_msg2(int n){
   printf("Message 2\n");
}

void hdl_quit(int n){
  write(STDOUT_FILENO, "\n[QUIT recu - changement handler]\n", 35);
 /* Avec signal()
 //renvoie du handler precedent pour SIGINT
  sig_avant = signal(SIGINT, sig_avant);*/

  /*Avec sigaction()*/
  struct sigaction sa_new, sa_old;
  sa_new.sa_handler = sig_avant;
  sa_new.sa_flags = 0;
  sigemptyset(&sa_new.sa_mask);
  sigaction(SIGINT, &sa_new, &sa_old);
  sig_avant = sa_old.sa_handler; 
}




void travail() {
  /* Je travaille tres intensement !    */
  /* Ne cherchez pas a comprendre ;-) */
  /* Il n'y a rien a modifier ici     */
  const char msg[] = "-\\|/";
  const int sz = strlen(msg);
  int i = 0;

  for (;;) {
    write(STDOUT_FILENO, "\r", 1);
    usleep(100000);
    write(STDOUT_FILENO, " => ", 4);
    write(STDOUT_FILENO, &msg[i++], 1);
    if (i == sz) i = 0;
  }
}
void travail() __attribute__((noreturn));
/* Petit raffinement pour le compilateur: cette fonction ne termine pas */


int main() {
 printf("PID: %d\n", getpid());
/* Avec signal()
 signal(SIGILL, hdl_sys1);
 signal(SIGINT, hdl_msg1);
 sig_avant = hdl_msg2;
 signal(SIGQUIT, hdl_quit);
 */

 /* Avec sigaction() */
sa_int.sa_handler = hdl_msg1;
sa_int.sa_flags = 0;
sigemptyset(&sa_int.sa_mask);
sigaction(SIGINT, &sa_int, NULL);

sig_avant = hdl_msg2;

sa_quit.sa_handler = hdl_quit;
sa_quit.sa_flags = 0;
sigemptyset(&sa_quit.sa_mask);
sigaction(SIGQUIT, &sa_quit, NULL);
  
travail();
}

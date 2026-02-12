/* 
 * Auteur(s):
 */

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

void (*sig_avant)(int);		/* pour la question 4.3 */

void hdl_sys1(int n) {
  printf("hdl_sys1: Signal recu: %d\n", n);

}

void hdl_msg1(int n){
  /*write(STDOUT_FILENO, "\nMessage numero 1\n", 18);*/
  printf("Message 1 \n");
}

void hdl_msg2(int n){
   /*write(STDOUT_FILENO, "\nMessage numero 2\n", 18);*/
   printf("Message 2\n");
}

void hdl_quit(int n){
  write(STDOUT_FILENO, "\n[QUIT recu - changement handler]\n", 35);
  sig_avant = signal(SIGINT, sig_avant);
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
 
 signal(SIGILL, hdl_sys1);
 signal(SIGINT, hdl_msg1);
 sig_avant = hdl_msg2;
 signal(SIGQUIT, hdl_quit);
 
  
  travail();
}

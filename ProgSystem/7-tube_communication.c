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
    int com[2];
    char buff[256];
    int n;
    pipe(com);
    pid_t pid = fork();
    if (pid == 0){
        /*Inside the son*/

      close(com[1]);
      dup2(com[0], STDIN_FILENO);
        while ((n = read(STDIN_FILENO, buff, sizeof(buff))) > 0) {
            buff[n] = '\0';
            printf("Fils reçu : %s", buff); 
            fflush(stdout);
        }
    }
    else if(pid>0){
        /*Inside the father*/
       close(com[0]);
       dup2(com[1], STDOUT_FILENO);
       while (1) {
            fgets(buff, sizeof(buff), stdin);
            write(com[1], buff, sizeof(buff));
      }
      close(STDOUT_FILENO); 
      wait(NULL);
    }
    else{
        perror("fork failed");
    }
}

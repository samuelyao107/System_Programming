/*
 * Auteur(s): Samuel YAO
 */

#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>


#define BUFSIZE 512
enum TypeFichier { NORMAL, REPERTOIRE, ERREUR };

const char* OK200 = "HTTP/1.1 200 OK\r\n\r\n";
const char* ERROR403 = "HTTP/1.1 403 Forbidden\r\n\r\nAccess denied\r\n";
const char* ERROR404 = "HTTP/1.1 404 Not Found\r\n\r\nFile or directory not found\r\n";


/* Fonction typeFichier()
 * argument: le nom du fichier
 * rend une valeur de type enumeration delaree en tete du fichier
 * NORMAL, ou REPERTOIRE ou ERRREUR
 */
enum TypeFichier typeFichier(char *fichier) {
  struct stat status;
  int r;

  r = stat(fichier, &status);
  if (r < 0)
    return ERREUR;
  if (S_ISREG(status.st_mode))
    return NORMAL;
  if (S_ISDIR(status.st_mode))
    return REPERTOIRE;
  /* si autre type, on envoie ERREUR (a fixer plus tard) */
  return ERREUR;
}


/* envoiFichier()
 * Arguments: le nom du fichier, la socket
 * valeur renvoyee: true si OK, false si erreur
 */
#define BUFSIZE 1048
bool envoiFichier(char *fichier, int soc) {
  int fd;
  char buf[BUFSIZE];
  ssize_t nread;

  if(access(fichier,R_OK)== 0){

      write(soc,OK200,strlen(OK200));
      fd = open(fichier, O_RDONLY);

      if (fd < 0) {
        perror("Erreur ouverture fichier");
        return false;
      }

      while ((nread = read(fd, buf, sizeof(buf))) > 0) {
        write(soc, buf, nread); 
      }

      close(fd);
      return true;

   }else{
       write(soc,ERROR403,strlen(ERROR403));
       return false;
   }
}


/* envoiRep()
 * Arguments: le nom du repertoire, la socket
 * valeur renvoyee: true si OK, false si erreur
 */
bool envoiRep(char *rep, int soc) {
  DIR *dp;
  struct dirent *pdi;
  char buf[1024], nom[1024];

  dp = opendir(rep);
  if (dp == NULL)
    return false;

  write(soc, OK200, strlen(OK200));
  sprintf(buf, "<html><title>Repertoire %s</title><body>", rep);
  write(soc, buf, strlen(buf));

  while ((pdi = readdir(dp)) != NULL) {

     sprintf(buf, "%s/%s",rep, pdi->d_name);
     if(typeFichier(buf)==NORMAL){
        sprintf(nom,"<img src=\"icons/generic.gif\">%s<br>",pdi->d_name);
        write(soc,nom,strlen(nom));
     }else if(typeFichier(buf)==REPERTOIRE){
        sprintf(nom,"<img src=\"icons/folder.gif\">/%s<br>",pdi->d_name);
        write(soc,nom,strlen(nom));
     }else{
        continue;
     }

  }
  write(soc, "\n\r", 2);
  closedir(dp);
  return true;
}


void communication(int soc, struct sockaddr *from, socklen_t fromlen) {
  int s;
  char buf[BUFSIZE];
  ssize_t nread;
  char *pf;
  enum op { GET, PUT } operation;
  enum TypeFichier type;

  nread = read(soc, buf, BUFSIZE);
  if (nread > 0) {
    if (strncmp(buf, "GET", 3) == 0)
      operation = GET;
  } else {
    perror("Erreur lecture socket");
    return;
  }

  switch (operation) {
    case GET:
      pf = strtok(buf + 4, " ");
      pf++; 
      if(strlen(pf) == 0){
        pf= "index.html";
      }
      type = typeFichier(pf);
      if(type == NORMAL){
          envoiFichier(pf, soc);
      }else if (type == REPERTOIRE){
            envoiRep(pf,soc);
      }else {
        write(soc,ERROR404,strlen(ERROR404));
      }
      break;
  }

  close(soc);
}


int main(int argc, char **argv) {
  int sfd, s, ns,r;
  char host[NI_MAXHOST];
  char port[NI_MAXSERV];
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  struct sockaddr_storage from;
  socklen_t fromlen;

  if (argc != 2) {
    printf("Usage: %s  port_serveur\n", argv[0]);
    exit(EXIT_FAILURE);
  }


  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;           
  hints.ai_socktype = SOCK_STREAM;      
  hints.ai_flags = AI_PASSIVE;          
  hints.ai_flags |= AI_V4MAPPED|AI_ALL; 
  hints.ai_protocol = 0;              

  s = getaddrinfo(NULL, argv[1], &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {

    
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;
     
 
    r = bind(sfd, rp->ai_addr, rp->ai_addrlen);
    if (r == 0)
      break;           
    close(sfd);
  }

    if (rp == NULL) {     /* Aucune adresse valide */
    perror("bind");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(result); /* Plus besoin */

  /* Positionnement de la machine a etats TCP sur listen */
  r= listen(sfd, 15);
  if(r == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
    
  signal(SIGCHLD, SIG_IGN);
  for (;;) {
    /* Acceptation de connexions */
    fromlen = sizeof(from);
    ns = accept(sfd, (struct sockaddr *)&from, &fromlen);
    if (ns == -1) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    switch (pid){
      case -1:
        perror("fork");
        exit(EXIT_FAILURE);
      case 0:
      /*Dans le fils*/
        close(sfd);
        /* Reconnaissance de la machine cliente */
        s = getnameinfo((struct sockaddr *)&from, fromlen,
                host, NI_MAXHOST,
                port, NI_MAXSERV,
                NI_NUMERICHOST | NI_NUMERICSERV);
                
        if (s == 0)
          printf("Debut avec client '%s:%s'\n", host, port);
        else
          fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

        communication(ns, (struct sockaddr *)&from, fromlen);

        exit(EXIT_SUCCESS); 
      default:
      /*Dans le père*/
        close(ns);
    }
  }

}

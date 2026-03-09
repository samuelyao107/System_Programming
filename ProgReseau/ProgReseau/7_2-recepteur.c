/*
 * Auteur(s): Samuel YAO
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#define BUFSIZE 100000
int sfd;
struct ip_mreqn config_ipv4;
struct ipv6_mreq config_ipv6;
struct addrinfo  *result2;

void handler(int n){
      if(result2->ai_family == AF_INET){

        if (setsockopt(sfd,IPPROTO_IP,IP_DROP_MEMBERSHIP,&config_ipv4, sizeof(config_ipv4))<0) { /*modifications faites ici*/
              perror("setsockopt");
              exit(EXIT_FAILURE);
          }
       }else if(result2->ai_family == AF_INET6){
          
          if (setsockopt(sfd,IPPROTO_IPV6,IPV6_DROP_MEMBERSHIP,&config_ipv6,sizeof(config_ipv6)) < 0) { /*modifications faites ici*/
              perror("setsockopt");
              exit(EXIT_FAILURE);
          }
       }

       printf("Quitting multicast group\n");
       exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  int s, rsz, r;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  ssize_t nrecv;
  char buf[BUFSIZE];
  struct sockaddr_storage from;
  socklen_t fromlen;
  char host[NI_MAXHOST], service[NI_MAXSERV];
  

  if (argc != 3) {
    printf("Usage: %s port\n", argv[0]);
    exit(EXIT_FAILURE);
  }

 /* Construction de l'adresse locale (pour bind) */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;           /* Force IPv6 */
  hints.ai_socktype = SOCK_DGRAM;       /* Datagram socket */
  hints.ai_flags = AI_PASSIVE;          /* Pour l'adresse IP joker */
  hints.ai_flags |= AI_V4MAPPED|AI_ALL; /* IPv4 remappe en IPv6 */
  hints.ai_protocol = 0;                /* Any protocol */

  s = getaddrinfo(NULL, argv[1], &hints, &result);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  getaddrinfo(argv[2], argv[1], &hints, &result2);

  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* getaddrinfo() retourne une liste de structures d'adresses.
     On essaie chaque adresse jusqu'a ce que bind(2) reussisse.
     Si socket(2) (ou bind(2)) echoue, on (ferme la socket et on)
     essaie l'adresse suivante. cf man getaddrinfo(3) */
  for (rp = result; rp != NULL; rp = rp->ai_next) {

    /* Creation de la socket */
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;

    int reuse = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));  

    /* Association d'un port a la socket */
    r = bind(sfd, rp->ai_addr, rp->ai_addrlen);
    if (r == 0){ /* Succes */
       if(result2->ai_family == AF_INET){
          config_ipv4.imr_multiaddr = ((struct sockaddr_in*)result2->ai_addr)->sin_addr;
          config_ipv4.imr_address.s_addr = INADDR_ANY;
          config_ipv4.imr_ifindex = 0;
          
          if (setsockopt(sfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&config_ipv4, sizeof(config_ipv4))<0) { /*modifications faites ici*/
              perror("setsockopt");
              exit(EXIT_FAILURE);
          }
       }else if(result2->ai_family == AF_INET6){
          config_ipv6.ipv6mr_multiaddr = ((struct sockaddr_in6*)result2->ai_addr)->sin6_addr;
          config_ipv6.ipv6mr_interface = 0;
          
          if (setsockopt(sfd,IPPROTO_IPV6,IPV6_JOIN_GROUP,&config_ipv6,sizeof(config_ipv6)) < 0) { /*modifications faites ici*/
              perror("setsockopt");
              exit(EXIT_FAILURE);
          }
       }
       break;  
    }
               
    close(sfd);
  }

  if (rp == NULL) {     /* Aucune adresse valide */
    perror("bind");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(result); /* Plus besoin */
  signal(SIGINT, handler);
  /* Force la taille du buffer de reception de la socket */
  rsz = 80000; //80 000 au-lieu de 800000
  if ( setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &rsz, sizeof(rsz)) == 0 )
    printf("SO_RCVBUF apres forcage: %d octets\n", rsz);
  else
    perror("setsockopt SO_RCVBUF");

  /* Boucle de communication */
  for (;;) {
    /* Reception donnees */
    fromlen = sizeof(from);
    nrecv = recvfrom(sfd,buf,sizeof(buf),0,(struct sockaddr *)&from,&fromlen);
    if (nrecv == -1) {
      perror("Erreur en lecture socket\n");
      exit(EXIT_FAILURE);
    }
    printf("Recu %zd octets\n", nrecv);

    /* Reconnaissance de la machine cliente */
    s = getnameinfo((struct sockaddr *)&from, fromlen,
			host, NI_MAXHOST, service, NI_MAXSERV,
			NI_NUMERICHOST | NI_NUMERICSERV);
    if (s == 0)
      printf("Emetteur '%s'  Port '%s'\n", host, service);
    else
      printf("Erreur: %s\n", gai_strerror(s));
  }
}

/* 
- Un paquet IPv4 a une taille maximale de **65535 octets** (champ "Total Length" sur 16 bits)
- L'en-tête IP fait **20 octets**
- L'en-tête UDP fait **8 octets**

Ce qui donne : `65535 - 20 - 8 = 65507 octets` de données utiles maximum.
En IPv6, l'en-tête fait 40 octets
Ce qui donne : 65535 - 40 - 8 = 65487 octets de données utiles maximum.
*/
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <features.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/timeb.h>

#include "../packet.h"
#include "../bloom.h"
#include "../utility.h"


/* per le socket */

#define SP_TCP_PORT    5002	//porta di riferimento trasmissione tcp
#define SP_UDP_PORT    5003  //porta di riferimento trasmissione udp
#define PEER_TCP_PORT  5004	//porta di riferimento trasmissione tcp
#define PEER_UDP_PORT  5005  //porta di riferimento trasmissione udp
#define UPLOAD_PORT    5006 //porta da ascolto per l'upload
#define DOWNLOAD_PORT  5007  //porta per download
#define BACKLOG          10
#define MAXLINE        1025

#define DIM_FILTER 8000 //dimensione del filtro di bloom
#define DIM_MAX_PKT 1500 //dimensione massima dei pacchetti UDP ricevibili
#define MAX_TIME 86400.0 //valore di riferimento per il calcolo del rating riferito al tempo 

//#define N_CHECK 3 //numero di prove da fare sul filtro

/* VARIABILI GLOBALI VARIE */
double ratC; //rating del computer
double ratT; //tempo passato in rete
double rating;

struct timeb t0,t;
struct in_addr IPSP; //riferimento all'ip del superpeer
int conn; //se conn=1 il superpeer dovrebbe essere presente altrimenti no
struct in_addr my_ip; //riferimento al mio ip 
char* ip_server;
struct sockaddr_in	servaddr;
int sockUDPsd; //descrittore della socket udp
int pid_up; //pid del processo per l'upload
int pid_SP; //pid del processo del SP (se si è SP, altrimenti -1)
int var_ack;
int ricerca; //se 1 c'è già una ricerca in corso

int CHUNK;
char* PATH_FILEBLOOM;
char* PATH_DIR;
char* PATH_LOCALE;
int NUM_UPLOAD;

int SERV_TCP_PORT;
int SERV_UDP_PORT;

/* FUNZIONI UDP*/
int join_UDP(int sockfd, struct in_addr IP, double rating, int cont);
int leave_UDP(int sockfd, struct in_addr IP, int cont);
int ping_UDP(int sockfd, struct in_addr IP, int cont);
int whohas_UDP (int sockfd, struct in_addr IP, char* word ,int cont);
int stop_UDP (int sockfd, struct in_addr IP, int cont);

/* GESTORI DEI SEGNALI */
void alarm_handler();
void ping_handler();

struct list_head 
{
	struct list_head  *next;
	struct list_head  *prev;
  struct sockaddr_in IP;
};

struct list_head list;
struct list_head list_result;

void Ins_coda(struct sockaddr_in ind, struct list_head *lista);
void Ins_testa(struct sockaddr_in ind, struct list_head *lista);
void Stampa(struct list_head *lista);
void Cancella(struct sockaddr_in x, struct list_head *lista);
struct sockaddr_in *GET_LIST(int pos, struct list_head *lista);
int Listlen(struct list_head *lista);
void cancella_lista(struct list_head *lista);
ssize_t	writen(int fd, const void *buf, size_t n);
int	readline(int fd, void *vptr, int maxlen);
void str_cli_echo(int sockd, char *IP);
int join();
void Peer();
void Superpeer();


/* DOWNLOAD E UPLOAD */
int download (struct in_addr ip, char* file);
int upload();

/* FUNZIONI PER L'INTERAZIONE DINAMICA CON I SUPERPEER*/
int connect_to_SP (); /* scandisce la lista dei SP e si connette al piu vicino */
int change_to_SP (struct in_addr ip); /* funzione per la connessione ad uno specifico SP di cui è noto l'indirizzo ip */
int change_SP (); /* sfruttando la connect_to_SP() cambia il SP del peer che invoca la funzione */
int exitP (); /* chiude tutti i processi */

int elezione_nuovo_SP(); /* restituisce il risultato di eleggi_UDP, quindi se restituisce 1 vuol dire che il peer si è connesso a un nuovo SP*/
int eleggi_UDP (int sockfd, struct in_addr IP); /* restituisce 1 se è stato creato un superpeer e il peer è riuscito a connettersi ad esso */


/* CONFIGURAZIONE */
void parsePeerVar();

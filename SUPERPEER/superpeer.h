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
#define BACKLOG       10
#define MAXLINE     1024

#define DIM_FILTER 8000 //dimensione del filtro di bloom
#define DIM_MAX_PKT 1500 //dimensione massima dei pacchetti UDP ricevibili

#define NUMERO_MAX_PEER 100 //numero massimo di "connesioni" UDP con i peer 
#define NUMERO_MAX_SUPERPEER 6 //numero massimo di "connesioni" UDP con i peer 

struct list_head 
{
         struct list_head  *next;
         struct list_head  *prev;
         struct sockaddr_in IP;     
};

struct list_peer
{
         struct list_peer  *next;
         struct list_peer  *prev;
         struct sockaddr_in IP;  
         BLOOM* filtro;   
         int TTL;
         /* parametri per il rate */
         double rating;
};

struct list_client 
{
         struct list_client  *next;
         struct list_client  *prev;
         int connsd;
         int num_peer;
         int num_SP;
         int TTL;
};

struct list_query
{
      struct list_query *next;
      struct list_query *prev;
      struct list_head try_list, done_list;      
      int ID;
      char* nome_file;
      int SP_contattati;
      int IP_ottenuti;
      struct sockaddr_in peer;
};

struct list_head list;
struct list_client client;
struct list_peer peer;
struct list_query lista_query;
int id_whohas;
struct sockaddr_in my_addr;
struct in_addr	bootaddr;
struct in_addr	appaddr;
double rating;
int MAXPEER;
int MAXSUPERPEER;
int SERV_TCP_PORT;
int SERV_UDP_PORT;

//FUNZIONI RELATIVE ALLE LISTE DI TIPO list_head
void insTesta(struct sockaddr_in ind, struct list_head *lista);
void stampa(struct list_head *lista);
void cancella(struct sockaddr_in x, struct list_head *lista);
struct sockaddr_in *getList(int pos, struct list_head *lista);
int listLen(struct list_head *lista);
void cancellaLista(struct list_head *lista);
int isPresentAddr(struct list_head *lista, long *ptr);

//FUNZIONI RELATIVE ALLA LISTA PEER
struct sockaddr_in *getPeer(int pos, struct list_peer *Peer);
int listLenPeer(struct list_peer *Peer);
void stampaPeer(struct list_peer *Peer);
void cancellaPeer(struct sockaddr_in x, struct list_peer *Peer);
void insPeer(struct sockaddr_in ind, BLOOM* bloom, double rating, struct list_peer *Peer);
void decrementaTtl(struct list_peer *peer);
void reimpostaTtl(struct sockaddr_in x, struct list_peer *peer);
void reimpostaRating(double rating, struct sockaddr_in x, struct list_peer *Peer);
int searchPeer(struct sockaddr_in x, struct list_peer *peer);
void riempiArray(BLOOM* filtri[], char ip[], struct list_peer *Peer);

//FUNZIONI RELATIVE ALLA LISTA DEI SUPERPEER
void inserimento(int conn, int peer, int s_peer, struct list_client *client);
void stampaClient(struct list_client *client);
void cancellaClient(int x, struct list_client *client);
int listLenClient(struct list_client *client);
int getConnsd(int pos, struct list_client *client);
int getMassimo(struct list_client *client);
void decrementaTtlClient(struct list_client *client);
void reimpostaTtlClient(int x, struct list_client *client);
void aggiornaStatistiche(int x, int p, int sp, struct list_client *client);
void ottieniViciniNonPieni(char ip[], int *cont, struct list_client *client);
int getSuperPeerBassaOccupazione (struct list_client *client);

//FUNZIONI RELATIVE ALLA LISTA DELLE QUERY PER LA WHOHAS
void insQuery(int id,struct sockaddr_in P, char* nome_file, struct list_query *query);
void cancellaQuery(int id, struct list_query *query);
struct sockaddr_in *getPeerQuery(int id, struct list_query *query);
int searchId(int id, struct list_query *query);
void incrementaIpOttenuti(int id, int IP, struct list_query *query);
void incrementaSpContattati(int id, int SP, struct list_query *query);
int getIpOttenuti(int id, struct list_query *query);
int getSpContattati(int id, struct list_query *query);
int getIdFromPeer(struct sockaddr_in peer, struct list_query *query);


//FUNZIONI SUPERPEER
ssize_t writen(int fd, const void *buf, size_t n);
int readline(int fd, void *vptr, int maxlen);
void receiveBloomUDP(int sockfd, struct sockaddr_in addr);
int whoHasUDP(packetWHHS *pck, char *dati_result, int *cont);
int joinUDP(struct sockaddr_in addr, packetUDP *pck);
int registerSP(char* str_ip);
int pingSpUDP (struct in_addr IP);
int pingSpTCP ();
int sendJoinTCP(struct in_addr ip, int *max, fd_set *allset, int cont);
int connessioneOverlay(int *max, fd_set *allset);
int parsingWhoHas(packetWHHS *pck, char *dati_result);
int inoltraWhoHas(int id, int sockfd);


/* FUNZIONI PER LA GESTIONE DINAMICA DELLA RETE */
struct in_addr bestPeer ();

/* CONFIGURAZIONE */
void parseSuperPeerVar();

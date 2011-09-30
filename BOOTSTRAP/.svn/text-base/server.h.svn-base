#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wait.h>      
#include <sched.h>


#define BACKLOG		200
#define MAXLINE		1025



struct list_head 
{
	struct list_head  *next;
	struct list_head  *prev;
  struct sockaddr_in IP;
  int TTL;
};

struct list_client 
{
	struct list_client  *next;
	struct list_client  *prev;
	int connsd;
};


struct list_client client;
struct list_head list;
int TCP_PORT;
int UDP_PORT;

      /****** FUNZIONI PER LA LISTA DI SUPERPEER ********/
void insCoda(struct sockaddr_in ind, struct list_head *lista); 
void insTesta(struct sockaddr_in ind, struct list_head *lista);
//void stampa(struct list_head *lista);
void cancella(struct sockaddr_in x, struct list_head *lista);
struct sockaddr_in *getList(int pos, struct list_head *lista);
int listLen(struct list_head *lista);
void decrementaTtl(struct list_head *lista);
void reimpostaTtl(struct sockaddr_in x, struct list_head *lista);
int search(struct sockaddr_in search, struct list_head *lista);
void cancellaLista(struct list_head *lista);

    /****** FUNZIONI PER LA LISTA DELLE CONNESSIONI DELLA SELECT ********/
void inserimento(int conn, struct list_client *client);
//void stampaClient(struct list_client *client);
void cancellaClient(int x, struct list_client *client);
int listLenClient(struct list_client *client);
int getConnsd(int pos, struct list_client *client);
int getMassimo(struct list_client *client);


         /****** FUNZIONI GENERICHE DEL SERVER ********/
void strSrv(int sockd, struct sockaddr_in);
void randomize(int n, int v[],int dim, long ip);
void join(int sockd, struct sockaddr_in);
ssize_t	writen(int fd, const void *buf, size_t n);
int	readline(int fd, void *vptr, int maxlen);


/* CONFIGURAZIONE */
void parseServVar();

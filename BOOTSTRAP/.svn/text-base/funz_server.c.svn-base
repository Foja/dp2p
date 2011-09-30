#include "server.h"

/******/
ssize_t writen(int fd, const void *buf, size_t n)
{
  size_t nleft;
  ssize_t nwritten;
  const char *ptr;

  ptr = buf;
  nleft = n;

  while (nleft > 0) 
  {

    if ((nwritten = write(fd, ptr, nleft)) <= 0)
    {
       if ((nwritten < 0) && (errno == EINTR)) 
          nwritten = 0;
 	    
       else
          return(-1);	    /* errore */
    }

    nleft -= nwritten;
    ptr += nwritten;  
  }

  return(n-nleft);
}

/******/
int readline(int fd, void *vptr, int maxlen)
{
  int  n, rc;
  char c, *ptr;

  ptr = vptr;

  for (n = 1; n < maxlen; n++) 
  {

    if ((rc = read(fd, &c, 1)) == 1) 
    { 
      *ptr++ = c;

      if (c == '\n') 
       break;

    } 

   else 
      if (rc == 0) 
      {           /* read ha letto l'EOF */

 	      if (n == 1)
        {
		       return(0);
	      }	/* esce senza aver letto nulla */

 	      else 
         break;
      }
 
      else
       if ( (rc < 0) && (errno == EINTR) )
       {
          n--;
          continue;
       } 

       else
        return(-1);    /* errore */
  }

  *ptr = 0;	/* per indicare la fine dell'input */
  return(n);  /* restituisce il numero di byte letti */
}
/************************************************************/
//FUNZIONI LISTA IP


void insTesta(struct sockaddr_in ind, struct list_head *lista)
{
    struct list_head *ptr_list, *ptr_el;
    ptr_list = lista->next;
    ptr_el = (struct list_head *)malloc(sizeof(struct list_head));
    ptr_el->IP = ind;
    ptr_el->TTL = 5;
    ptr_el->prev = lista;
    ptr_el->next = ptr_list;
    ptr_list->prev = ptr_el;
    lista->next = ptr_el;
}

void decrementaTtl(struct list_head *lista)
{
  int i = 0;
  struct list_head *ptr_list, *ptr_app;
  ptr_list = lista;

  while (ptr_list->next != lista)
  {
     ptr_list = ptr_list->next;
     ptr_list->TTL--;
     printf("TTL:%d\n",ptr_list->TTL);

     if (ptr_list->TTL == 0)
     {
         ptr_app = ptr_list->prev;
         printf("CANCELLATO IP:%s\n",inet_ntoa(ptr_list->IP.sin_addr));
         cancella(ptr_list->IP,lista);
         ptr_list = ptr_app;
     }
  }
  return;
}

void reimpostaTtl(struct sockaddr_in x, struct list_head *lista)
{
  struct list_head *ptr_app, *ptr_list, *ptr_el;
  ptr_list = lista->next;
  ptr_el = ptr_list;

  while ( (ptr_list != lista) && ( ptr_list->IP.sin_addr.s_addr != x.sin_addr.s_addr ) )
         ptr_list = ptr_list->next;

  if (ptr_list == lista)
      printf("IP nn presente\n");

  if (ptr_list->IP.sin_addr.s_addr == x.sin_addr.s_addr)
  {
      ptr_list->TTL = 5;
  }
  return;
}

/*void stampa(struct list_head *lista)
{
  struct list_head *ptr_list;
  ptr_list = lista;

  while (ptr_list->next != lista)
  {
    ptr_list = ptr_list->next;
    printf("next= %lx\n, prev= %lx\n, IP= %s\n",(long)ptr_list->next, (long)ptr_list->prev, inet_ntoa(ptr_list->IP.sin_addr));
  }

  return;
}*/

void cancella(struct sockaddr_in x, struct list_head *lista)
{
       struct list_head *ptr_app, *ptr_list, *ptr_el;
       ptr_list = lista->next;
       ptr_el = ptr_list;

       while ( (ptr_list != lista) && ( ptr_list->IP.sin_addr.s_addr != x.sin_addr.s_addr ) )
             ptr_list = ptr_list->next;

       if (ptr_list == lista)
           printf("IP nn presente\n");

       if (ptr_list->IP.sin_addr.s_addr == x.sin_addr.s_addr)
       {
            ptr_el = ptr_list;
            ptr_app = ptr_list->prev;
            ptr_app->next = ptr_list->next;
            ptr_list = ptr_list->next;
            ptr_list->prev = ptr_app;
            free(ptr_el);
       }
       return;
}

void cancellaLista(struct list_head *lista)
{
  struct list_head *ptr_list, *ptr_el;
  ptr_list = lista->next;

  while (ptr_list != lista)
  {
    ptr_el = ptr_list;
    ptr_list = ptr_list->next;
    free(ptr_el);
  }

  lista->next = lista;
  lista->prev = lista;
}

int search(struct sockaddr_in search, struct list_head *lista)
{
  int res = 0;
  struct list_head *ptr_list;
  ptr_list = &list;

  while (ptr_list->next != &list)
  {
    ptr_list = ptr_list->next;

    if (ptr_list-> IP.sin_addr.s_addr == search.sin_addr.s_addr)
    {
       res = 1;
       break;
    }

  }

  return res;
}


int listLen(struct list_head *lista)
{
    int i = 0;
    struct list_head *ptr_list;
    ptr_list = lista->next;

    while (ptr_list != lista)
    {
      ptr_list = ptr_list->next;
      i++;
    }

    return i;
}


struct sockaddr_in *getList(int pos, struct list_head *lista)
{
   int i = 0;
   struct list_head *ptr_list;
   ptr_list = lista->next;

   while ( (ptr_list != lista) && (i < pos) )
   {
     ptr_list = ptr_list->next;
     i++;
   }

   if (ptr_list == lista)
      return NULL;

   return &(ptr_list->IP);
}

/*************************************/
//FUNZIONI LISTA CLIENT

void inserimento(int conn, struct list_client *client)
{
    struct list_client *ptr_list, *ptr_el;
    ptr_list = client->next;
    ptr_el = (struct list_client *)malloc(sizeof(struct list_client));
    ptr_el->connsd = conn;
    ptr_el->prev = client;
    ptr_el->next = ptr_list;
    ptr_list->prev = ptr_el;
    client->next = ptr_el;
}

/*void stampaClient(struct list_client *client)
{
       struct list_client *ptr_list;
       ptr_list = client;

       while (ptr_list->next != client)
       {
         ptr_list = ptr_list->next;
         printf("next= %lx\n, prev= %lx\n, conn= %d\n",(long)ptr_list->next, (long)ptr_list->prev,ptr_list->connsd);
       }
}*/


void cancellaClient(int x, struct list_client *client)
{      
       struct list_client *ptr_app, *ptr_list, *ptr_el;
       ptr_list = client->next;
       ptr_el = ptr_list;
       
       while ( (ptr_list != client) && (ptr_list->connsd != x) )
          ptr_list = ptr_list->next;                 
       
       if (ptr_list == client)
          printf("IP nn presente\n");

       if (ptr_list->connsd == x)
       {
          ptr_el = ptr_list;
          ptr_app = ptr_list->prev;
          ptr_app->next = ptr_list->next; 
          ptr_list = ptr_list->next;
          ptr_list->prev = ptr_app;
          free(ptr_el);        
       }
}


int listLenClient(struct list_client *client)
{
    int i = 0;
    struct list_client *ptr_list;
    ptr_list = client->next;

    while (ptr_list != client)
    {
       ptr_list = ptr_list->next;
       i++; 
    }   

    return i;
}


int getConnsd(int pos, struct list_client *client)
{
   int i = 0;
   struct list_client *ptr_list;
   ptr_list = client->next;
  
   while ( (ptr_list != client) && (i < pos) )
   {
       ptr_list = ptr_list->next;
       i++;
   }

   if (ptr_list == client)
      return -1;

   return ptr_list->connsd;
}

int getMassimo(struct list_client *client)
{
   int max = -1;
   struct list_client *ptr_list;
   ptr_list = client->next;
  
   while  (ptr_list != client) 
   {
      if (ptr_list->connsd > max )
          max = ptr_list->connsd;

      ptr_list = ptr_list->next;
   }

   return max;
}

/**************************************/

void randomize(int n, int v[],int dim, long ip)
{
 int numero_casuale;
 int loop, i = 0, j;
 time_t t;
 srand((int)time(NULL));
 v[i] = (rand() + ip) % n;
	
 for (i = 1; i < dim; i++)
 {
   do
   {
     loop = 0;
     numero_casuale = ((rand() + ip) % n) ;
     for (j = 0; j < i; j++)
     {
        if (numero_casuale == v[j])
        {
           loop = 1;
           break;
        }
     }

   }while(loop);

   v[i] = numero_casuale;
 }

 return ;
}

void join(int sockd, struct sockaddr_in peer)
{  
   int fd1, fd2, ld, x;
   int i = 0, j = 0, cont = 0, len, *v, dim, ap;
   struct sockaddr_in *app;
   char buffer[1024],*c;
   long ip;
   x = listLen(&list);

   if (x == 0) /*se lista vuota restituisci un messaggio di empty*/
   {
     printf("empty\n");

     if (writen(sockd,"empty", 5) != 5) 
     {
	     fprintf(stderr, "errore in write");
	     exit(1);
     }   

     insTesta(peer,&list);
   }
     
   else if (x <= 5) /*se la lista contiene meno di 6 elementi restituisci tutta la lista di superpeer*/
   {

     if (writen(sockd,"ack\n", 4) != 4) 
     {
       fprintf(stderr, "errore in write");
       exit(1);
     }   

     len = (listLen(&list) << 2) + 4;
     ap = len - 4;
     c = (char *) &ap;

     for (i = 0; i < 4; i++)
        buffer[i] = *(c + i);

     x = listLen(&list);
     printf("lunghezza lista: %d\n", x);

     for (i = 0; i < x; i++)
     {
       ip = getList(i,&list)->sin_addr.s_addr;
       c = (char *)&ip;

       for (j = 0; j < 4; j++)
       {
         buffer[cont+4]= *((char *)(c+j));
         cont++;
       }

     }

     if (writen (sockd, buffer,len )  != len ) 
     {
	     fprintf(stderr, "errore in write");
	     exit(1);
     } 
   }

   else if (x > 5) 
   { 

     if (writen(sockd,"ok\n", 3) != 3) 
     {
         fprintf(stderr, "errore in write");
         exit(1);
     } 

     printf("lunghezza lista: %d\n", listLen(&list));

     ap = 20;
     c = (char *) &ap;
             
     for (i = 0; i < 4; i++)
       buffer[i] = *(c + i);

     if ( (v = (int *)malloc(20) ) == NULL)
     {
        perror("errore in malloc");
        exit (0);
     }

     ip = peer.sin_addr.s_addr;
     randomize(listLen(&list), v, 5, ip); //seleziona a caso 5 IP dalla lista e mette in v l'indirizzo del primo elemento dell'array di IP 

     for (i = 0; i < 5; i++)      
     {  
       ip = getList(*(v+i),&list)->sin_addr.s_addr;
       c = (char *)&ip;

       for (j = 0; j < 4; j++)
       {
           buffer[cont+4] = *((char *)(c+j));
           cont++;
       }
                          
     }

     if ( writen (sockd, buffer, 24 )  != 24 ) 
     {
       fprintf(stderr, "errore in write");
       exit(1);
     }

     memset((void *)&buffer, 0, sizeof(buffer));
     free(v); 
   }

   i = 1;

   while (i)
   {

      if ( ( (x = close(sockd) ) == -1) && (errno == EINTR) )
         continue;

      if ( (x == -1) && (errno != EINTR) )
        {
           perror("errore in close");
           exit(1);
        }

      i = 0;
    }

   return;
}

/* Il server processa la richiesta arrivata da un peer */
void strSrv(int sockd, struct sockaddr_in cli)
{
  int n;
  char line[10];

  for (;;)
  {

    if ((n = readline(sockd, line, MAXLINE)) <= 0)
      return; //il client ha chiuso la connessione e inviato EOF
    

    if (strncmp(line,"join\n",5) == 0) //verifica se il comando ricevuto è di tipo join
    {
      printf("%s",line);
      join(sockd,cli); //svolge il lavoro della join lato server bootstrap
    }

    else
    if (strcmp(line,"register\n") == 0) //se comando ricevuto è di tipo register salva IP nella sua lista di superpeer
    {
      join(sockd,cli);

      if (search(cli, &list) == 0)
         insTesta(cli, &list);//se l'elemento non è presente lo metto in lista

    }

    else
    if (strcmp(line,"leave\n") == 0) //se comando ricevuto è di tipo leave cerca indirizzo IP nella lista superpeer e lo elimina
    {  
       printf("Cancellato dalla lista IP:%s\n", inet_ntoa(cli.sin_addr));
       cancella(cli, &list);
    }

  }
}

void parseServVar()
{

  char* path_config="../config";
  
  int fd = open ( path_config, O_RDONLY);

  int n;
  char c;
  char* nome_var;
  char* val_var;

  while ((n = read( fd, &c, 1)) == 1)
  {

    nome_var = malloc (100);
    val_var = malloc (100);
    if(c == '#')
    {
      int i = 0; //d'appoggio per concatenare

      while (((n = read( fd, &c, 1)) == 1) && (c != ':'))
      {
        //finche non trovo ":" vuol dire che sto parsando il nome della variabile da settare
        *(nome_var+i) = c;
        i++;
      }

      while (((n = read( fd, &c, 1)) == 1) && (c == ' '))
      {
        //salto gli spazi tra il nome della variabile e il suo valore
      }

      i = 1;
      *val_var = c;
      while (((n = read( fd, &c, 1)) == 1) && (c != ';'))
      {
        //finche non trovo ":" vuol dire che sto parsando il nome della variabile da settare
        *(val_var+i) = c;
        i++;
      }

      //una volta letto il nome della variabile e il suo valore lo devo inserire nella giusta variabile
      
			if( strcmp( nome_var, "BOOT_PORT_TCP") == 0)
      {
        TCP_PORT = atoi(val_var);        
      }
      if( strcmp( nome_var, "BOOT_PORT_UDP") == 0)
      {
        UDP_PORT = atoi(val_var);        
      }
    }
  }
}

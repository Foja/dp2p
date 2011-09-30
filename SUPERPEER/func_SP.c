#include "superpeer.h"



/*** FUNZIONI PER LETTURA E SCRITTURA SU SOCKET ***/
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
         return (-1);   /* errore */
    }

    nleft -= nwritten;
    ptr += nwritten;  
  }

  return (n-nleft);
}


int readline(int fd, void *vptr, int maxlen)
{
  int  n, rc;
  char c, *ptr;

  ptr = vptr;
  for (n = 1; n < maxlen; n++) 
  {

    if ( (rc = read(fd, &c, 1) ) == 1) 
    { 

      *ptr++ = c;

      if (c == '\n') 
        break;
    } 

   else 
      if (rc == 0) 
      {           /* read ha letto l'EOF */

 	      if (n == 1)
		       return(0);  /* esce senza aver letto nulla */

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
//FUNZIONI LISTA SUPERPEER

void inserimento(int conn, int peer, int s_peer, struct list_client *client)
{
  struct list_client *ptr_list, *ptr_el;
  ptr_list = client->next;
  ptr_el = (struct list_client *)malloc(sizeof(struct list_client));
  ptr_el->connsd = conn;
  ptr_el->num_peer = peer;
  ptr_el->num_SP = s_peer;
  ptr_el->TTL = 5;
  ptr_el->prev= client;
  ptr_el->next= ptr_list;
  ptr_list->prev= ptr_el;
  client->next= ptr_el;
}

void stampaClient(struct list_client *client)
{
  struct list_client *ptr_list;
  ptr_list = client;
  printf("%d ==> STAMPA LISTA\n", getpid());
  int cont = 0;

  while (ptr_list->next != client)
  {
    ptr_list = ptr_list->next;
    printf("elem [%d] ==> next= %lx, prev= %lx, conn= %d\n", cont, (long)ptr_list->next, (long)ptr_list->prev, ptr_list->connsd);
    cont++;   
  }

}


void cancellaClient(int x, struct list_client *client)
{      
  struct list_client *ptr_app, *ptr_list, *ptr_el;
  ptr_list = client->next;
  ptr_el = ptr_list;
       
  while ( (ptr_list != client) && (ptr_list->connsd != x) )
    ptr_list = ptr_list->next;                 
       
  if (ptr_list == client)
    printf("%d ==> IP non presente nella lista\n", getpid() );

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
      ptr_list=ptr_list->next;
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


void decrementaTtlClient(struct list_client *client)
{
  int i = 0;
  struct list_client *ptr_list, *ptr_app;
  ptr_list = client;

  while (ptr_list->next != client)
  {
     ptr_list = ptr_list->next;
     ptr_list->TTL--;

     if (ptr_list->TTL == 0)
     {
         ptr_app = ptr_list->prev;
         cancellaClient(ptr_list->connsd, client);
         ptr_list = ptr_app;
     }

  }

  return;
}


void reimpostaTtlClient(int x, struct list_client *client)
{
  struct list_client *ptr_app, *ptr_list, *ptr_el;
  ptr_list = client->next;
  ptr_el = ptr_list;

  while ( (ptr_list != client) && ( ptr_list->connsd != x ) )
         ptr_list = ptr_list->next;

  if (ptr_list == client)
      printf("%d ==> connsd %d non presente in lista\n", getpid(), x);

  if (ptr_list->connsd == x)
      ptr_list->TTL= 5;

  return;
}


void aggiornaStatistiche(int x, int p, int sp, struct list_client *client)
{
  struct list_client *ptr_list;
  ptr_list = client->next;

  while ( (ptr_list != client) && ( ptr_list->connsd != x ) )
         ptr_list = ptr_list->next;

  if (ptr_list == client)
      printf("%d ==> connsd %d non presente in lista\n", getpid(), x);

  if (ptr_list->connsd == x)
  {
      ptr_list->num_peer = p;
      ptr_list->num_SP = sp;
      ptr_list->TTL = 5;
  }

  return;
}


void ottieniViciniNonPieni(char ip[], int *cont, struct list_client *client)
{
  char *app;
  long ip_vicino;
  int i, connsd, n;
  struct list_client *ptr_list;
  struct sockaddr_in vicino;
  socklen_t len = sizeof(vicino);
  ptr_list = client->next;
  *cont = 0;

  while (ptr_list != client)
  {

       if (ptr_list-> num_SP < NUMERO_MAX_SUPERPEER)
       {
          connsd = ptr_list->connsd;

          while (1)
          {

            if ( ( (n = getpeername(connsd, (struct sockaddr *)&vicino, &len) ) < 0) && (errno == EINTR) )
               continue;

            if ( (n < 0) && (errno != EINTR) )
            {
               perror("errore in getpeername:");
               exit(1);
            }

            else
               break;

          }

          ip_vicino = vicino.sin_addr.s_addr;
          app = (char *) &ip_vicino;

          for (i = 0; i < 4; i++)
          {
             ip[*cont] = * (app +i);
             *cont = *cont + 1;
          }

       }

       ptr_list = ptr_list->next;
  }

}


int getSuperPeerBassaOccupazione (struct list_client *client)
{
   struct list_client *ptr_list;
   ptr_list = client->next;
   
   while ( (ptr_list != client) && (ptr_list->num_peer > 20) )
       ptr_list = ptr_list->next;

   if (ptr_list != client)
     return ptr_list->connsd;

   else
     return -1;
}


/************************************************************/
//FUNZIONI LISTA PEER

void insPeer(struct sockaddr_in ind, BLOOM* bloom, double rating, struct list_peer *Peer)
{
    struct list_peer *ptr_list;
		if (searchPeer(ind, Peer) == 1)
		{
		//	struct list_peer *ptr_app, *ptr_list, *ptr_el;
  		ptr_list = Peer->next;
			

			while ( (ptr_list != Peer) && ( ptr_list->IP.sin_addr.s_addr != ind.sin_addr.s_addr ) )
				     ptr_list = ptr_list->next;

			if (ptr_list->IP.sin_addr.s_addr == ind.sin_addr.s_addr)
			{
				  ptr_list->TTL = 5;
					ptr_list->filtro = bloom;
					ptr_list->rating = rating;
			}	

		}

		else
		{		
		  struct list_peer *ptr_el;
		  ptr_list = Peer->next;
		  ptr_el = (struct list_peer *)malloc(sizeof(struct list_peer));
		  ptr_el->IP = ind;
		  ptr_el->TTL = 5;
		  ptr_el->filtro = bloom;
			ptr_el->rating = rating;
		  ptr_el->prev = Peer;
		  ptr_el->next = ptr_list;
		  ptr_list->prev = ptr_el;
		  Peer->next = ptr_el;
		}

}


void decrementaTtl(struct list_peer *Peer)
{
  int i = 0;
  struct list_peer *ptr_list;
  ptr_list = Peer;

  while (ptr_list->next != Peer)
  {
     ptr_list = ptr_list->next;
     ptr_list->TTL--;

     if (ptr_list->TTL == 0)
     {
         struct list_peer *ptr_app;
         ptr_app = ptr_list->prev;
         cancellaPeer(ptr_list->IP,Peer);
         ptr_list = ptr_app;
     }

  }

  return;
}


void reimpostaTtl(struct sockaddr_in x, struct list_peer *Peer)
{
  struct list_peer *ptr_list;
  ptr_list = Peer->next;

  while ( (ptr_list != Peer) && ( ptr_list->IP.sin_addr.s_addr != x.sin_addr.s_addr ) )
         ptr_list = ptr_list->next;

  if (ptr_list == Peer)
      printf("%d ==> IP %s non presente in lista\n", getpid(), inet_ntoa(x.sin_addr) );

  if (ptr_list->IP.sin_addr.s_addr == x.sin_addr.s_addr)
      ptr_list->TTL = 5;

  return;
}


void reimpostaRating(double rating, struct sockaddr_in x, struct list_peer *Peer)
{
  struct list_peer *ptr_list;
  ptr_list = Peer->next;

  while ( (ptr_list != Peer) && ( ptr_list->IP.sin_addr.s_addr != x.sin_addr.s_addr ) )
         ptr_list = ptr_list->next;

  if (ptr_list == Peer)
      printf("%d ==> IP %s non presente in lista\n", getpid(), inet_ntoa(x.sin_addr) );

  if (ptr_list->IP.sin_addr.s_addr == x.sin_addr.s_addr)
      ptr_list->rating = rating;

  return;
}

/* ricerca di un peer per IP nella lista, se presente restituisce 1 altrimenti 0 */
int searchPeer(struct sockaddr_in x, struct list_peer *Peer)
{
	int res = 0;
  struct list_peer *ptr_list;
  ptr_list = Peer->next;

  while ( (ptr_list != Peer) && ( ptr_list->IP.sin_addr.s_addr != x.sin_addr.s_addr ) )
         ptr_list= ptr_list->next;

  if (ptr_list == Peer)
      return 0;

  if (ptr_list->IP.sin_addr.s_addr == x.sin_addr.s_addr)
      return 1;

  return 0;
}


void cancellaPeer(struct sockaddr_in x, struct list_peer *Peer)
{      
  struct list_peer *ptr_app, *ptr_list, *ptr_el;
  ptr_list = Peer->next;
  ptr_el = ptr_list;
       
  while ( (ptr_list != Peer) && ( ptr_list->IP.sin_addr.s_addr != x.sin_addr.s_addr ) )
     ptr_list = ptr_list->next;                 
       
  if (ptr_list == Peer)
     printf("%d ==> IP %s non presente in lista\n", getpid() , inet_ntoa(x.sin_addr) );

  if (ptr_list->IP.sin_addr.s_addr == x.sin_addr.s_addr)
  {
     ptr_el=ptr_list;
     ptr_app=ptr_list->prev;
     ptr_app->next=ptr_list->next; 
     ptr_list=ptr_list->next;
     ptr_list->prev=ptr_app;
     free(ptr_el);       
  }

}


int listLenPeer(struct list_peer *Peer)
{
    int i = 0;
    struct list_peer *ptr_list;
    ptr_list = Peer->next;

    while (ptr_list != Peer)
    {
       ptr_list= ptr_list->next;
       i++; 
    }   

    return i;
}


void stampaPeer(struct list_peer *Peer)
{
   struct list_peer *ptr_list;
   ptr_list = Peer;
   int cont = 0;
   printf("%d ==> STAMPA LISTA PEER\n", getpid());

   while (ptr_list->next != Peer)
   {
      ptr_list = ptr_list->next;
      printf("elem [%d] ==> next= %lx, prev= %lx, IP= %s, RAT= %f\n", cont, (long)ptr_list->next, (long)ptr_list->prev, inet_ntoa(ptr_list->IP.sin_addr), ptr_list->rating );
      cont++;
   }

}


struct sockaddr_in *getPeer(int pos, struct list_peer *Peer)
{
   int i = 0;
   struct list_peer *ptr_list;
   ptr_list = Peer->next;
  
   while ( (ptr_list != Peer) && (i < pos) )
   {
      ptr_list = ptr_list->next;
      i++;
   }

   if (ptr_list == Peer)
      return NULL;

   return &(ptr_list->IP);
}


void riempiArray(BLOOM* filtri[], char ip[], struct list_peer *Peer)
{
   struct list_peer *ptr_list;
   ptr_list = Peer->next;
   int i = 0, cont = 0, j;
   long addr;
   char *app;

   while (ptr_list != Peer)
   {
    
       filtri[i] = ptr_list->filtro;
       addr = (long)ptr_list->IP.sin_addr.s_addr;
       app = (char *)&addr;

       for (j = 0; j < 4; j++)
       {
         ip[cont] = *(app + j);
         cont++; 
       }

       ptr_list = ptr_list->next;
       i++;
     
   }   

   return ;
}


/************************************************************/
//FUNZIONI LISTA list_head


void insTesta(struct sockaddr_in ind, struct list_head *lista)
{
  struct list_head *ptr_list, *ptr_el;
  ptr_list = lista->next;
  ptr_el = (struct list_head *)malloc(sizeof(struct list_head));
  ptr_el->IP = ind;
  ptr_el->prev = lista;
  ptr_el->next = ptr_list;
  ptr_list->prev = ptr_el;
  lista->next = ptr_el;
}





void stampa(struct list_head *lista)
{
  struct list_head *ptr_list;
  ptr_list = lista;
  printf("%d ==> STAMPA LISTA\n ", getpid() );
  int cont = 0;

  while (ptr_list->next != lista)
  {
    ptr_list = ptr_list->next;
    printf("elem [%d] ==> next= %lx, prev= %lx, IP= %s\n", cont, (long)ptr_list->next, (long)ptr_list->prev,inet_ntoa(ptr_list->IP.sin_addr));
    cont++;
  }

}

void cancella(struct sockaddr_in x, struct list_head *lista)
{      
  struct list_head *ptr_app, *ptr_list, *ptr_el;
  ptr_list = lista->next;
       
  while ( (ptr_list != lista) && ( ptr_list->IP.sin_addr.s_addr != x.sin_addr.s_addr ) )
    ptr_list = ptr_list->next;                 
       
  if (ptr_list == lista)
    printf("%d ==> IP non presente in lista\n", getpid() );

  if (ptr_list->IP.sin_addr.s_addr == x.sin_addr.s_addr)
  {
    ptr_el = ptr_list;
    ptr_app = ptr_list->prev;
    ptr_app->next = ptr_list->next; 
    ptr_list = ptr_list->next;
    ptr_list->prev = ptr_app;
    free(ptr_el);        
  }

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

  lista->next= lista;
  lista->prev= lista;
}


int listLen(struct list_head *lista)
{
  int i= 0;
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
    ptr_list=ptr_list->next;
    i++;
  }

  if (ptr_list == lista)
     return NULL;

  return &(ptr_list->IP);
}

int isPresentAddr(struct list_head *lista, long *ptr)
{
   /* return 1 if ptr is present, 0 otherwise */
   struct list_head *ptr_list;
   ptr_list=lista->next;
  
   while ( (ptr_list != lista) )
   {
      if( ptr_list->IP.sin_addr.s_addr == *(ptr) )
        return 1;
      ptr_list = ptr_list->next;
      
   }

  return 0;
}
  




/******* FUNZIONI LISTA QUERY ***********/


void insQuery (int id, struct sockaddr_in P, char* nome_file, struct list_query *query)
{
  struct list_query *ptr_list, *ptr_el;
  struct sockaddr_in app;
  socklen_t addrlen = sizeof(app);
  int i, connsd;
  ptr_list = query->next;
  ptr_el = (struct list_query *)malloc(sizeof(struct list_query));
  ptr_el->peer = P;
  ptr_el->ID = id;
  ptr_el->nome_file = nome_file;
  ptr_el->SP_contattati = listLenClient(&client);
  ptr_el->IP_ottenuti = 0;
  ptr_el->try_list.next = &(ptr_el->try_list);
  ptr_el->try_list.prev = &(ptr_el->try_list);
  ptr_el->done_list.next = &(ptr_el->done_list);
  ptr_el->done_list.prev = &(ptr_el->done_list);
  ptr_el->prev = query;
  ptr_el->next = ptr_list;
  ptr_list->prev = ptr_el;
  query->next = ptr_el;
  

}


void cancellaQuery (int id, struct list_query *query)
{      
  struct list_query *ptr_app, *ptr_list, *ptr_el;
  ptr_list = query->next;
  ptr_el = ptr_list;
       
  while ( (ptr_list != query) && ( ptr_list->ID != id ) )
    ptr_list = ptr_list->next;                 
       
  if (ptr_list == query)
    printf("%d ==> ID non presente in lista\n", getpid() );

  if (ptr_list->ID == id)
  {
     cancellaLista ( &(ptr_list->try_list));
     cancellaLista ( &(ptr_list->done_list));
     ptr_el = ptr_list;
     ptr_app = ptr_list->prev;
     ptr_app->next = ptr_list->next; 
     ptr_list = ptr_list->next;
     ptr_list->prev = ptr_app;
     free(ptr_el);        
  }
}


struct sockaddr_in *getPeerQuery (int id, struct list_query *query)
{
  int i = 0;
  struct list_query *ptr_list;
  ptr_list = query->next;
  
  while ( (ptr_list != query) && ( ptr_list->ID != id ) )
    ptr_list = ptr_list->next;

  if (ptr_list == query)
    return NULL;

  return &(ptr_list->peer);
}


int searchId (int id, struct list_query *query)
{
  struct list_query *ptr_list;
  ptr_list = query->next;
       
  while ( (ptr_list != query) && ( ptr_list->ID != id ) )
    ptr_list = ptr_list->next;                 
       
  if (ptr_list == query)
   return 0;

  else if (ptr_list->ID == id)
   return 1; 

}


void incrementaIpOttenuti (int id, int IP, struct list_query *query)
{
  struct list_query *ptr_list;
  ptr_list = query->next;
       
  while ( (ptr_list != query) && ( ptr_list->ID != id ) )
    ptr_list = ptr_list->next;                 
       
  if (ptr_list == query)
    printf("%d ==> ID non presente in lista\n", getpid() );

  else if (ptr_list->ID == id)
    ptr_list->IP_ottenuti += IP;
}


void incrementaSpContattati(int id, int SP, struct list_query *query)
{
  struct list_query *ptr_list;
  ptr_list = query->next;
       
  while ( (ptr_list != query) && ( ptr_list->ID != id ) )
    ptr_list = ptr_list->next;                 
       
  if (ptr_list == query)
    printf("%d ==> ID non presente in lista\n", getpid() );

  else if (ptr_list->ID == id)
    ptr_list->SP_contattati += SP;

}


int getIpOttenuti(int id, struct list_query *query)
{
  struct list_query *ptr_list;
  ptr_list = query->next;
       
  while ( (ptr_list != query) && ( ptr_list->ID != id ) )
    ptr_list = ptr_list->next;                 
       
  if (ptr_list == query)
    return -1;

  else if (ptr_list->ID == id)
    return ptr_list->IP_ottenuti;

}


int getSpContattati(int id, struct list_query *query)
{
  struct list_query *ptr_list;
  ptr_list = query->next;
       
  while ( (ptr_list != query) && ( ptr_list->ID != id ) )
    ptr_list = ptr_list->next;                 
       
  if (ptr_list == query)
    return -1;

  else if (ptr_list->ID == id)
    return ptr_list->SP_contattati;

}



int getIdFromPeer(struct sockaddr_in peer, struct list_query *query)
{
  struct list_query *ptr_list;
  ptr_list = query->next;
       
  while ( (ptr_list != query) && ( ptr_list->peer.sin_addr.s_addr != peer.sin_addr.s_addr ) )
     ptr_list = ptr_list->next;

  if (ptr_list == query)
     return -1;

  else if (ptr_list->peer.sin_addr.s_addr == peer.sin_addr.s_addr)
     return ptr_list->ID;

}


/******** FUNZIONE CRESTA **********/


void receiveBloomUDP (int sockfd, struct sockaddr_in addr)
{
	int uscita = 0;
  int n;
	socklen_t leng = sizeof(my_addr);
	packetWHHS* pck_whhs;
  packetUDP* pck;
 	socklen_t len = sizeof(addr);
	char* res; //stringa di riscontro da mandare dopo un operazione
	//essendo un processo del SP che deve processare le richieste rimarrà sempre attivo ciclando all'infinito
  char buff[DIM_MAX_PKT]; //buffer per la ricezione del messaggio
  int cont = 0, i;

  while (1)
  {

    if ( ((n = recvfrom(sockfd, buff, DIM_MAX_PKT, 0, (struct sockaddr *)&addr, &len)) < 0) && (errno == EINTR) )
      continue;

    if ( (n < 0) && (errno != EINTR) )
    {
      perror("errore in recvfrom del comando size\n");
      return;
    }

    else
      break;

  }

  res = "nak";    //se va tutto male res rimane cosi altrimenti viene settato ad ACK

  if ( (strncmp(buff, "join", 4) != 0) && (searchPeer(addr, &peer) == 1) )
     reimpostaTtl(addr, &peer);

  if ( (strncmp(buff,"whhs",4) == 0) || ( (strncmp(buff, "ack", 3) == 0) && ( *(buff + 3) != 'E') ) )
      pck_whhs = create_packet_whhs(buff);
 
  else
      pck = create_packet(buff);

  char* dati_result = (char *)malloc(800);
  *dati_result = '\0';

  //se il comando ricevuto è una join leggo e salvo il filtro di bloom
  if (strncmp(buff, "join", 4) == 0)
  {   
      if (joinUDP(addr, pck) == 1)
         res = "ack";
       
      stampaPeer(&peer);
  }			

  if (strncmp(buff, "elez", 4) == 0)
  {   
    appaddr = addr.sin_addr;
    struct in_addr bestip = bestPeer();
    char* sendbuff = create_buffer("nwsp", 0, "");      

    if(bestip.s_addr != 0)
    {
      struct sockaddr_in best;
      memset((void *)&best, 0, sizeof(best));
      best.sin_family = AF_INET;
      best.sin_addr.s_addr = bestip.s_addr;
      best.sin_port = htons(PEER_UDP_PORT);

      while (1)
      {

        if ( ( (n = sendto(sockfd, sendbuff, 8, 0, (struct sockaddr *)&best, sizeof(best))) < 0) && (errno == EINTR) )
          continue;

        if ( (n < 0) && (errno != EINTR) )
        {
          perror("errore in sendto");
          return ;
        }

        else
          break;
      }
    }
    else
    {
      sendbuff = create_buffer("crsp", 0, "");//CReate SuperPeer

      //inviare sendbuff a chi lo aspettava
      struct sockaddr_in app;
      memset((void *)&app, 0, sizeof(app));
      app.sin_addr = addr.sin_addr;
      app.sin_family = AF_INET;
      app.sin_port = htons(PEER_UDP_PORT);

      while (1)
      {

        if ( ( (n = sendto(sockfd, sendbuff, 8 + cont, 0, (struct sockaddr *)&app, sizeof(app))) < 0) && (errno == EINTR) )
          continue;

        if ( (n < 0) && (errno != EINTR) )
        {
          perror("errore in sendto");
          return ;
        }

        else
          break;

      }
    }
    return;
  }

  //gestione risposta ad un comando di elezione superpeer
  if ((strncmp(buff, "ackE", 4) == 0) || (strncmp(buff, "nakE", 4) == 0))// && (searchPeer(addr, &peer) == 1))
  {   
    packetUDP* pck;
    pck = create_packet (buff);

    if((pck->size) == 0)
    { //il messaggio va inoltrato solo nel caso non c'è il flag di switch    
      char* sendbuff;
      int cont = 0;

      if ((strncmp(buff, "ackE", 4) == 0))
      {
        long app = (addr.sin_addr.s_addr);
        char* app_str;
        int j;
        app_str = (char*)&app;
        cont = 4;
        sendbuff = create_buffer("crsp", cont, app_str);//CReate SuperPeer
      }

      else
        sendbuff = create_buffer("crsp", 0, "");//CReate SuperPeer

      //inviare sendbuff a chi lo aspettava
      struct sockaddr_in app;
      memset((void *)&app, 0, sizeof(app));
      app.sin_addr = appaddr;
      app.sin_family = AF_INET;
      app.sin_port = htons(PEER_UDP_PORT);

      while (1)
      {

        if ( ( (n = sendto(sockfd, sendbuff, 8 + cont, 0, (struct sockaddr *)&app, sizeof(app))) < 0) && (errno == EINTR) )
          continue;

        if ( (n < 0) && (errno != EINTR) )
        {
          perror("errore in sendto");
          return ;
        }

        else
          break;

      }
    }

    return;
  }


  //se il comando ricevuto è una leave cancello il filtro di bloom, eliminando di fatto la connesione tra peer e SP
  if (strncmp(buff, "leav", 4) == 0 )
  { 

     if (searchPeer(addr, &peer) == 1)
     {
        cancellaPeer(addr,&peer);
				res = "ack";
     }

     else
        printf ("%d => ERRORE: peer %s non connesso a questo SP\n", getpid(), inet_ntoa(addr.sin_addr));

     if (my_addr.sin_addr.s_addr == addr.sin_addr.s_addr) 
       uscita = 1;

     else 
     {
       uscita = 0;

       if (listLenPeer(&peer) < 20)
       {
         int connsd, i, num_client;
         num_client = listLenClient(&client);
         char* app = (char *)&rating;
         connsd = getSuperPeerBassaOccupazione(&client);
        
         if ( connsd > 0)
         {
           char *sendbuff = create_buffer_TCP("mrge", sizeof(double), app);
           writen(connsd, sendbuff, 8 +sizeof(double));
         }
        
       }
     }
  }

  //se il comando ricevuto è un ping
  if ( (strncmp(buff, "ping", 4) == 0) )
  {

    if (searchPeer(addr, &peer) == 1)
    {
      /* leggo il rating*/
      double* rat;
      char* char_rat;
			char_rat = malloc(sizeof(double));

      for (i = 0; i < sizeof(double); i++)
          *( char_rat + i ) = *( pck->dati + i );

      rat = (double*) char_rat;
      reimpostaRating(*rat, addr, &peer);
      
      /* se è il superpeer stesso a pingarsi tengo traccia del rating in una variabile apposita */
      if(my_addr.sin_addr.s_addr == addr.sin_addr.s_addr)
        rating = *rat;
     

      if(((*rat) > (rating * 1.2)) && (my_addr.sin_addr.s_addr != addr.sin_addr.s_addr))
      {
        char* sendbuff = create_buffer("pong", 0, ""); 
        //per differenziare il caso di elezione del SP da quello dello switch inserisco un "flag" si switch

        while (1)
        {
          n = sendto(sockfd, sendbuff, 8 , 0, (struct sockaddr *)&addr, sizeof(addr) );
          if ( ( n < 0) && (errno == EINTR) )
             continue;

          if ( (n < 0) && (errno != EINTR) )
          {
             perror("errore in sendto pong");
             return ;
          }  

          else
             break;
        }
        
        /* messaggio al server di bootstrap */
        int bootsd, n;
        struct sockaddr_in boot;
        memset((void *)&boot, 0, sizeof(boot));
        boot.sin_addr = bootaddr;
        boot.sin_family = AF_INET;
        boot.sin_port = htons(SERV_TCP_PORT);

        while (1)
        {
           if ( ( (bootsd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) && (errno == EINTR) )
              continue;
           if ((bootsd < 0) && (errno != EINTR) )
           {
              perror("errore in socket");
              return;
           }
           else
             break;
        }

        while (1)
        {
          if ( ( (n = connect (bootsd, (struct sockaddr *) &boot, sizeof(boot)) ) < 0 ) && (errno == EINTR) )
             continue;

          if ( (n < 0 ) && (errno != EINTR) )
          {
             perror("errore in connect");
             return;
          }

          else
            break;

        }

        if (writen(bootsd, "leave\n", 6) != 6)
           printf("errore write in leave\n");

        //trovo il best peer, sarebbe dannoso fare in continuazione switch tra SP
        int num_peer = listLenPeer(&peer);
        char *ip_best_peer;
        struct sockaddr_in *peer_figli, best;
        best.sin_addr = bestPeer();
        best.sin_port = htons(PEER_UDP_PORT);   
        best.sin_family = AF_INET;
        ip_best_peer = (char *)&best.sin_addr.s_addr;
        sendbuff = create_buffer("nwsp", 1, "s"); 
        //per differenziare il caso di elezione del SP da quello dello switch inserisco un "flag" si switch
        if(best.sin_addr.s_addr != 0)
        {
          while (1)
          {
            n = sendto(sockfd, sendbuff, 9 , 0, (struct sockaddr *)&best, sizeof(best) );
            if ( ( n < 0) && (errno == EINTR) )
               continue;

            if ( (n < 0) && (errno != EINTR) )
            {
               perror("errore in sendto nwsp");
               return ;
            }

            else
               break;
          }
          
          sendbuff = create_buffer("leav", 4, ip_best_peer);
        }
        else
          sendbuff = create_buffer("leav", 0, "");        
        
        for (i = 0; i < num_peer; i++)
        {
          peer_figli = getPeer(i, &peer);
          if (peer_figli != NULL)
          {

             if ((peer_figli->sin_addr.s_addr != best.sin_addr.s_addr))
             {

               while (1)
               {
                  n = sendto(sockfd, sendbuff, 12, 0, (struct sockaddr *)peer_figli, ((socklen_t)sizeof(*peer_figli)) );
                  if ( ( n < 0) && (errno == EINTR) )
                    continue;

                  if ( (n < 0) && (errno != EINTR) )
                  {
                    perror("errore in sendto peerfigli");
                    return ;
                  }

                  else
                    break;
                  
               }
             }
          }
        }

        sleep(1); 
        exit(0);
      }
    }

    res = "pong";
  }
  
  //se il comando ricevuto è una whohas determino chi ha il file e lo comunico al peer
  if ((strncmp(buff, "whhs", 4) == 0) && (pck_whhs->id == 0) && (searchPeer(addr, &peer) == 1 ) )
  {
     char *buffer = create_buffer_whhs(pck_whhs->cmd, pck_whhs->size, id_whohas, pck_whhs->dati);
     insQuery (id_whohas, addr, pck_whhs->dati, &lista_query);
     id_whohas = ((id_whohas) % 65536) + 1;
     struct sockaddr_in vicino_contattato;
     socklen_t len = sizeof(vicino_contattato);
     int n;
     struct list_query *ptr_list;
     ptr_list = lista_query.next;

     for (i = 0; i < listLenClient(&client); i++) //PROPAGO LA WHOHAS AI MIEI VICINI
     {

         if (writen(getConnsd(i, &client), buffer, pck_whhs->size + 13) < 0)
         {
            perror("errore nella propagazione del whohas tra SP:");
            continue;
         }

         while (1)
         {
            if ( ( (n = getpeername(getConnsd(i, &client), (struct sockaddr *) &vicino_contattato, &len)) < 0) && (errno == EINTR) )
               continue;

            if (  (n < 0) && (errno != EINTR) )
            {
               perror("errore in get peer:");
               exit(0);
            }   

            else
               break;
         }

         insTesta(vicino_contattato, &(ptr_list->done_list));
     }

     
     if (whoHasUDP (pck_whhs, dati_result, &cont) >= 0)  
       res = "ack";

     char* sendbuff = create_buffer_whhs(res, cont, 0, dati_result);

     while (1)
     {
       n = sendto(sockfd, sendbuff, 13 + cont, 0, (struct sockaddr *)&addr, sizeof(addr));

       if ( ( n < 0) && (errno == EINTR) )
         continue;

       if ( (n < 0) && (errno != EINTR) )
       {
         perror("errore in sendto");
         return ;
       }

       else
         break;

     }

     return;
  }

  else if ((strncmp(buff, "ack", 3) == 0) && (pck_whhs->id != 0) && (searchId(pck_whhs->id, &lista_query) == 1 ))
  {
    cont = parsingWhoHas(pck_whhs, dati_result);
    struct sockaddr_in *addrPeer = getPeerQuery(pck_whhs->id, &lista_query);
    res = "ack";
    char* sendbuff = create_buffer_whhs(res, cont, pck_whhs->id, dati_result);

    while(1)
    {

      n = sendto(sockfd, sendbuff, 13 + cont, 0, (struct sockaddr *)addrPeer, sizeof(*addrPeer));
      if ( (n < 0) && (errno == EINTR) ) 
        continue; 

      if ( (n < 0) && (errno != EINTR) ) 
      {
        perror("errore in sendto");
        return ;
      }

      break;
    }

    if ( (getIpOttenuti(pck_whhs->id, &lista_query) >= MAXPEER )|| (getSpContattati(pck_whhs->id, &lista_query) >= MAXSUPERPEER) )
    {
      cancellaQuery(pck_whhs->id, &lista_query);
      free(sendbuff);
      sendbuff = create_buffer("stop", 0, "");

      while(1)
      {
         n = sendto(sockfd, sendbuff, 8, 0, (struct sockaddr *) addrPeer, sizeof(addrPeer));

         if ( (n < 0) && (errno == EINTR) ) 
           continue; 

         if ( (n < 0) && (errno != EINTR) ) 
         {
           perror("errore in sendto");
           return ;
         }

         break;
      
    }

    }
    return;
  }

//GESTIONE WHHS RICEVUTA DA ALTRI SUPERPEER IN UDP 
/***********************************************************************/

  else if ( (strncmp(buff, "whhs", 4) == 0) && (pck_whhs->id != 0) )
  {
    int byte_IP_P = 0;
    int *e;
    struct in_addr apps;
     
    if (whoHasUDP(pck_whhs, dati_result, &byte_IP_P) > 0)
    {
      e = (int*) (dati_result);
      apps.s_addr = *e;
    }

    *(dati_result + byte_IP_P) = ';';
    e = (int*) (dati_result);
    apps.s_addr = *e;
    int j, byte_IPSP = 1;
    struct sockaddr_in app;
    socklen_t addrlen = sizeof(app);

    for (i= 0; i < listLenClient(&client); i++)
    {

      if (getConnsd(i, &client) != sockfd)
      {

        while(1)
        {

          if ( (getpeername(getConnsd(i, &client),(struct sockaddr *) &app, &addrlen) < 0) && (errno == EINTR) )
             continue;

          else if ( (getpeername(getConnsd(i, &client),(struct sockaddr *) &app, &addrlen) < 0) && (errno != EINTR) )
          {
             perror("bad getpeername on superpeer\n");
             return ;
          }

          else
             break;

        }

        char *addr = (char *) &app.sin_addr.s_addr;
        j = 0;

        for (j = 0; j < 4; j++)
        {
          *(dati_result + byte_IPSP + byte_IP_P ) = *(addr + j );
          byte_IPSP++;
        }

      }

   }
              
   if (byte_IPSP != 1)
   {
     e = (int*) (dati_result + 5);
     apps.s_addr = *e;
   }

   char* sendbuff= create_buffer_whhs("ack\0", byte_IPSP + byte_IP_P, pck_whhs->id, dati_result);


   if (sendto(sockfd, sendbuff, 13 + byte_IPSP + byte_IP_P, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
   {
     perror("errore in sendto");
     return ;
   }

   return;
 }


  else if ( (strncmp(buff, "stop", 4) == 0) )
  {
    //GESTIONE STOP RICERCA RICEVUTA DAL PEER IN UDP
    int n, id;
  
    if ((id = getIdFromPeer(addr, &lista_query) ) >= 0)
    {
      cancellaQuery(id, &lista_query);
      res = "ack"; //riscontro positivo in quanto è andato tutto bene
    }

    else
    {
      int a = searchPeer (addr , &peer);
      if(a != 1)
        res = "nak"; //restituisco nak solo se il peer non è connesso a me
      else
        res = "ack"; //ack in quanto, anche se non aveva una query salvata il messaggio di stop è stato processato
    }

  }

  char* sendbuff = create_buffer(res, cont, dati_result);

  if (sendto(sockfd, sendbuff, 8 + cont, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)   //finito di processare l'operazione invio il riscontro
  {
     perror("errore in sendto");
     return ;
  }



	if (uscita == 1)
	{
    int bootsd, n;
		//notifica alla rete che si sta sconnettendo
    struct sockaddr_in boot;
    memset((void *)&boot, 0, sizeof(boot));
    boot.sin_addr = bootaddr;
    boot.sin_family = AF_INET;
    boot.sin_port = htons(SERV_TCP_PORT);

    while (1)
    {
       if ( ( (bootsd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) && (errno == EINTR) )
          continue;

       if ((bootsd < 0) && (errno != EINTR) )
       {
          perror("errore in socket");
          return;
       }

       else
         break;

    }

    while (1)
    {
      if ( ( (n = connect (bootsd, (struct sockaddr *) &boot, sizeof(boot)) ) < 0 ) && (errno == EINTR) )
         continue;

      if ( (n < 0 ) && (errno != EINTR) )
      {
         perror("errore in connect");
         return;
      }

      else
        break;

    }

    if (writen(bootsd, "leave\n", 6) != 6)
       perror("errore write in leave\n");

    //Comunico ai miei peer ke mi stò scollegando e nel campo dati del pacchetto dò l'indirizzo del nuovo super peer (i super peer lo capiscono da soli tramite select)
    int num_peer = listLenPeer(&peer);
    char *ip_best_peer;
    struct sockaddr_in *peer_figli, best;
    best.sin_addr = bestPeer();
    best.sin_port = htons(PEER_UDP_PORT);   
    best.sin_family = AF_INET;
    ip_best_peer = (char *)&best.sin_addr.s_addr;

    if(best.sin_addr.s_addr != 0)
    {
      char* sendbuff = create_buffer("nwsp", 1, "s");

      
      while (1)
      {

        if ( ( (n = sendto(sockfd, sendbuff, 9, 0, (struct sockaddr *)&best, sizeof(best) ) ) < 0) && (errno == EINTR) )
           continue;

        if ( (n < 0) && (errno != EINTR) )
        {
           perror("errore in sendto");
           return ;
        }

        else
           break;

      }

      sendbuff = create_buffer("leav", 4, ip_best_peer);
    }
    else 
      sendbuff = create_buffer("leav", 0, "");

    for (i = 0; i < num_peer; i++)
    {

      if ((peer_figli = getPeer(i, &peer)) != NULL)
      {

         if (peer_figli->sin_addr.s_addr != best.sin_addr.s_addr)
         {

           while (1)
           {

              if ( ( (n = sendto(sockfd, sendbuff, 12, 0, (struct sockaddr *)peer_figli, sizeof(peer_figli))) < 0) && (errno == EINTR) )
                continue;

              if ( (n < 0) && (errno != EINTR) )
              {
                perror("errore in sendto");
                return ;
              }

              else
                break;

           }

         }

      }

    }

    alarm(0);
    exit(0);
	}

  //destroy_packet(pck);	
  return ;
}


int joinUDP(struct sockaddr_in addr, packetUDP *pck)
{
    int search = searchPeer ( addr , &peer );
    int count = listLenPeer(&peer);

    if ( (search == 0) && (count >= NUMERO_MAX_PEER) )
       return 0;

    else
    {			
         //inserimento dell'ip e del filtro
         char* a;
				 a = malloc(real_dim_filter);
         int i;

				 for (i = 0; i < real_dim_filter; i++)
            *( a + i )  = *( pck->dati + i );

				 double* rat;
				 char* char_rat;
				 char_rat = malloc(sizeof(rat));

				 for (i = real_dim_filter; i < pck->size; i++)
						*( char_rat + i - real_dim_filter ) = *( pck->dati + i );

				 rat = (double*) char_rat;
         if(my_addr.sin_addr.s_addr == addr.sin_addr.s_addr)
           rating = *rat;


         BLOOM* bloom = create_bloom_struct(a);
         insPeer(addr, bloom, *rat, &peer);
         return 1;
    }

    return 0;
}

int whoHasUDP(packetWHHS *pck, char *dati_result, int *cont)
{
   int num_ip = 0;
   char *ind_ip;
   char *nome_file = (char *)malloc(strlen(pck->dati) );
   strncpy(nome_file, pck->dati, strlen(pck->dati));
   strcat(nome_file,"\0");
   struct in_addr app;
   int size = listLenPeer (&peer);
   BLOOM* filtri[size]; //carico i filtri di bloom presenti
   size = size << 2;
   char ip_peer[size]; //tengo traccia degli IP in modo tale che so quale peer ha il file
   size = size >> 2;
   int i, j = 0;
   //scorro i vari peer nel SP
   riempiArray(filtri, ip_peer, &peer);

   ind_ip = ip_peer;
   struct in_addr appoggio;

   //li interrogo cosi da sapere chi ha il file
   for (i = 0; i < (size); i++)
   {
      //controllo se nel filtro è contenuto il file cercato in caso positivo tengo traccia degli ip
      if (bloom_check(filtri[i], nome_file) ) 
      {

         //vengono salvati in una stringa dati_result con sintassi ip1ip2 ...ipn;
         for (j = 0; j < 4; j++)
         {
             *(dati_result + (*cont)) = *(ip_peer + 4*i + j);
              (*cont)++;
         }
					
				 appoggio.s_addr = *((long*)(dati_result + 4*i));
         num_ip++;
      }

      ind_ip += 4;
   }

   if (num_ip == 0)
      dati_result = "\0";  // se non andava salvato nessun ip lo imposto a stringa vuota

   
   for (i = 0; i < num_ip; i++)
   {

     long *e;
		 e = (long*) (dati_result+(i*4));
		 struct in_addr apps;
		 apps.s_addr = *e;
   }

   return num_ip;
}



//***********FOJA E SUPERPEER GENERICHE***************//


int pingSpUDP (struct in_addr IP) //controlla l'attività in rete del superpeer
{
  fflush (stdout);
  
	int sockfd; //descriptor della socket
  struct sockaddr_in servaddr; //riferimento all'IP
	socklen_t iplen = sizeof(servaddr); //dimensione della struttura sockaddr_in

  //creazione della socket udp
  if ((sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {				
      perror ("errore in socket");
			return 0;
    }

	memset((void *)&servaddr, 0, sizeof(servaddr)); //azzera servaddr
  servaddr.sin_family = AF_INET; //assegnazione del tipo di indirizzo
  servaddr.sin_port = htons(SERV_UDP_PORT); //assegnazione della porta del server
  //assegnazione dell'indirizzo del SP passato dal server di bootstrap.
	servaddr.sin_addr = IP;

	char* sendbuff = create_buffer("ping", 0, "");
	
	//invio pacchetto
  if (sendto(sockfd, sendbuff, 8, 0, (struct sockaddr *)&servaddr, iplen) < 0) 
  {
      perror("errore in sendto");
      return 0;
   }	


   return 1; //restituisco 1 se l'invio del ping è andato a buon fine

  //return 0;
}


int pingSpTCP () //controlla l'attività in rete del superpeer
{
  int n;

  if ( (n = listLenClient(&client) ) == 0)
     return 0;

  int i = 0, num_peer, num_SP;
  char dati[8], *p, *sp;
  num_peer = listLenPeer(&peer);
  num_SP = listLenClient(&client);
  p = (char *) &num_peer;
  sp = (char *) &num_SP;

  for (i = 0; i < 4; i++)
  {
     dati[i] = *(p + i);
     dati[4+i] = *(sp + i);
  }

  char *sendbuff = create_buffer_TCP("ping", 8, dati);
  
  for (i = 0; i < n; i++) //SCORRO TUTTA LA LISTA DEI VICINI E GLI MANDO IL PING CON L'AGGIORNAMENTO SULLE PROPIE STATISTICHE
     writen(getConnsd(i, &client), sendbuff, 16);
     

    return 1;
}


int connessioneOverlay(int *max, fd_set *allset)
{

  if (listLen(&list) == 0)
  {  
     return 0;
  }

  int dim = listLen(&list);
  struct in_addr array_ip[dim],ip;              //creo array di ip di dimensione della lista
  float ping[dim];                              //conterrà il valore dei ping dei SP pingati
  int join = 0, cont = 0;                       //contatori
  struct list_head *ptr_list;
  ptr_list = &list;
	
  while ( (ptr_list->next != &list) && (cont < (NUMERO_MAX_SUPERPEER >> 1)) ) 
  {
    ptr_list = ptr_list->next;
    join = sendJoinTCP(ptr_list->IP.sin_addr,max, allset, cont);
    cont += join;
  }

  return cont;  
}


int sendJoinTCP(struct in_addr ip, int *max, fd_set *allset, int cont)
{
   int socksd, join = 0, n_peer, n_s_peer, i, n;
   int *p, *sp;
   char sendline[8], recvline[25], *c1, *c2;
   struct sockaddr_in SP_addr;

   while (1)
   {

     if ( ((socksd = socket(AF_INET, SOCK_STREAM, 0)) < 0) && (errno == EINTR) )
         continue; 

     if ( ( socksd < 0) && (errno != EINTR) ) 
     {
       perror("errore in socket");
       return 0;
     }

     else
      break;

   }

   memset((void *)&SP_addr, 0, sizeof(SP_addr));
   memset((void *)&recvline, 0, sizeof(recvline));
   memset((void *)&sendline, 0, sizeof(sendline));
   SP_addr.sin_family = AF_INET;
   SP_addr.sin_port = htons(SP_TCP_PORT);
   SP_addr.sin_addr = ip;

   while (1)
   {
     n = connect (socksd, (struct sockaddr *) &SP_addr, sizeof(SP_addr) );

     if ( (n < 0) && (errno == EINTR) )
        continue;

     if ( (n < 0) && (errno != EINTR) )
     {
       perror("errore in connect TCP:");
       return 0;
     }

     else
       break;     
   }

   if (my_addr.sin_addr.s_addr != SP_addr.sin_addr.s_addr)
   {
     
     if (readline(socksd, recvline, 5) < 0)
     {
       perror("errore in readline");
       return 0;
     } 

     if (strcmp(recvline, "full\n") == 0)
     {
       int j, ip_restituiti;

       while (1)
       {

          if ( ( (n = read(socksd, recvline, 4)) < 0) && (errno == EINTR) )
             continue;

         if (  (n  < 0) && (errno != EINTR) )
         {
            perror("errore in read");
            exit(1);
         }

         else
            break;
       }

       n = *(int *)recvline;

       if (n == 0)
         return 0;

       while (1)
       {

          if ( ( (n = read(socksd, recvline, n)) < 0) && (errno == EINTR) )
             continue;

         if (  (n  < 0) && (errno != EINTR) )
         {
            perror("errore in read");
            exit(1);
         }

         if (n == 0)
           return 0;

         else
            break;
       }

       struct in_addr vicino;
       ip_restituiti = n/4;

       for (j = 0; j < ip_restituiti; j++)
       {
          vicino.s_addr = * ((long *)&recvline[j*4]);
          if (cont < 3)
             cont += sendJoinTCP(vicino, max, allset, cont);

          if (cont == 3)
            return cont;
       }

       
       return cont;
     }

     if (strcmp(recvline,"ack\n") == 0)
     {

       while(1)
       {

         if ( (n < 0) && (errno == EINTR) )
           continue;

         if ( ((n = read(socksd, recvline, 8)) < 0) && (errno != EINTR) )
         {
           fprintf(stderr, "errore in readline");
           return 0;
         }

         break;
       }

       p = (int *)recvline;
       sp = (int *)&recvline[4];
       *p = ntohl(*p);
       *sp = ntohl(*sp);
       n_peer = listLenPeer(&peer);
       n_s_peer = listLenClient(&client);
       n_peer = htonl(n_peer);
       n_s_peer = htonl(n_s_peer);
       c1 = (char *)&n_peer;
       c2 = (char *)&n_s_peer;

       for (i = 0; i < 4; i++)
       {
         sendline[i] = *(c1 + i);
         sendline[4+i] = *(c2 + i);
       }


       if (writen(socksd,sendline,8) < 0)
       {
         cancellaClient(socksd, &client);
         return 0;
       }

       inserimento(socksd, *p, *sp, &client);
       FD_SET(socksd, allset);

       if (socksd > *max)
          *max = socksd;
   }

   stampaClient(&client);
   return 1;
 }

 else //ho fatto la connect a me stesso non và bene chiudo la socksd
 {

   while (1)
   {

     if ( ((n = close(socksd)) < 0) && (errno == EINTR) )
        continue;

     if ( (n < 0) && (errno != EINTR) )
     {
       perror("errore in close:");
       return -1;
     }

     break;

   }

   return 0;
 }

}


/* funzione per il register del superpeer al server di bootstrap, il SP che si registra al bootstrap riceve una lista di IP di altri SP */
/* il valore di ritorno è 1 se è andato tutto bene */
int registerSP(char* str_ip)
{
  char sendline[10], recvline[100];
  int *len, n, cont;
  long *ip;
  struct sockaddr_in IP;
  int sockfd;
  struct sockaddr_in servaddr;
  socklen_t slen;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
     perror("errore in socket");
     return -1;
  }

  memset((void *)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERV_TCP_PORT);

  if (inet_pton(AF_INET, str_ip , &servaddr.sin_addr) <= 0)
  {
     fprintf(stderr, "errore in inet_pton per %s", str_ip);
     return -1;
  }

  if (connect (sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
  {
     perror("errore in connect al server");
     return -1;
  }

  strcpy(sendline, "register\n");
  
  if ( writen(sockfd, sendline, strlen(sendline)) < 0)
  {
     fprintf(stderr, "errore in write");
     return -1;
  }

  if (readline(sockfd, recvline, 6) < 0)
  { 
     perror("errore readline:");
     return -1;
  }

  if (strcmp(recvline, "empty") == 0)
  {
     slen = sizeof(my_addr);
     getsockname(sockfd, (struct sockaddr *)&my_addr, &slen);
  }

  if (strcmp(recvline, "ack\n") == 0)
  {
     memset((void *)&IP, 0, sizeof(IP));
     IP.sin_family = AF_INET;
     memset((void *)recvline, 0, sizeof(recvline));
     int k = 0;

     if (read(sockfd, recvline, 4) == 4)
     {
        len = (int *)recvline;
     }

     char *str;
     cont = 0;
     str = (char *)malloc(*len);
     slen = sizeof(my_addr);
     getsockname(sockfd, (struct sockaddr *)&my_addr, &slen);

     if ( (n = read(sockfd, str, *len) ) < 0)
     {
         perror("errore in read:");
         exit(1);
     }

       for (k = 0; k < n; k += 4)
       {
          ip = (long *)(str+k);
          IP.sin_addr.s_addr = *ip;

          if (my_addr.sin_addr.s_addr != IP.sin_addr.s_addr)
            insTesta(IP,&list);

          cont++;
       }

     free(str);
     }

     stampa(&list);
  

  if (close(sockfd) == -1)
  {
     perror("errore in close");
     return -1;
  }

  return 1;
}


int parsingWhoHas(packetWHHS *pck, char *dati_result)
{
  int cont = 0;
  struct list_query *ptr_list;
  ptr_list = lista_query.next;
       
  while (ptr_list->ID != pck->id)
      ptr_list = ptr_list->next; 

  if (pck->dati == '\0')
    return 0;
   
  int j, i;
  char *data = pck->dati;
  int size = pck->size;

  for (j = 0; j < size; j++)
  {

    if (*(data + j) != ';')
    {
      *(dati_result + j) = *(data + j);
      cont++;
    }
    
    if(*(data + j) == ';')
    { 
      j++;
      i = j;
      break;
    }
    
   }

   incrementaSpContattati(pck->id, 1, &lista_query);
   incrementaIpOttenuti(pck->id, cont >> 2, &lista_query);
   long *ptr;
   struct sockaddr_in sckapp;
  
   for(i; i < pck->size; i += 4)
   {
     ptr = (long *) (data+i);

     //verifico se ptr è presente in try o done list  
     if ( (isPresentAddr( &(ptr_list->try_list), ptr) == 0) || (isPresentAddr( &(ptr_list->done_list), ptr) == 0) ) 
     {
       sckapp.sin_addr.s_addr = *ptr;
       sckapp.sin_port = htons(SP_UDP_PORT);  
       sckapp.sin_family = AF_INET;
       insTesta(sckapp, &(ptr_list->try_list));
     }

   }

  return cont;
}


int inoltraWhoHas(int id, int sockfd)
{
    if (searchId(id, &lista_query) == 1)
    {
       int i, n;
       char* sendbuff;
       int num_elementi; //numero elementi presenti nella try_list abbinata alla query
       struct sockaddr_in super_peer;
       struct list_query *ptr_list;
       ptr_list = lista_query.next;
       
       while (ptr_list->ID != id)
           ptr_list = ptr_list->next;

       num_elementi = listLen(&(ptr_list->try_list));
       sendbuff = create_buffer_whhs ("whhs", strlen(ptr_list->nome_file), id, ptr_list->nome_file);

       for (i = 0; i < num_elementi ; i++)
       {
          //MANDO MESSAGGIO DELLA WHOHAS IN UDP A TUTTI I SUPER PEER IN TRY LIST E SPOSTARLI IN DONE LIST
          super_peer = *getList(i, &(ptr_list->try_list));

          while (1)
          {

             if ( ( (n= sendto(sockfd, sendbuff, 13 + strlen(ptr_list->nome_file), 0, (struct sockaddr *)&super_peer, sizeof(super_peer))) < 0) && (errno == EINTR) )
              continue;

             if ( (n < 0) && (errno != EINTR) )
             {
                perror("errore in sendto");
                return ;
             }

             else
              break;
          }

          incrementaSpContattati(id, 1, &lista_query);
          cancella(super_peer, &(ptr_list->try_list));
          insTesta(super_peer, &(ptr_list->done_list));
       }

    }
}


/* 
FUNZIONI PER LA GESTIONE DINAMICA DELLA RETE 
*/

/* restituisce il miglior peer connesso ad esso */
struct in_addr bestPeer ()
{
  struct list_peer *ptr_list;
  ptr_list = &peer;
  double rating_max = 0;
  struct in_addr best_peer;
  best_peer.s_addr = 0;

  while (ptr_list->next != &peer)
  {

    if (((ptr_list->IP).sin_addr.s_addr) != my_addr.sin_addr.s_addr)
    {
      ptr_list = ptr_list->next;

      if(rating_max < ptr_list->rating)
      {
        rating_max = ptr_list->rating;
        best_peer = (ptr_list->IP).sin_addr;
      }

    }
  }

  return best_peer;
}

void parseSuperPeerVar ()
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
        //salto i spazi tra il nome della variabile e il suo valora
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
        SERV_TCP_PORT = atoi(val_var);        
      }
      if( strcmp( nome_var, "BOOT_PORT_UDP") == 0)
      {
        SERV_UDP_PORT = atoi(val_var);        
      }
      if( strcmp( nome_var, "MAXPEER") == 0)
      {
        MAXPEER = atoi(val_var);        
      }
      if( strcmp( nome_var, "MAXSUPERPEER") == 0)
      {
        MAXSUPERPEER = atoi(val_var);        
      }
    }
  }
}

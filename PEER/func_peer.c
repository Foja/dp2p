#include "peer.h"

/*funzione per il sigaction da associare al timeout*/
void alarm_handler()
{
	printf("\n%d =>\n *****************\ntimeout scaduto\n*****************\n", getpid());
	var_ack = 0;
	return;
}


void Superpeer()
{
  char *arglist1[3] = {"./superpeer", ip_server, NULL};
	execve(arglist1[0], arglist1, NULL);
}

/*invia il filtro di bloom in udp*/
/*RETURN VALUE
 * 1 success
 * 0 insuccess
*/
int join_UDP (int sockfd, struct in_addr IP, double rating , int cont)
{
  alarm(0);
	if (cont == 0) return 0; //se il contatore delle chiamate ricorsive è arrivato fino a zero mi fermo e restituisco false
	char* PATH = PATH_FILEBLOOM;  
	char result[DIM_MAX_PKT];
  
	printf ("\n%d => JOIN (%d) IP=%s\n", getpid(), cont, inet_ntoa(IP));
  
  struct sockaddr_in appaddr; //riferimento all'IP
	int iplen = sizeof(appaddr); //dimensione della struttura sockaddr_in

	memset((void *)&appaddr, 0, sizeof(appaddr)); 
  appaddr.sin_family = AF_INET; 
  appaddr.sin_port = htons(SP_UDP_PORT); 
	appaddr.sin_addr = IP;

  //file descriptor del filtro di bloom da leggere ed inviare
  FILE* fd;
  //apro il file
	if(!(fd = fopen(PATH, "rb"))) 
  {
		fprintf(stderr, "ERROR: Could not open file %s\n", PATH);
		return 0;
	}

	//leggo il filtro di bloom
	char* filter_buf = malloc(real_dim_filter + sizeof(rating));
	if((fread(filter_buf,real_dim_filter, 1, fd)) == 1 )
	{
		//leggo e salvo i dati letti nel buffer		
	}
 	else 
	{
		perror("errore in read lettura filtro");
		return 0;	
	}
	
	int j;
	for( j = 0; j < sizeof(rating); j++)	
    *(filter_buf + real_dim_filter + j) = * (((char *) (&rating)) + j);

	char* sendbuff = create_buffer("join", real_dim_filter + sizeof(rating), filter_buf);
	
	//invio pacchetto
	if (sendto(sockfd, sendbuff, 8 + real_dim_filter + sizeof(rating), 0, (struct sockaddr *)&appaddr, iplen) < 0)
  {
      perror("errore in sendto della join");
      return 0;
   }	

  //ricevo il risultato dell'operazione

  //rendo "sicura" la trasmissione
  //associo tramite la sigaction la mia funzione alarm_handler al segnale SIGALRM
  struct sigaction alarmaction = {.sa_handler = alarm_handler};
  int retcode = sigaction(SIGALRM, &alarmaction, NULL);

  if (retcode == -1) 
  {
    perror("bad sigaction()\n");
    return 0;
  }

	alarm(6-cont);	//il timeout aumenta nelle chiamate ricorsive successive
	var_ack = 1;
	int received = 0;
	while(var_ack) //provo a ricevere finche non va a buon fine e finche non vado in timeout
	{
		//la recvfrom è non bloccante
		if ((recvfrom (sockfd, result, DIM_MAX_PKT, MSG_DONTWAIT , (struct sockaddr *)&appaddr, (socklen_t *)&iplen )) >= 3)
		{
			var_ack=0;
			received=1;
		}
	}

	//chiudo i descriptor
  if (fclose(fd) < 0)
     perror("errore in close");
	
	if (received == 1)
	{
		packetUDP* pck;
		pck=create_packet(result);
		printf("%d => cont=%d RESULT=> %s\n",getpid(),cont,pck->cmd);//stampo il risultato dell'operazione

		if (strcmp(pck->cmd,"nak") == 0)
		{	//se l'operazione è andata male riprovo per un numero fissato di volte (fino a che il contatore va a zero)
			return join_UDP(sockfd, IP, rating, cont-1);
		}
		if (strcmp(pck->cmd,"ack") == 0)
		{	//se l'operazione è andata bene restituisco 1
      alarm(0);
			return 1;
		}	
	}
	else
	{
		printf("%d => cont=%d RESULT=> nessuna risposta\n",getpid(),cont);//stampo il risultato dell'operazione	
		return join_UDP(sockfd, IP, rating, cont-1);
	} 
  alarm(0);
	return 0;	
}


int ping_UDP (int sockfd, struct in_addr IP ,int cont) //in questo caso cont è il timeout in secondi della chiamata ping
{
  alarm(0);
	if (cont == 0) return 0; //se il contatore delle chiamate ricorsive è arrivato fino a zero mi fermo e restituisco false
	char result[DIM_MAX_PKT];//result[4] = {'n','a','k','\0'}; //inizializzazione del risultato a "nak"
  
	printf ("\n%d => PING (rating=%f) - timeout=%d sec\n",getpid(), rating, cont);

  struct sockaddr_in appaddr;
	int iplen=sizeof(appaddr); 

	memset((void *)&appaddr, 0, sizeof(appaddr));
  appaddr.sin_family = AF_INET;
  appaddr.sin_port = htons(SP_UDP_PORT);
	appaddr.sin_addr=IP;

  int j;
  char* char_rat = malloc(sizeof(rating));
 	for( j = 0; j < sizeof(rating); j++)	
    *( char_rat + j ) = * ( ((char *)(&rating)) + j);

  char* sendbuff=create_buffer( "ping", sizeof(rating), char_rat );

	//invio pacchetto
	if (sendto(sockfd, sendbuff, 8 + sizeof(rating) , 0, (struct sockaddr *)&appaddr, iplen) < 0) 
  {
      perror("errore in sendto ping");
      return 0;
   }	

	//ricevo il risultato dell'operazione
	struct sigaction alarmaction = {.sa_handler = alarm_handler};
	int retcode = sigaction(SIGALRM, &alarmaction, NULL);
  if (retcode == -1) 
  {
  	perror("bad sigaction()\n");
		return 0;
  }
  
	alarm(cont);	//il timeout aumenta nelle chiamate ricorsive successive
	var_ack=1;
	int received = 0;

	while(var_ack) //provo a ricevere finche non va a buon fine e finche non vado in timeout
	{
		//la recvfrom è non bloccante
		if ((recvfrom (sockfd, result, DIM_MAX_PKT, MSG_DONTWAIT , (struct sockaddr *)&appaddr, (socklen_t *)&iplen )) >= 3)
		{
			var_ack=0;
			received=1;
		}
	}
	
	if (received == 1)
	{
		alarm(0);
		packetUDP* pck;
		pck=create_packet(result);
		printf("%d => cont=%d RESULT=> %s\n",getpid(),cont,pck->cmd);//stampo il risultato dell'operazione

		if (strcmp(pck->cmd,"nak") == 0)
		{	//se l'operazione è andata male riprovo per un numero fissato di volte (fino a che il contatore va a zero)
			return 0;
		}

		if (strcmp(pck->cmd,"pong") == 0)
		{	//se l'operazione è andata bene restituisco 1
			return 1;
		}	
	}

	else
	{
		printf("%d => cont=%d RESULT=> nessuna risposta\n",getpid(),cont);//stampo il risultato dell'operazione	
    alarm(0);		
    return 0;
	}
	alarm(0);
	return 0;	
}

/* il peer si sconnette dal SP */
/* RETURN VALUE
 * 1 success
 * 0 insuccess
*/
int leave_UDP (int sockfd, struct in_addr IP ,int cont)
{
  conn = 0;
  alarm(0);
	if (cont == 0)
      return 0; //se il contatore delle chiamate ricorsive è arrivato fino a zero mi fermo e restituisco false
	
	char buff[12]; //buffer per i dati da inviare nel segmento udp

  char result[DIM_MAX_PKT];

  printf ("\n%d => LEAVE (%d)\n",getpid(), cont);

  struct sockaddr_in appaddr; //riferimento all'IP
	int iplen=sizeof(appaddr); //dimensione della struttura sockaddr_in


	memset((void *)&appaddr, 0, sizeof(appaddr)); 
  appaddr.sin_family = AF_INET; 
  appaddr.sin_port = htons(SP_UDP_PORT); 
	appaddr.sin_addr=IP;
	
  char* sendbuff=create_buffer("leav", 0, "");

	//invio pacchetto
	if (sendto(sockfd, sendbuff, 8, 0, (struct sockaddr *)&appaddr, iplen) < 0)
  {
      perror("errore in sendto");
			return 0;
  }	

	//ricevo il risultato dell'operazione
	
	//rendo "sicura" la trasmissione
	//associo tramite la sigaction la mia funzione alarm_handler al segnale SIGALRM
	struct sigaction alarmaction = {.sa_handler = alarm_handler};
	int retcode = sigaction(SIGALRM, &alarmaction, NULL);

  if (retcode == -1) 
  {
 	  perror("bad sigaction()\n");
	  return 0;
  }

	alarm(6-cont);	//il timeout aumenta nelle chiamate ricorsive successive

  int received = 0;
	var_ack = 1;
	while(var_ack) //provo a ricevere finche non va a buon fine e finche non vado in timeout
	{
		//la recvfrom è non bloccante
		if ((recvfrom (sockfd, result, DIM_MAX_PKT, MSG_DONTWAIT, (struct sockaddr *)&appaddr, (socklen_t *)&iplen )) >= 3)
		{
      received = 1;
			var_ack = 0;
		}
	}

  if(received == 1)
	{
    packetUDP* pck;
	  pck=create_packet(result);
	  printf("%d => cont=%d RESULT=> %s\n",getpid(),cont,pck->cmd);//stampo il risultato dell'operazione
	
	  if (strcmp(pck->cmd,"nak") == 0)
	  {	//se l'operazione è andata male riprovo per un numero fissato di volte (fino a che il contatore va a zero)
		  return leave_UDP(sockfd, IP, cont-1);
	  }

	  if (strcmp(pck->cmd,"ack") == 0)
	  {	//se l'operazione è andata bene restituisco 1
		  alarm(0);
		  return 1;
	  }	
  } 
  else
  {
    printf("%d => cont=%d RESULT=> nessuna risposta\n",getpid(),cont);//stampo il risultato dell'operazione	
    alarm(0);		
    return 0;
  }

  alarm(0);
	return 0;	
}

/* il peer effettua una richiesta di un file al SP */
/* RETURN VALUE
 * peerlist di chi ha il file
 * -1 in caso di errore
*/
int whohas_UDP (int sockfd, struct in_addr IP, char* word ,int cont)
{
  alarm(0);
	if (cont == 0)
    return 0; //se il contatore delle chiamate ricorsive è arrivato fino a zero mi fermo e restituisco false

  long ind_ip;
	char buff[ 12 + strlen(word) ]; //buffer per i dati da inviare nel segmento udp

  char result[DIM_MAX_PKT];
	printf ("\n%d => WHOHAS:%s - cont=%d\n",getpid(), word, cont);

  struct sockaddr_in appaddr; //riferimento all'IP
	int iplen=sizeof(appaddr); //dimensione della struttura sockaddr_in

	memset((void *)&appaddr, 0, sizeof(appaddr)); 
  appaddr.sin_family = AF_INET; 
  appaddr.sin_port = htons(SP_UDP_PORT); 
	appaddr.sin_addr=IP;

  int j;
	char* word_buf= (char *)malloc(strlen(word));

	for (j=0; j < strlen(word);	j++)
	{
		word_buf[j]= *(word + j);	
	}

	char* sendbuff=create_buffer_whhs("whhs", strlen(word), 0, word_buf);
	
	//invio pacchetto
	if (sendto(sockfd, sendbuff, 13 + strlen(word), 0, (struct sockaddr *)&appaddr, iplen) < 0) 
  {
      perror("errore in sendto");
			return 0;
  }	

	//ricevo il risultato dell'operazione
	
	//rendo "sicura" la trasmissione
	//associo tramite la sigaction la mia funzione alarm_handler al segnale SIGALRM
	struct sigaction alarmaction = {.sa_handler = alarm_handler};
	int retcode = sigaction(SIGALRM, &alarmaction, NULL);

  if (retcode == -1) 
  {
  	perror("bad sigaction()\n");
		return 0;
  }

  int received=0;
	alarm(6-cont);	//il timeout aumenta nelle chiamate ricorsive successive

	var_ack= 1;  //imposto a 1 in modo tale che entra nel ciclo, saranno gli eventi di SIGALRM o di recvfrom a sbloccare il ciclo	
	while (var_ack) //provo a ricevere finche non va a buon fine e finche non vado in timeout
	{
		//la recvfrom è non bloccante
		if ((recvfrom (sockfd, result, DIM_MAX_PKT, MSG_DONTWAIT , (struct sockaddr *)&appaddr, (socklen_t *)&iplen )) >= 13)
		{
			var_ack = 0;
      received = 1;
		}
	}

  if( received == 1)
  {
	  packetWHHS* pck;
	  pck= create_packet_whhs(result);
	
	  //nel caso è stato trovato un file verranno ricevuti anche un insieme di IP dei peer contenenti il file cercato
	  int numero_ip= 0;
    numero_ip= pck->size >> 2;
	  printf("%d => numero di ip %d\n", getpid(), numero_ip);
	  //salvo gli ip trovati
	  struct sockaddr_in ip[numero_ip];
	  int k = 0;
    long *app;
    //printf("\nsize:%d\n",pck->size);
	  for (j= 0; j < numero_ip; j++)
	  {	
      app= (long *)(pck->dati+j*4);
      ip[j].sin_addr.s_addr= *app;
    }
	
	  //stampo gli ip trovati	
	  printf("%d => LISTA IP RESTITUITI\n",getpid());

	  for (j= 0; j < numero_ip; j++)
	  {
      if(ip[j].sin_addr.s_addr != my_ip.s_addr)
      {      
        Ins_testa(ip[j], &list_result);
		    printf("%d => %d = %s\n", getpid(), j, inet_ntoa(ip[j].sin_addr));	
      }
	  }
	

	  printf("%d => cont=%d RESULT OF THE OPERATION=> %s\n",getpid(), cont, pck->cmd);//stampo il risultato dell'operazione
   	if (strcmp(pck->cmd, "nak") == 0)
	  {	//se l'operazione è andata male riprovo per un numero fissato di volte (fino a che il contatore va a zero)
		  return whohas_UDP(sockfd, IP, word, cont-1);
	  }

	  if (strcmp(pck->cmd,"ack") == 0)
	  {	//se l'operazione è andata bene restituisco 1
		  alarm(0);
		  return 1;
	  }	  
  }
  else
	{
		printf("%d => cont=%d RESULT=> nessuna risposta\n",getpid(),cont);//stampo il risultato dell'operazione	
		return whohas_UDP(sockfd, IP, word, cont-1);
	} 
  alarm(0);  
	return 0;	
}

/* messaggio di stop per le ricerche */
int stop_UDP (int sockfd, struct in_addr IP, int cont)
{
  alarm(0);
	if (cont == 0)
    return 0; //se il contatore delle chiamate ricorsive è arrivato fino a zero mi fermo e restituisco false
	
	char buff[12]; //buffer per i dati da inviare nel segmento udp
  fflush (stdout);
	
  char result[DIM_MAX_PKT];
  printf ("\n%d => STOP (%d)\n",getpid(), cont);

  struct sockaddr_in appaddr; //riferimento all'IP
	int iplen=sizeof(appaddr); //dimensione della struttura sockaddr_in

	memset((void *)&appaddr, 0, sizeof(appaddr)); 
  appaddr.sin_family = AF_INET; 
  appaddr.sin_port = htons(SP_UDP_PORT); 
	appaddr.sin_addr=IP;

  char* sendbuff=create_buffer("stop", 0, "");
	//invio pacchetto

	if (sendto(sockfd, sendbuff, 8, 0, (struct sockaddr *)&appaddr, iplen) < 0)
  {
      perror("errore in sendto");
			return 0;
  }	
	
	//rendo "sicura" la trasmissione
	//associo tramite la sigaction la mia funzione alarm_handler al segnale SIGALRM
	struct sigaction alarmaction = {.sa_handler = alarm_handler};
	int retcode = sigaction(SIGALRM, &alarmaction, NULL);

  if (retcode == -1) 
  {
	  //controllo della giusta esecuzione della sigaction
 	  perror("bad sigaction()\n");
	  return 0;
  }

	alarm(6-cont);	//il timeout aumenta nelle chiamate ricorsive successive

	var_ack = 1; //imposto a 1 in modo tale che entra nel ciclo, saranno gli eventi di SIGALRM o di recvfrom a sbloccare il ciclo	
	while(var_ack) //provo a ricevere finche non va a buon fine e finche non vado in timeout
	{
		//la recvfrom è non bloccante
		if ((recvfrom (sockfd, result, DIM_MAX_PKT , MSG_DONTWAIT , (struct sockaddr *)&appaddr, (socklen_t *)&iplen )) >= 3)
		{
			var_ack = 0;
		}
	}

	packetUDP* pck;
	pck = create_packet(result);
	printf("%d => cont=%d RESULT=> %s\n", getpid(), cont, pck->cmd);//stampo il risultato dell'operazione
	
	if (strcmp(pck->cmd, "nak") == 0)
	{	//se l'operazione è andata male riprovo per un numero fissato di volte (fino a che il contatore va a zero)
		return stop_UDP(sockfd, IP, cont-1);
	}

	if (strcmp(pck->cmd,"ack") == 0)
	{	//se l'operazione è andata bene restituisco 1
		alarm(0);
		return 1;
	}	
  alarm(0);
	return 0;	
}

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
/* JOIN AL SERVER DI BOOTSTRAP */
int join()
{   
  int sockd;
  cancella_lista(&list);		

  if ((sockd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("errore in socket");
    exit(1);
  }

  memset((void *)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERV_TCP_PORT); 

  if (inet_pton(AF_INET, ip_server, &servaddr.sin_addr) <= 0) 
  {
    fprintf(stderr, "errore in inet_pton per %s", ip_server);
    exit(1);
  }
      
  if ( connect (sockd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) 
  {
    perror("errore in connect");
    exit(1);
  }    
  
  char sendline[10], recvline[1028];
  int p=0,i=1,j=0,*len,n,cont;
  long *ip;
  struct sockaddr_in IP;
  socklen_t lun= sizeof(IP);
  strcpy(sendline,"join\n");

  getsockname (sockd,(struct sockaddr *)&IP, &lun);
  my_ip= IP.sin_addr;

  if ( writen(sockd, sendline, strlen(sendline)) < 0) 
  {
	  fprintf(stderr, "errore in write");
	  exit(1);
  }

  if ( readline(sockd, recvline, 6) <0 )
  { 
    perror("errore readline:");
    exit(1);
  }

  if (strcmp(recvline, "empty") == 0)
  {
    printf("%d => LISTA SUPERPEER VUOTA\n", getpid());
    //SONO SUPERPEER iniz_superpeer();
    p=1;
  }

  if (strcmp(recvline, "ack\n") == 0)
  {
    memset((void *)&IP, 0, sizeof(IP));
    IP.sin_family = AF_INET;   
    memset((void *)recvline, 0, sizeof(recvline));
    int k=0;

    if (read(sockd, recvline, 4) == 4)
    {
      len= (int *)recvline;
    }
    char *str;
    cont=0;
    str=(char *)malloc(*len);

    /* ricevo gli indirizzi ip dei superpeer */
    while ((n=read(sockd, str, *len)) > 0)
    {  
      for (k = 0; k < n; k+=4)
      {
        ip=(long *)(str+k);
        IP.sin_addr.s_addr= *ip;
        if ((my_ip.s_addr) != IP.sin_addr.s_addr)
          Ins_testa(IP, &list);
        else // il mio indirizzo IP è nella lista dei superpeer e restituisco 1 per dire ke sono superpeer
          p=1;
                       
        cont++;
      }
    }

    free(str);
    Stampa(&list);
  }

  if (close(sockd) == -1) 
  {
    perror("errore in close");
    exit(1);
  }      
  return p;
}


/************************************************************/
//FUNZIONI LISTA


void Ins_testa(struct sockaddr_in ind, struct list_head *lista)
{
  struct list_head *ptr_list, *ptr_el;
  ptr_list= lista->next;
  ptr_el = (struct list_head *)malloc(sizeof(struct list_head));
  ptr_el->IP = ind;
  ptr_el->prev = lista;
  ptr_el->next = ptr_list;
  ptr_list->prev = ptr_el;
  lista->next = ptr_el;
}

void Stampa(struct list_head *lista)
{
  struct list_head *ptr_list;
  ptr_list = lista;
  printf("%d => STAMPA LISTA\n", getpid());
  int i=0;
  while (ptr_list->next != lista)
  {
    ptr_list = ptr_list->next;
    printf("ELEM[%d] IP= %s\n", i, inet_ntoa(ptr_list->IP.sin_addr) );
    i++;
  }
}

void Cancella(struct sockaddr_in x, struct list_head *lista)
{      
  struct list_head *ptr_app, *ptr_list, *ptr_el;
  ptr_list = lista->next;
  ptr_el = ptr_list;
       
  while ( (ptr_list != lista) && ( ptr_list->IP.sin_addr.s_addr != x.sin_addr.s_addr ) )
    ptr_list=ptr_list->next;                 
       
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
}

void cancella_lista(struct list_head *lista)
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

int Listlen(struct list_head *lista)
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

struct sockaddr_in *GET_LIST(int pos, struct list_head *lista)
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

/* 
FUNZIONI PER IL DOWNLOAD E PER L'UPLOAD 
*/

int download (struct in_addr ip, char* file) //restituisce 1 se il download va a buon fine, 0 altrimenti
{
  char link[512]="Scaricati/";
  int j;
  
	int sockfd;
	char recvline[MAXLINE];
	char recvlinechunk[CHUNK];
	struct sockaddr_in servaddr;

	fflush (stdout);
	
  /* creo la socket */
  while (1)
  {
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) 
    {
      if(errno == EINTR)
        continue;
      else
      {	
        perror ("errore in socket");
        return 0;
      }
    }
    else 
      break;
  }
  memset ((void *) &servaddr, 0, sizeof (servaddr));	
  servaddr.sin_family = AF_INET;	
 	servaddr.sin_addr = ip;
	servaddr.sin_port = htons(UPLOAD_PORT);


  // stabilisce la connessione con il peer 
  while (1)
  {
    if (connect (sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) < 0) 
    { 
      if(errno == EINTR)
        continue;
      else
      {
        perror ("errore in connect");
        return 0;
      }
    }
    else 
      break;
  }

  printf ("\nINIZIO DOWNLOAD\n");
  
  //invio nome file che voglio scaricare
  
  char nome_file[255];
  int i;  
  for(i=0;i<255;i++)nome_file[i]='\0';
  for(i=0;i<strlen(file);i++)nome_file[i]=file[i];

	printf("nome file : %s\n",nome_file);
  
  while (1)
  {
    if (write (sockfd, nome_file, 255) != 255)
    {
      if(errno == EINTR)
        continue;
      else
      {
        perror ("errore in write");
        return 0;
      }
    }
    else 
      break;
  }
  //fine invio nome file

// ricezione dimensione file 
	char app_size[4];
	int* size;		
  while (1)
  {
    if (read(sockfd,app_size,4)!=4)
    {
      if(errno == EINTR)
        continue;
      else
      {
		    printf ("errore nella read della size\n");
        return 0;
      }
    }
    else 
      break;
  }
 
	size=(int *)&app_size;
	printf("dimensione file: %d \n",*size);
	
  if(*size>=0)
  {
    // ricezione numero chunk file 
	  char app_sizechunk[4];
	  int* sizechunk;		

    while (1)
    {
      if (read(sockfd,app_sizechunk,4)!=4)
      { 
        if(errno == EINTR)
          continue;
        else
        {
          printf ("errore nella read della size\n");
          return 0;
        }
      }
      else 
        break;
    }

	  sizechunk=(int *)&app_sizechunk;
	  printf("numero chunk file: %d \n",*sizechunk);		

	  //invio ack
	  char ack[4];
    if(*sizechunk>0)
      sprintf(ack,"ack\0");
    else  
      sprintf(ack,"nak\0");
      	
    while (1)
    {
      if ((write (sockfd, ack, 4)) !=  4) 
      { 
        if(errno == EINTR)
          continue;
        else
        {
		      perror ("errore nell'ack");
          return 0;
        }
      }
      else 
      {
        printf("invio ack riuscito\n");
        break;
      }
    }

    printf("-----------------\n");
		
	  // lettura da socket e salvataggio file
    if(strcmp(ack,"ack")==0)
    {
      char *result=malloc(5);
	    char infochunk[4];
	    int fd2;
	    int n;
	    int t=0;
	    int temp_size=0;
      int ctrl=0;
	    strcat(link,nome_file);
	    printf ("posizione in cui verrà salvato il file : %s\n", link);	

      while (1)
      {
        if (((fd2 = open (link, O_WRONLY, 0666)) < 0) && (errno==ENOENT))
        {
          if(errno == EINTR) 
            continue;
          else
          {
            while(1)
            {
             	if (((fd2 = open (link, O_WRONLY | O_CREAT, 0666)) < 0) && (errno==ENOENT)) 
              {
                if(errno == EINTR)
                  continue;
                else
                {
                  perror ("errore nel file");
                  return 0; 
                }
              }
            	else
              { 	
           			printf("store\n");  // informo l'uploader che il file ricevuto sarà salvato come nuovo 
                		                // (l'info sarà trasmessa a trasferimento avvenuto)				 
                ctrl=1; //per uscire dal doppio ciclo infinito
                break;
              }//fine blocco della creazione del file
            }
            if(ctrl==1) //esco anche dal secondo while 
              break;
          }
        }
        else
        { 	
	        printf("update\n");  //se il file gia esiste,richiedi file da chunk x oppure segnala di avere il file completo
          break;
        }//fine else dell 'invio del file gia fatto(completo o non)
      }

		
      int chunk=lseek(fd2,0,SEEK_END);//dimensione del file in byte
			int chunk_app;
	    chunk_app=chunk/CHUNK;	//numero chunk gia ricevuti
	    printf("numero chunk già ricevuti di questo file: %d\n", chunk_app);
		     		
			
	    if(chunk_app == *sizechunk) //ho già il file completo, informo l'uploader e chiudo la connessione
	    { 
        printf("file già ricevuto\n");
	      sprintf(infochunk,"comp\n");

        while (1)
        {
          if ((write(sockfd,infochunk,4))!=4)
          {
            if(errno == EINTR)
              continue;
            else
            {
              perror ("errore nell'infochunk");
              return 0;
            }
          }
          else 
            break;
        }
        	
        while (1)
        {
          if (close(fd2) < 0)
          {
            if(errno == EINTR)
              continue;
            else
            {
              perror ("errore in close del file");
              return 0;
            }
          }
          else 
            break;
        }
	    }
      
      if (chunk_app < *sizechunk) // il file che ho è incompleto, richiedo il trasferimento a partire dall'ultimo chunk ricevuto
      {
        printf("file incompleto\n");
        sprintf(infochunk,"part\n");
        
        while (1)
        {
          if (write(sockfd,infochunk,4)!=4)
          {
            if(errno == EINTR)
              continue;
            else    
            {
              perror ("errore nell'infochunk");
              return 0;
            }
          }
          else 
            break;
        }
		    //richiedi invio da chunk x

		    char *sent_sizechunk;
		    sent_sizechunk=(char *)&chunk_app; //numero dell'ultimo chunk ricevuto
		    int temp_size2=(chunk_app*CHUNK);  //numero dei byte del file gia ricevuti
		
        while (1)
        {
          if (write(sockfd,sent_sizechunk,4) != 4)
          {
            if(errno == EINTR)
              continue;
            else
            {
        		  perror("errore nella write del sent_sizechunk");
              return 0;
            }
          }
          else 
            break;
        }
		    off_t z=0;
		    z=(off_t)temp_size2;
		    lseek(fd2,z,SEEK_SET);	
		    printf("**********\n");	
        printf ("DOWNLOAD IN CORSO\n");
		    printf("**********\n");	
        while (1)
        {
          if (((n = read (sockfd, recvlinechunk, CHUNK)) < 0) && (temp_size2 < (*size)))
          {
            if(errno == EINTR)
              continue;
            else
            {
        		  perror("errore nella read del file da socket");
              return 0;
            }
          }
          else 
          {
            temp_size2+=n;
	          //printf("ricevuto chunk: %d\n %d di %d\n",chunk_app,temp_size2,(*size));
			      
            while (1)
            {
              if ((write (fd2, &recvlinechunk, n)) != n) 
              {
                if(errno == EINTR)
                  continue;
                else
                {
               	  perror ("errore scrittura");
               		return 0;
               	} 
              }    
              else
                break;
            }

     			  chunk_app++;
			      if(temp_size2==(*size))
              break;
            if(n==0)
              break;
          }
        }

        printf ("fine lettura da socket\n");
	
        if (n >= 0 && (temp_size2==(*size)))
        {  
          printf("transazione completata, file ricevuto senza errori\n");
          result="ack";
        }
        else
          result="nak"; //qualcosa è andato male
        
        //invio del risultato e chiusura socket
        while (1)
        {
          int m;
          m=(write (sockfd, result, 3));
          if (m != 3)
          {
            if(errno == EINTR)
              continue;
            else
            {
           	  perror ("errore scrittura");
           		return 0;
           	} 
          }    
          else
          {
            printf("result inviato: %s\n",result);
            break;
          }
        }
      }//fine dell esle if che richiede il file a partire dall'ultimo chunk ricevuto    


    }	
  }
  else
    printf("file non presente sul SP\n");

  while (1)
  {
    if (close (sockfd) < 0 )
    {
      if(errno == EINTR)
        continue;	    
      else
      {            
     	  perror ("errore in close");
    		return 0;
      }
    }
    else 
      break;
  }
  printf ("\nFINE DOWNLOAD\n");
  return 1;
}

int upload()
{

  //printf ("%d => CODICE UPLOAD\n", getpid());
  int listensd, connsd;
  struct sockaddr_in servaddr;
  pid_t pid;
  char buff[CHUNK];
  char buffname[255];
  char* result=malloc(5);
  char nome_file[255];
  int k=1;
  int t=NUM_UPLOAD; //contatore per gestire numero massimo di upload
  int status;


  // fase di ascolto e connessione 
  while (1)
  {
    if (((listensd = socket (AF_INET, SOCK_STREAM, 0)) < 0) && (errno == EINTR))
      continue;
      
    if (((listensd = socket (AF_INET, SOCK_STREAM, 0)) < 0) && (errno != EINTR))
    {				// crea il socket 
      perror ("errore in socket");
      return 0;
    }
    
    else 
      break;
  }
  
  setsockopt(listensd,SOL_SOCKET,SO_REUSEADDR,&k,sizeof(int));
  memset ((void *) &servaddr, 0, sizeof (servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
  servaddr.sin_port = htons (UPLOAD_PORT);	 

  
  while (1)
  {
    if (((bind (listensd, (struct sockaddr *) &servaddr, sizeof (servaddr))) < 0))
    {
      if(errno == EINTR)
      {
        continue;
      }
      else
      {
        perror ("errore in bind");
        return 0;
      }
    }          
    else 
    {
      break;
    }  
  }


  while (1)
  {
    if (listen (listensd, BACKLOG) < 0)
    {
      if(errno == EINTR)
      {
        continue;
      }
      else
      {
        perror ("errore in listen");
        return 0;
      }
    }          
    else 
    {
      break;
    }  
  }

  for (;;)
  {	
  	//printf("%d => in attesa di connessioni entranti\n", getpid());

    while (1)
    {
      if ((connsd = accept (listensd, NULL , NULL)) < 0) 
      {
        if(errno == EINTR)
          continue;
        else
        {			
          perror ("errore in accept");
          return 0;
        }
      }
      else 
        break;
    }

    if ( t < 0 )
    { 
      waitpid( WAIT_ANY, &status, 0);		
    	t++;
    }
    
    t--;	

    if(( pid=fork()) == 0)
    {
      while (1)
      {
        if (close(listensd)<0) 
        {
          if(errno == EINTR)
            continue;
          else
          {			
            perror ("errore in close");
            return 0;
          }
        }        
        else 
          break;
      }

    	
		  // accetta una connessione con un client 
      //leggo il nome del file inviato dal client
      int n_name;
      int fd1;
      char *dir_down=PATH_LOCALE;//cartella dove l'uploader tiene i file disponibili da condividere
      char *link=malloc(516); 
      char c;
      char* parziale;
      int ind=0;
    	
      while (1)
      {
        if ((n_name = read (connsd, buffname, 255)) != 255) 
        {
          if(errno == EINTR)
            continue;
          else        
          {			
            perror("errore in read nome file");
            return 0;
          }
        }        
        else 
          break;
      }
      //printf("buffname %s \n",buffname);
      int pres=search_file(dir_down,buffname,1);

     	if ((strcmp(buffname,"") == 0) || (pres != 1))
   		{ 	
        int size=0;
        char* sent_size;
        sent_size=(char *)&size;
         
        while (1)
        {
          if (write (connsd, sent_size, 4) != 4) 
          {
            if (errno == EINTR)
              continue;
            else
            {			
           		printf("errore nella write size\n");
              return 0;
            }
          }
          else 
            break;
        }

        //printf("file non trovato\n");
				while (1)
        {
          if (close(connsd)<0) 
          {
            if(errno == EINTR)
              continue;
            else
            {			
              perror("errore in close");
              return 0;
            }
          }         
          else 
            break;
        }

 				t++;
 				break;
   		}

     	else
      {
        //se il file c'è cerco il path
        parziale=search_path_file(dir_down,buffname,1);
       	//printf( "percorso file:%s\n", parziale );
      	
        int j;
        //concateno il link della cartella di riferimento a quello del file specifico
        for(j=0;j<strlen(dir_down);j++) *(link+j)=*(dir_down+j);  
        for(j=strlen(dir_down);j<strlen(parziale)+strlen(dir_down);j++)
            *(link+j)=*(parziale+j-strlen(dir_down));  
        *(link+j)='\0';
       	//printf( "percorso file:%s\n", link );
   				
        while (1)
        {
          if (((fd1 = open(link, O_RDONLY)) < 0) && (errno==ENOENT)) 
          {
            if (errno == EINTR)
              continue;
            else
            {			
           		printf("errore nella open, il file non esiste\n");
              return 0;
            }
          }
          else 
            break;
        }        
 
        char * sent_size;
	      int size;
	      int m;
	       
        while (1)
        {
          size=lseek(fd1,0,SEEK_END);
          if ((size < 0) && (errno == EINTR))
            continue;
              
          if ((size < 0) && (errno != EINTR))
          {			
         		printf("errore nella lseek\n");
            return 0;
          }
          else 
            break;
        }  
        //printf("dimesione file :%d \n",size);

        while (1)
        {
          if (lseek(fd1,0,SEEK_SET)< 0) 
          {
            if(errno == EINTR)
              continue;
            else
            {			
           		printf("errore nella lseek\n");
              return 0;
            }
          }
          else 
            break;
        }
        sent_size=(char *)&size;
         
        while (1)
        {
          if (write (connsd, sent_size, 4) != 4) 
          {
            if (errno == EINTR)
              continue;
            else
            {			
           		printf("errore nella write size\n");
              return 0;
            }
          }
          else 
            break;
        }
      	//segnalo numero chunk da inviare

        if( (size<CHUNK) && (size >0) ) //caso in cui il file richiesto è piu piccolo di un CHUNK
          size=1;
      	else
          size = size / CHUNK;
  
        char * sent_sizechunk;
       	sent_sizechunk=(char *)&size;
       	//printf("numero chunk da inviare: %d\n", size);
 
        while (1)
        {
          if (write (connsd,sent_sizechunk, 4) != 4) 
          {
            if (errno == EINTR)
              continue;
            else 
            {			
           		printf("errore nella write size\n");
              return 0;
            }
          }
          else 
            break;
        }

      	//ricezione ack
        
        while (1)
        {
          if ((read (connsd, buffname, 4)) != 4) 
          {
            if(errno == EINTR)
              continue;
            else   
            {			
           	  perror ("errore in read");
              return 0;
            }
          }
          else
          {
 	          buffname[3]='\0'; 
            break;
          }
        }  
     	  //printf("%s\n",buffname);	  
       	//fine ricezione ack
       	//printf("-------------------\n");
        if(strcmp(buffname,"ack")==0)
        {
          //se ricevo ack invio il file altrimenti termino senza inviare niente
          char sendchunk[CHUNK];
         	char c;
         	int i=0;
         	off_t p1=0;
         	//invio file
         	char infochunk[5];
          
          while (1)
          {
            if ((read(connsd,infochunk,4))!=4) 
            {
              if(errno == EINTR)
                continue;
              else
              {			
                perror("errore nella read dell infochunk");
                return 0;
              }
            }
            else
            {
              break;
            }
          }           	
          infochunk[4]='\0';
          //printf("infochunk: %s\n", infochunk);
          
          off_t sizebyte=0;
          if ((strcmp(infochunk,"comp")) == 0) 
          {
            //file completo non faccio niente
            printf("nessun invio, file già completo\n");
          }
          if ((strcmp(infochunk,"part")) == 0) 
          { 
            //l'uploader riceve il numero di chunk gia ricevuti, li calcola in byte
					  //e si posiziona nel file a partire dall'ultimo byte ricevuto dal downloader
		        char app_sizechunk[4];
                   
            while (1)
            {
              if ((read(connsd,app_sizechunk,4))!=4) 
              {
                if(errno == EINTR)
                  continue;
                else 
                {			
                  perror("errore nella read dell sizechunk");
                  return 0;
                }
              }                
              else
              {
                break;
              }
            }  
		        int* sizechunk=(int *)app_sizechunk;

		        sizebyte= (off_t)( (*sizechunk) * CHUNK);
            //printf("sizebyte=%d\n",(int)sizebyte);
            while (1)
            {
              if (lseek(fd1,sizebyte,SEEK_SET)<0) 
              {
                if(errno == EINTR)
                  continue;
                else  
                {			
                  perror("errore nella lseek");
                  return 0;
                }
              }
              else
              {
                break;
              }
            }  
            int r;
            while(1) 
            {
							r=read (fd1, &c, 1);
              if(r==1)
              {
           	  	sendchunk[p1 % CHUNK] = c;
             		p1++;
		
							
             		if(p1%CHUNK==0) 
                {
                  while (1)
                  {
                    if (write (connsd, sendchunk, CHUNK) != CHUNK) 
                    {
                      if(errno == EINTR)
                        continue;
                      else   
                      {			
                        perror("errore in write");
                        return 0;
                      }
                    }
                    else
                    {
                      break;
                    }
                  }  
              // 		printf("chunk numero %d inviato \n", i);
	                i++;
         	      } 
              }
					
								if((r < 0) && (errno == EINTR))
								continue;							

								if((r < 0) && (errno != EINTR))
								perror("errore in lettura file");
					
								if( r == 0) break;
							
            }//fine while

            if (p1 % CHUNK != 0)
   		      {
             while (1)
              {
                if (write (connsd, sendchunk, p1 % CHUNK) != p1 % CHUNK)
                {
                  if(errno == EINTR)
                    continue;
                  else   
                  {			
                    perror("errore in write");
                    return 0;
                  }
                }
                else
                {
                  break;
                }
              } 
              //printf("chunk numero %d inviato \n", i);
	          }
            
            int m;
            while (1)
            {
              m=read(connsd,result,3);
              if (m!=3)
              {
                if(errno == EINTR)
                  continue;
                else       
                {			
                  fprintf (stderr,"errore nel result:%s\nm=%d\n",result,m);
                  return 0;
                }
              }
              else
              {
                *(result+3)='\0';
             		//printf("\nresult ricevuto: %s\n",result);             	
                break;
              }
            }
          }//fine if dell'infochunk uguale a "richiesta chunk x"	
                      
          while (1)
          {
            if (close(fd1) < 0)
            {
              if(errno == EINTR)
                continue;
              else
              {			
                perror("errore in close");
                return 0;
              }
            }
            else
            {
              break;
            }
          }
	      } 	
                    
        while (1)
        {
          if (close(connsd) < 0)
          { 
            if(errno == EINTR)
              continue;
            else   
            {			
              perror("errore in close");
              return 0;
            }
          }
          else
          {
            break;
          }
        }
          
	      t++;
	      return 1;
      }		
    }//fine if processo figlio

    // chiudo la connessione 	  
    
    while (1)
    {
      if (close(connsd) < 0)
      {
        if(errno == EINTR)
          continue;
       else                
        {			
          perror("errore in close");
          return 0;
        }
      }
      else
      {
        break;
      }
    }
    //printf("\nconnessione servita\n");
  } //fine for
}


/*
FUNZIONI PER L'INTERAZIONI DINAMICHE CON I SUPERPEER
CONNESSIONI / CAMBI DI SP / etc
*/
int connect_to_SP ()
{
/* codice della connessione al SP da parametrizzare */
  
  struct in_addr ip;
	struct list_head *ptr_list;
  int sp = join(); //join del peer al bootstrap
  printf("MY_IP: %s***** SP: %d\n", inet_ntoa(my_ip), sp);
	//se il peer che si sta connettendo è stato eletto superpeer faccio partire il codice relativo
  if (sp == 1)
  {
	  if ((pid_SP = fork() ) == 0) 
    {
		  Superpeer();
    }
  }
  sleep(1); //per dare modo al SP di avviarsi
	int dim=Listlen(&list); //dimensione lista
	int join=0; //a 0 finchè non riesce un join
  struct in_addr array_ip[dim]; //creo array di ip di dimensione della lista
  float ping[dim]; //conterrà il valore dei ping dei SP pingati
	int j=0; //contatore
  int k=0;

  if (sp != 1)
  {
    printf("%d ==> dimensione lista SP ricevuta = %d\n", getpid(), dim );
    ptr_list = &list;
	
	  //scorro la lista e salvo i vari ip nell'array	
    while ( (ptr_list->next != &list) && (join == 0) )
    {
    	ptr_list = ptr_list->next;
		  ip=ptr_list->IP.sin_addr;
		  array_ip[j]=ip;
      ping[j]=5.0;
		  j++;
	  }
	  struct timeb start,end;
	  float elapsed;
    int res_ping=0;
	  //faccio il ping di tutti i superpeer
	  for (j=0 ; j<dim ; j++)
	  {
		  ftime(&start);
      res_ping=ping_UDP(sockUDPsd, array_ip[j],5);
		  ftime(&end);
		  elapsed = (end.millitm-start.millitm)*1.0e-3 + (end.time-start.time);	
      printf("%d ==> ping a %s (%f) : %d\n", getpid(), inet_ntoa(array_ip[j]), elapsed, res_ping);
		  ping[j] = elapsed;
	  }

	  //ordino l'array in ordine crescente
	  for( j=0 ; j<dim-1 ; j++ )
	  {
		  for ( k=j+1 ; k<dim ; k++ )
		  {
         if( ping[j] > ping[k] )
				 {
		       struct in_addr app_ip;
		       float app_ping;
		       //scambio
		       app_ping=ping[j];
		       ping[j]=ping[k];
		       ping[k]=app_ping;
					 app_ip=array_ip[j];
					 array_ip[j]=array_ip[k];
					 array_ip[k]=app_ip;
				}		
			}
		}
	
		// una volta ordinato provo a fare la join in ordine finche non va a buon fine
		for( j=0 ; j < dim && join == 0 ; j++ )
		{
			ip=array_ip[j];
			if(ping[j]!=5) join=join_UDP(sockUDPsd, array_ip[j], rating , 5 ); //se ping[j]==0 vuol dire che era scaduto il timeout quindi non provo neanche la 
                                                                         //connessione
			//printf("%d ==> join a %s : %d\n", getpid(), inet_ntoa(array_ip[j]), join);
		}
    if(join==1)
  	{
    	IPSP=ip;
      conn=1;
    }
    else
      conn=0;
  }

  else
  {
    //printf("%d ==> JOIN MY IP\n\n", getpid() );
    join=join_UDP(sockUDPsd, my_ip, rating, 5 );
		if(join==1)
    {
      conn=1;
      IPSP=my_ip;
    }
    else
      conn=0;
  }
/* fine codice della connessione al SP da parametrizzare */

  return join;
}

/* funzione per la connessione ad uno specifico SP di cui è noto l'indirizzo ip */
int change_to_SP (struct in_addr ip)
{
  int join=0;

  printf("%d ==> leave : %d\n", getpid(), leave_UDP(sockUDPsd, IPSP, 2) ); //per precauzione mando una leave al SP

  join=join_UDP(sockUDPsd, ip, rating , 5 ); //se ping[j]==0 vuol dire che era scaduto il timeout quindi non provo neanche la connessione
	
  if(join==1)
  {
    IPSP=ip;
    conn=1;
  }
  else
    conn=0;
  return join;
}

/* cambia il SP del peer che invoca la funzione */
int change_SP ()
{
  printf("%d ==> leave : %d\n", getpid(), leave_UDP(sockUDPsd, IPSP, 2) ); //per precauzione mando una leave al SP

  int join=0;
  join=connect_to_SP();
  return join;
}

/* chiude tutti i processi */
int exitP ()
{
  alarm(0);
  sleep(1); //per essere sicuri di non chiudere il SP prima che abbia finito la leave
  if (close(sockUDPsd) < 0) 
    perror("errore in close");
            
  if(pid_up>=0)
    kill(pid_up, SIGKILL); //prima di uscire uccido il processo di upload
  if(pid_SP>=0)
    kill(pid_SP, SIGKILL); //prima di uscire uccido il processo di upload 
  
  printf ("fine codice del peer\n");
  return;
}

/* restituisce il risultato di eleggi_UDP, quindi se restituisce 1 vuol dire che il peer si è connesso a un nuovo SP*/
int elezione_nuovo_SP()
{ 
  struct in_addr ip;
	struct list_head *ptr_list;

	int dim=Listlen(&list); //dimensione lista
	int join=0; //a 0 finchè non riesce un join
  struct in_addr array_ip[dim]; //creo array di ip di dimensione della lista
  float ping[dim]; //conterrà il valore dei ping dei SP pingati
	int j=0; //contatore
  int k=0;

	
  printf("%d ==> dimensione lista SP ricevuta = %d\n", getpid(), dim );
  ptr_list = &list;
	
	//scorro la lista e salvo i vari ip nell'array	
  while ( (ptr_list->next != &list) && (join == 0) )
  {
  	ptr_list = ptr_list->next;
		ip=ptr_list->IP.sin_addr;
		array_ip[j]=ip;
		j++;
	}

  struct timeb start,end;
  float elapsed;
  int res_ping=0;
  //faccio il ping di tutti i superpeer
  for (j=0 ; j<dim ; j++)
  {
	  ftime(&start);
    res_ping=ping_UDP(sockUDPsd, array_ip[j],5);
		ftime(&end);
		elapsed = (end.millitm-start.millitm)*1.0e-3 + (end.time-start.time);	
    printf("%d ==> ping a %s (%f) : %d\n", getpid(), inet_ntoa(array_ip[j]), elapsed, res_ping);
		ping[j] = elapsed;
	}

	//ordino l'array in ordine crescente
	for( j=0 ; j<dim-1 ; j++ )
	{
	  for ( k=j+1 ; k<dim ; k++ )
		{
      if( ping[j] > ping[k] )
			{
		    struct in_addr app_ip;
		    float app_ping;
		    //scambio
		    app_ping=ping[j];
		    ping[j]=ping[k];
		    ping[k]=app_ping;
				app_ip=array_ip[j];
				array_ip[j]=array_ip[k];
				array_ip[k]=app_ip;
			}		
		}
	}
  int loop=1;
	for( j=0 ; (j<dim) && loop ; j++)
  {
    conn = 0;
    if(eleggi_UDP (sockUDPsd, array_ip[j]) == 1) 
    {
      loop = 0;
    }
  }
  if(loop == 0)
    return 1;
  else
    return 0;
}

/* restituisce 1 se è stato creato un superpeer e il peer è riuscito a connettersi ad esso */
int eleggi_UDP (int sockfd, struct in_addr IP)
{
  alarm(0);
  char result[DIM_MAX_PKT];  
	printf ("\n%d => ELEGGI SP\n",getpid());
  
  struct sockaddr_in appaddr; //riferimento all'IP
	int iplen=sizeof(appaddr); //dimensione della struttura sockaddr_in

	memset((void *)&appaddr, 0, sizeof(appaddr)); //azzera servaddr
  appaddr.sin_family = AF_INET; //assegnazione del tipo di indirizzo
  appaddr.sin_port = htons(SP_UDP_PORT); //assegnazione della porta del server
	appaddr.sin_addr=IP;

	char* sendbuff=create_buffer("elez", 0, "");
	
	//invio pacchetto
	if (sendto(sockfd, sendbuff, 8, 0, (struct sockaddr *)&appaddr, iplen) < 0)
  {
      perror("errore in sendto");
      return 0;
  }	
  printf("INVIATO MESSAGGIO PER ELEZIONE SUPERPEER\n");
  //attendo la risposta
  struct sigaction alarmaction = {.sa_handler = alarm_handler};
  int retcode = sigaction(SIGALRM, &alarmaction, NULL);

  if (retcode == -1) 
  {
    //controllo della giusta esecuzione della sigaction
    perror("bad sigaction()\n");
    return 0;
  }

	alarm(5);	//il timeout aumenta nelle chiamate ricorsive successive
	
	var_ack=1;//imposto a 1 in modo tale che entra nel ciclo, saranno gli eventi di SIGALRM o di recvfrom a sbloccare il ciclo	
	int received = 0;
	while(var_ack) //provo a ricevere finche non va a buon fine e finche non vado in timeout
	{
		//la recvfrom è non bloccante
		if (((recvfrom (sockfd, result, DIM_MAX_PKT, MSG_DONTWAIT , (struct sockaddr *)&servaddr, (socklen_t *)&iplen )) >= 3) && (strncmp(result,"crsp",4) == 0))
		{
			var_ack=0;
			received=1;
		}
	}
  alarm(0);

	if (received == 1)
	{
    printf("RICEVUTO CREATE SP\n");
		packetUDP* pck;
		pck=create_packet(result);
    if(strcmp(pck->cmd,"crsp")==0)
    {
      if( (pck->size > 0) )
      {
        long* app_punt;
        struct in_addr new;
        app_punt = (long*) pck->dati;
        new.s_addr = *app_punt;

        printf("CREATO SUPERPEER A CUI CONNETTERSI: %ld-%s\n",*app_punt, inet_ntoa(new));    
        int join = join_UDP(sockfd, new, rating, 5);
		    if(join==1)
        {
          conn=1;
          IPSP=new;
          return 1;
        }
        else  
        {
          conn=0;
          return 0;
        }
      }
      else
      {
        printf("NESSUN SUPERPEER CREATO\n");
        return 0;
      }
    }
    else
      printf("QUA NON CI DEVO ENTRARE\n");
  }
  else
  {
    printf("ricevuto comando non valido come risposta al comando 'elez' \n");
    return 0;
  }

	return 0;	
}


void parsePeerVar ()
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
      if( strcmp( nome_var, "PATH_FILEBLOOM") == 0)
      {
        PATH_FILEBLOOM = val_var;
      }
      if( strcmp( nome_var, "PATH_DIR") == 0)
      {
        PATH_DIR = val_var;
      }
      if( strcmp( nome_var, "PATH_LOCALE") == 0)
      {
        PATH_LOCALE = val_var;
      }
      if( strcmp( nome_var, "CHUNK") == 0)
      {
        CHUNK = atoi(val_var);        
      }
			if( strcmp( nome_var, "NUM_UPLOAD") == 0)
      {
        NUM_UPLOAD = atoi(val_var);
      }
			if( strcmp( nome_var, "BOOT_PORT_TCP") == 0)
      {
        SERV_TCP_PORT = atoi(val_var);        
      }
      if( strcmp( nome_var, "BOOT_PORT_UDP") == 0)
      {
        SERV_UDP_PORT = atoi(val_var);        
      }
    }
  }
}

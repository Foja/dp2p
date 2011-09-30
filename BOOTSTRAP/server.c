#include "server.h"

void alarm_handler()
{

  if (listLen(&list) == 0)
  {
     alarm(100);
     return;
  }

  decrementaTtl(&list);
  alarm(100);
  return;
}

int main(int argc, char **argv)
{
  int socksd, sockUDPsd, listensd, connsd;               //descrittori delle socket utilizzate dal server di bootstrap
  int k = 1, n = 0, i = 0;
  int fd1;                                               //descrittore del file
  int maxd, ready;                                       //variabili per la select 
  fd_set rset, allset;                                   //strutture dati per la gestione della select
  struct sockaddr_in servaddr, cliaddr, Speer, addr;     //Strutture per passare indirizzi alle socket
  unsigned int len = sizeof(struct sockaddr_in);         //dimensione delle sockaddr_in 
  char buffer[20];
  struct sigaction alarmaction = {.sa_handler = alarm_handler};
  int retcode = sigaction(SIGALRM, &alarmaction, NULL);

  if (retcode == -1) 
  {
     //controllo della giusta esecuzione della sigaction
     perror("bad sigaction()\n");
     return 0;
  }                    

  //inizializzazione della lista degli ip

  list.next = &list;
  list.prev = &list;

  //inizializzazione della lista della connessioni
  client.next = &client;
  client.prev = &client;

	parseServVar();

  //creazione della socket listen TCP
  if ((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("errore in socket");
    exit(1);
  }

   //creazione della socket listen UDP
   if ((sockUDPsd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
   { 
      perror("errore in socket");
      exit(1);
   }

  setsockopt(listensd, SOL_SOCKET, SO_REUSEADDR, &k, sizeof(int)); // setta l'opzione SO_REUSEADDR a true tramite il parametro k grazie alla quale è possibile
                                                               // riavviare subito il server in caso di una caduta 
  memset((void *)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(TCP_PORT);

  if ((bind(listensd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
  {
    perror("errore in bind");
    exit(1);
  }

  //la socket viene messa in listen
  if (listen(listensd, BACKLOG) < 0 )
  {
    perror("errore in listen");
    exit(1);
  }

  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); //il server accetta pacchetti su una qualunque delle sue interfacce di rete
  addr.sin_port = htons(UDP_PORT); //numero di porta del server

  if (bind(sockUDPsd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("errore in bind");
    exit(1);
  }

  FD_ZERO(&allset);                                               //Inizializza a zero l'insieme dei descrittori
  FD_SET(listensd, &allset);                                      //Inserisce il descrittore di ascolto
  FD_SET(sockUDPsd, &allset);                                     //Inserisce il descrittore della socket UDP
  maxd = (sockUDPsd < listensd) ? (listensd): (sockUDPsd);        //maxd è sempre uguale al maggiore dei descrittori da controllare con la select
  len = sizeof(cliaddr);
  alarm(100);

  for (;;) 
  {
    //all'inizio di ogni ciclo devo usare una struttura fd_set di appoggio da passare come parametro alla chiamata select in quanto quest'ultima nè altera il valore.
    rset = allset;  

    //viene fatta la select e ready contiene il numero di descrittori pronti in lettura la select inoltre altera il valore di rset.
    if ( ((ready = select(maxd+1, &rset, NULL, NULL, NULL)) < 0) && (errno == EINTR))
         continue;

    else if ((ready < 0) && (errno != EINTR))
    {
      perror("errore in select");
      exit(1);
    }
	
		//controllo sulla socket in listen
    if (FD_ISSET(listensd, &rset))
    {
      printf("listen attiva\n");
      //la socket è in attesa di connessioni

      while(1)
      {

        if ( ((connsd = accept(listensd, (struct sockaddr *)&cliaddr, &len) ) < 0) && (errno == EINTR) )
           continue;

        if ( (connsd < 0) && (errno != EINTR) )
        {
           perror("errore in accept\n");
           exit(1);
        }

        break;
      }

      //Inserisce il descrittore del nuovo socket in allset
      FD_SET(connsd, &allset);

      if (connsd > maxd)
         maxd = connsd; //aggiorno il massimo dei connsd

      inserimento(connsd, &client); //inserimento del descrittore della connessione nella lista dei client

      if (--ready <= 0) //Cicla finchè ci sono ancora descrittori pronti in lettura da controllare
         continue;
    }

    //Controlla se sono arrivati PING
    if (FD_ISSET(sockUDPsd, &rset))
    {

      while (1)
      {
        if ( ((n= recvfrom(sockUDPsd, buffer, 12, 0, (struct sockaddr *)&addr, &len)) < 0) && (errno == EINTR) )
          continue;

        if ( (n < 0) && (errno != EINTR) )
        {
          perror("errore in recvfrom del comando/size");
          exit(1);
        }

        break;
      }

      if (strncmp(buffer, "ping",4) == 0)
        reimpostaTtl(addr, &list);
    }

    //Controlla i socket attivi per controllare se sono pronti in lettura
    for (i = 0; i < listLenClient(&client); i++)
    {
      socksd = getConnsd(i, &client);

      if (FD_ISSET(socksd, &rset)) 
      {
        //Gestione della connessione da parte del server di bootstrap
        printf("conn attiva\n", socksd);

        while (1)
        {
          if ( ((n = getpeername(socksd,(struct sockaddr *)&cliaddr,&len)) < 0) && (errno == EINTR) ) 
             continue;

          if ( (n < 0) && (errno != EINTR) )
          {
             perror("errore in getpeername: ");
             exit(1);
          }

          break;
        } 
          
        strSrv(socksd, cliaddr); //eseguita funzione del server che processa la richiesta del client
        FD_CLR(socksd, &allset); //Cancella socksd da client

        //aggiorna maxd
        if (maxd == socksd)
        {
          cancellaClient(socksd, &client);

          if (listLenClient(&client) != 0)
             maxd = getMassimo(&client);

          else
           	maxd = (sockUDPsd < listensd) ? (listensd): (sockUDPsd);

        }

        else
         cancellaClient(socksd, &client);

        if (--ready <= 0) break;
    }
   }

 }
}




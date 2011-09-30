#include "superpeer.h"


void alarm_handler()
{

   if (listLenPeer(&peer) == 0)
   {
     pingSpUDP(bootaddr);
     pingSpTCP();
     alarm(60);
     return;
   }

  decrementaTtl(&peer);
  pingSpUDP(bootaddr);
  pingSpTCP();
  alarm(60);
  return;


}


int main(int argc, char **argv)
{
  parseSuperPeerVar();                        //funzione per il parsing del file config
  int	socksd, sockUDPsd, listensd, connsd;    //descrittori di socket
  int i = 0, j = 0, cont = 0;                 //contatori
  int maxd, ready;                            //variabili per la select 
  fd_set rset, allset;                        //variabili per la select
  int n = 1;
  long *ip;
  id_whohas = 1;
  struct sockaddr_in	servaddr, cliaddr, addr; 
  struct in_addr	serv_in_addr;
  socklen_t len;
  char buffer[30], *c;

  list.next= &list;                         /* INIZIALIZZAZIONE LISTA CHE CONTERRÀ GLI INDIRIZZI RESTITUITI DAL BOOTSTRAP */
  list.prev= &list;

  client.next= &client;                     /* INIZIALIZZAZIONE LISTA DEI SUPERPEER VICINI */
  client.prev= &client;

  peer.next= &peer;                         /* INIZIALIZZAZIONE LISTA DEI PEER */
  peer.prev= &peer;

  lista_query.next= &lista_query;           /* INIZIALIZZAZIONE LISTA QUERY */
  lista_query.prev= &lista_query;

   if (argc != 2) 
   {
     fprintf(stderr, "utilizzo: ./superpeer <indirizzo IP bootstrap>\n");
     exit(1);
   }    

  
  memset((void *)&serv_in_addr, 0, sizeof(serv_in_addr));
  memset((void *)&bootaddr, 0, sizeof(bootaddr));

  if (inet_pton(AF_INET, argv[1], &bootaddr) <= 0) 
  {
     fprintf(stderr, "errore in inet_pton per %s", argv[1]);
     exit(1);
  }


 
  /* inizio codice superpeer */   
  
  if ((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0) //crea la socket di ascolto TCP
  {
     perror("errore in socket");
     exit(1);
  }

  if ((sockUDPsd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) //crea il socket UDP 
  {
    perror("errore in socket");
    exit(1);
  }

  setsockopt(listensd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int)); //per poter far ripartire subito il Super peer in caso di caduta
  memset((void *)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SP_TCP_PORT);

  if ( (bind(listensd, (struct sockaddr *)&servaddr, sizeof(servaddr) ) ) < 0)
     exit(1);

  if (listen(listensd, BACKLOG) < 0 )
  {
     perror("errore in listen");
     exit(1);
  }

  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); //il superpeer accetta pacchetti su una qualunque delle sue interfacce di rete
  addr.sin_port = htons(SP_UDP_PORT); //numero di porta del superpeer per la socket udp

  //assegna l'indirizzo al socket
  if (bind(sockUDPsd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("errore in bind udp");
    exit(1);
  }

  FD_ZERO(&allset); /* Inizializza a zero l'insieme dei descrittori */
  FD_SET(listensd, &allset); /* Inserisce il descrittore di ascolto della socket TCP */
  FD_SET(sockUDPsd, &allset); /* Inserisce il descrittore della socket UDP */
  maxd = (sockUDPsd < listensd) ? (listensd): (sockUDPsd); /* maxd contiene sempre il maggiore dei descrittori da controllare con la select */

	struct sigaction alarmaction = {.sa_handler = alarm_handler};
  int retcode = sigaction(SIGALRM, &alarmaction, NULL);

  if (retcode == -1) 
  {
     //controllo della giusta esecuzione della sigaction
     perror("bad sigaction()\n");
     return 0;
  }   

   /* codice per il join con il bootstrap */
  registerSP(argv[1]);
  connessioneOverlay(&maxd, &allset);
  cancellaLista(&list);
  len = sizeof(cliaddr);
  alarm(60);

  for (;;)
  {
     rset= allset;

     if ( ((ready = select(maxd + 1, &rset, NULL, NULL, NULL)) < 0) && (errno == EINTR))
         continue;

     else if ((ready < 0) && (errno != EINTR))
     {
        perror("errore in select");
        exit(1);
     }

     if (FD_ISSET(listensd, &rset))
     {
                  
         if ((connsd = accept(listensd, (struct sockaddr *)&cliaddr, &len)) < 0)
         {
             perror("errore in accept\n");
             exit(1);
         }

         if (listLenClient(&client) >= NUMERO_MAX_SUPERPEER)
         {
            char *vicini_non_pieni = (char *) malloc(NUMERO_MAX_SUPERPEER * 4);
            int contatore_vicini = 0;
            cont=4;
            /*RESTITUIRE GLI INDIRIZZI DEI PROPRI VICINI*/
            ottieniViciniNonPieni(vicini_non_pieni, &contatore_vicini, &client);
            char* sendbuff = create_buffer_TCP("full", contatore_vicini, vicini_non_pieni);
            free(vicini_non_pieni);
            writen(connsd, sendbuff, contatore_vicini + 8);
            
            if (close(connsd) == -1)
            {
               perror("errore in close");
               exit(1);
            }

            if (--ready <= 0) /* Cicla finchè ci sono ancora descrittori leggibili da controllare */
	              continue;
         }

         /* Inserisce il descrittore del nuovo socket */
         if (listLenClient(&client) < NUMERO_MAX_SUPERPEER)
         {
            int n, p, sp;
            char dati[8], *c1, *c2;
            p = listLenPeer(&peer);
            sp = listLenClient(&client);
            p = htonl(p);
            sp = htonl(sp);
            c1 = (char *)&p;
            c2 = (char *)&sp;

            for (j = 0; j < 4; j++)
            {
               dati[j] = *(c1 + j);
               dati[4+j] = *(c2 + j);
            }

            if (writen(connsd,"ack\n",4) < 0)
            {
               perror("errore write");
               return;
            }

            if (writen(connsd,dati,8) < 0)
            {
               perror("errore write");
               return;
            }

            while(1)
            {
              if ( (n = read(connsd,buffer,8) < 0) && (errno == EINTR))
                 continue;

              else if ( (n < 0) && (errno != EINTR) ) 
              {

                 if (close(connsd) == -1)
                 {
                   perror("errore in close:");
                   exit(1);
                 }

                 exit(1);
              }

              else
              { 
                 int *peer, *s_peer;
                 peer = (int *)buffer;
                 s_peer = (int *)&buffer[4];
                 *peer = ntohl(*peer);
                 *s_peer = ntohl(*s_peer);
                 inserimento(connsd,*peer,*s_peer,&client);
                 FD_SET(connsd, &allset);
                 stampaClient(&client);
                 
                 if (connsd > maxd)
                    maxd = connsd;

                 break;
              }

            }

            if (--ready <= 0) /* Cicla finchè ci sono ancora descrittori leggibili da controllare */
               continue;
         }
     }

      if (FD_ISSET(sockUDPsd, &rset))
      {
        receiveBloomUDP(sockUDPsd, addr);

        if (--ready <= 0) /* Cicla finchè ci sono ancora descrittori leggibili da controllare */
	       continue;

      }

     /* Controlla i socket attivi per controllare se sono leggibili */
     for (i = 0; i < listLenClient(&client); i++)
     {
         /* se il descrittore non è stato selezionato viene saltato */
         socksd = getConnsd(i,&client);

         if (FD_ISSET(socksd, &rset))
         {
            packetWHHS* pck;
            packetTCP* pck_TCP;

            while(1)
            {

              if ( ((n = read(socksd, buffer, 8)) < 0) && (errno != EINTR) )
                 continue;

              if ( (n  < 0) && (errno != EINTR) )
              {
                fprintf(stderr, "errore in readline");
                exit(1);
              }


              break;
            }


            if (n != 0)
            {

              if ( (strncmp(buffer, "whhs", 4) == 0) || ( (strncmp(buffer, "ack", 3) == 0) && (buffer[3] != 'M') ) )
              {

                while(1)
                {

                   if ( ( (n = read(socksd, &buffer[8], *(int *)&buffer[4] + 5) ) < 0) && (errno != EINTR) )
                      continue;

                   if ( (n  < 0) && (errno != EINTR) )
                   {
                      fprintf(stderr, "errore in readline");
                      exit(1);
                   }

                   if (n > 0)
                     pck = create_packet_whhs(buffer);

                   break;
                }
              }

              else 
              {

                while(1)
                {

                   if ( ( (n = read(socksd, &buffer[8], *(int *)&buffer[4] + 1) ) < 0) && (errno != EINTR) )
                      continue;

                   if ( (n  < 0) && (errno != EINTR) )
                   {
                      fprintf(stderr, "errore in readline");
                      exit(1);
                   }

                   if (n > 0)
                     pck_TCP = create_packet_TCP(buffer);

                   break;
                }

              }

            }

             /******    CONNESSIONE CHIUSA DAL SUPERPEER  **************/
            if (n == 0) 
            {
                FD_CLR(socksd, &allset);   /* Cancella socksd tra i descrittori da controllare e dalla lista client */

                if (maxd == socksd)
                {
	                  cancellaClient(socksd,&client);  /* Rimuove socksd dalla lista dei socket da controllare */

                    if (listLenClient(&client) == 0)
                       maxd = (sockUDPsd < listensd) ? (listensd): (sockUDPsd);

                    else
                     maxd = getMassimo(&client);
                }

                else
                  cancellaClient(socksd, &client);

                if (close(socksd) == -1)
                {
                    perror("errore in close");
                    exit(1);
                }

                if (--ready > 0)
                   continue;

                else
                  break;
            }

            /******    GESTIONE CONNESSIONE CON ALTRI SUPERPEER  **************/
            int byte_IP_P = 0;
            int *e, n;
            struct in_addr apps;

            if ((strncmp(buffer, "whhs", 4) == 0) && (pck->id != 0))
            {  
               /* GESTIONE WHOHAS RICEVUTA IN TCP DA UN VICINO */

               char *dati_result = (char *) malloc(426);

               if (whoHasUDP(pck, dati_result, &byte_IP_P) > 0)
							 {
								 e = (int*) (dati_result);
							   apps.s_addr = *e;
               }

               *(dati_result + byte_IP_P) = ';';
							   
               e = (int*) (dati_result);
               apps.s_addr = *e;
    				     
               int byte_IPSP = 1;
               struct sockaddr_in app;
               socklen_t addrlen = sizeof(app);

               for (i = 0; i < listLenClient(&client); i++)
               {

                if (getConnsd(i, &client) != socksd)
                {

                  while(1)
                  {
                    if ( ( (n = getpeername(getConnsd(i, &client),(struct sockaddr *) &app, &addrlen) ) < 0) && (errno == EINTR) )
                       continue;

                    else if ( (n < 0) && (errno != EINTR) )
                    {
                       perror("bad getpeername on superpeer\n");
                       return -1;
                    }

                    else
                     break;

                  }

                  char *addr = (char *) &app.sin_addr.s_addr;

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
 			
                char* sendbuff = create_buffer_whhs("ack\0", byte_IPSP + byte_IP_P, pck->id, dati_result);
                writen(socksd, sendbuff, 13 + byte_IPSP + byte_IP_P );

            }

            else if ((strncmp(buffer, "ack", 3) == 0) && (searchId(pck->id, &lista_query) != 0))
            {
               char *dati_result = (char *) malloc(425);
               int cont = 0;
               cont = parsingWhoHas(pck, dati_result);
               inoltraWhoHas(pck->id, sockUDPsd);
               struct sockaddr_in *addr_peer = getPeerQuery(pck->id, &lista_query);
               char* sendbuff = create_buffer_whhs("ack\0", cont, pck->id, dati_result);
               
               if (sendto(sockUDPsd, sendbuff, 13 + cont , 0, (struct sockaddr *) addr_peer, sizeof(*addr_peer)) < 0) 
               {
                  perror("errore in sendto");
                  return 0;
               }

            }

            else if (strncmp(buffer,"ping",4) == 0)
            {
              int *p,*sp;
              p= (int *)pck_TCP->dati;    
              sp= (int *)(pck_TCP->dati+4);
              aggiornaStatistiche(socksd, *p, *sp, &client);
            }
           
            else if (strncmp(buffer, "mrge", 4) == 0)
            {
              double rat = *((double *) &buffer[8]);

              if (rat > rating)
              {
                 struct sockaddr_in superpeer, *peer_figli;
                 socklen_t len = sizeof(superpeer);

                 while(1)
                 {
                    int n;
                    if ( ( (n = getpeername(socksd,(struct sockaddr *) &superpeer, &len)) < 0) && (errno == EINTR) )
                       continue;

                    else if ( (n < 0) && (errno != EINTR) )
                    {
                       perror("bad getpeername on superpeer\n");
                       return -1;
                    }

                    else
                     break;

                 }

                 int i, num_peer;
                 char* app = (char *)&superpeer.sin_addr.s_addr;
                 char* sendbuff = create_buffer_TCP("ackM", 4, app);
                 writen(socksd, sendbuff, 12);
                 num_peer = listLenPeer(&peer);
                 free(sendbuff);
                 sendbuff = create_buffer("leav", 4, app);

                 for (i = 0; i < num_peer; i++)
                 {
                    if ( (peer_figli = getPeer(i, &peer) ) != NULL)
                       sendto(sockUDPsd, sendbuff, 12, 0, (struct sockaddr *)peer_figli, len );
                 }
                 
                 exit(0);
              }

              else
              {
                  char* app = (char *)&my_addr.sin_addr.s_addr;
                  char* sendbuff = create_buffer_TCP("ackM", 4, app);
                  writen(socksd, sendbuff, 12);
              }

            }

            else if (strncmp(buffer, "ackM", 4) == 0)
            {
               struct sockaddr_in app, *peer_figli;
               int num_figli, bootsd;
               char* app_ind;
               app.sin_addr.s_addr = *( (long *) &buffer[8]);

               if (app.sin_addr.s_addr != my_addr.sin_addr.s_addr) 
               {
                 /*DEVO CONTATTARE IL BOOTSTRAP E TUTTI I MIEI PEER CHE MI STÒ SCOLLEGANDO*/
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

                 num_figli = listLenPeer(&peer);
                 app_ind = (char *)&buffer[8];
                 char* sendbuff = create_buffer("leav", 4, app_ind);
                 for (i = 0; i < num_figli; i++)
                 {
                     if ( (peer_figli = getPeer(i, &peer) ) != NULL)
                       sendto(sockUDPsd, sendbuff, 12, 0, (struct sockaddr *)peer_figli, len );
                 }

               exit(0);
               }

            }

            if (--ready <= 0)
               break;
         }
     }
  }
}


#include "peer.h"

float elapsed;

void ping_handler()
{
  int respng;

  if(conn==1)
  {
    /* calcolo del rating dovuto al tempo */
    ftime(&t);
    elapsed = (t.millitm-t0.millitm)*1.0e-3 + (t.time-t0.time);	//tempo da cui è attivo il peer
    if( elapsed >= MAX_TIME)
      ratT=1;
    else
      ratT = elapsed/MAX_TIME;

    rating = ratC + ratT; //aggiorno il rating
    int j;

    /* invio del ping */
    respng=0;
    for( j=0; j<5 && respng!=1; j++)
      respng = ping_UDP(sockUDPsd, IPSP, 1);

	  printf("%d => ping del peer al superpeer associato = %d\n", getpid(), respng );

    /* controllo sul risultato del ping */
    if(respng == 0)
    {
      printf("%d => errore nell'ping al superpeer, tentativo di connessione ad un altro SP\n",getpid());
      respng = change_SP(0); //dato che il SP non risponde, il peer prova aconnettersi ad un altro
      if(respng == 0)
      {
        printf("%d => nessun SP disponibile, elezione superpeer\n", getpid());
        //nessun SP disponibile ne va eletto uno nuovo
        elezione_nuovo_SP();
      }
    }
  }
  else
  {
    /* peer non connesso a nessun SP */
    printf("%d => peer non connesso a nessun SP, tentativo di connessione ad uno di essi\n", getpid());
    respng = connect_to_SP();
    if ( respng != 1 )
      elezione_nuovo_SP(); //nessun superpeer disponibile, viene chiamata la funzione di elezione
  }
  
  if(conn==1)
  {
    printf("----------------------------------\n");
    printf("%d ==> #COMANDI\n" , getpid());
	  if( ricerca == 0 )
      printf("1: LEAVE\n");
		printf("2: UPDATE\n");
		if( (ricerca == 0) && (conn == 1) )
  	  printf("3: WHOHAS\n");
    if( ricerca == 1 )	
      printf("4: STOP (della ricerca)\n");
	  printf("=>inserire il numero della scelta:\n");
    printf("----------------------------------\n");
  }
}

int main(int argc, char **argv)
{
  /* inizializzazione variabili per processi*/
  conn = 0;       //variabile che viene impostata ad 1 se il peer è connesso ad un SP
  pid_up = -1;    //pid del processo di upload
  pid_SP = -1;    //pid del processo di superpeer (se esiste) 
  parsePeerVar(); //lettura file di configurazione
  
  /* creazione processi */
  pid_up = fork();  
  if(pid_up == 0) 
    upload();
  else
  {
    /* fork per processo di main (figlio) e processo di controllo (padre) */
    int pid_main = fork(); 
    if(pid_main == 0)
		{
      /* PROCESSO MAIN */
      /* inizializzazione rating (rating massimo 3, 2 per il ratingC e 1 per il ratingT) */
	    ratC = ratingC(); 
	    ratT = 0 ; 
      ftime(&t0); 
      rating= ratC + ratT;

      /* inizializzazione liste */	    
      list.next=&list;
      list.prev=&list;
    
      list_result.next=&list_result;
      list_result.prev=&list_result;
    
      /* controllo parametri */    
      if (argc != 2) 
      {
         fprintf(stderr, "%d ==> utilizzo: peer <indirizzo IP server>\n", getpid() );
         exit(1);
      }    
		  ip_server = argv[1];

      /* AVVIO CODICE PEER */
	    Peer();
    }
	  else
    {
      /* PROCESSO DI CONTROLLO */
      for(;;)
      {
        pid_t var = waitpid(pid_main, NULL, WNOHANG); //waitpid non bloccante per controllare se il processo di main è attivo o no
        if(var != 0) 
        {
          exitP();
          return 0;
        }
        sleep(45);
        kill( pid_main, SIGUSR1); //invio segnale di SIGUSR1 al processo di main per far scattare il ping       
      }
    }
    return 1;
  }
  return 0;
}

void Peer()
{
  /* creazione e salvataggio filtro di bloom */	
	unlink (PATH_DIR);
	open_dir ( PATH_LOCALE, PATH_DIR, 1 ); //scansione cartella PATH_LOCALE e salvataggio risultati in PATH	
	BLOOM *bloom = create_bloom_file(PATH_DIR); //a partire dal file in PATH creazione del filtro di bloom
 	bloom_destroy(bloom);

  /* creazione della socket */
  if ((sockUDPsd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {				
    fprintf(stderr, "%d => errore in socket udp\n", getpid());
		return ;
  }

  struct sockaddr_in addr;
  memset((void *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  addr.sin_port = htons(PEER_UDP_PORT); 

  if (bind(sockUDPsd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    fprintf(stderr, "%d => errore in bind udp\n", getpid());
    exit(1);
  }

  printf("%d ==> CODICE PEER\n", getpid());

  /* connesione al superpeer */
  int join_sp = 0;
  join_sp = connect_to_SP();
  if (join_sp != 1)
    join_sp = elezione_nuovo_SP(); //se il peer non riesce a connettersi a nessun superpeer viene avviata la fase di elezione
  if (join_sp != 1)
  {
    pid_SP = fork();
    if(pid_SP == 0)
      Superpeer();
    join_sp = join_UDP(sockUDPsd, my_ip, rating, 5);
    if(join_sp == 1)
    {
      IPSP = my_ip;
      conn = 1;
    }
  }
  if (join_sp == 1)
  {
		printf("%d ==> join effettuato a %s\n", getpid(), inet_ntoa(IPSP));
  
    /* associazione del segnale di SIGUSR1 al gestore del ping */
    struct sigaction alarmactionping = {.sa_handler = ping_handler};
		int retcodeping = sigaction(SIGUSR1, &alarmactionping, NULL);
   	if (retcodeping == -1) 
		{
    	perror("bad sigaction()\n");
			return;
    }
      
    /* inizio codice select */
    fd_set rset, allset;
	  int maxd;
	  int ready;
    char* s = malloc(255);
    char* search = malloc(255);
	  char 	buffer;
   
	  FD_ZERO (&allset);
	  FD_SET	(fileno(stdin), &allset);
    FD_SET  (sockUDPsd, &allset);
    maxd = (sockUDPsd < fileno(stdin)) ? (fileno(stdin)): (sockUDPsd);
    ricerca = 0; //a 1 se ci sono ricerche in corso
    printf("----------------------------------\n");
    printf("%d ==> #COMANDI\n", getpid());
	  if( ricerca == 0 )
      printf("1: LEAVE\n");
		printf("2: UPDATE\n");
		if( (ricerca == 0) && (conn == 1) )
      printf("3: WHOHAS\n");
    if( ricerca == 1 )	
      printf("4: STOP (della ricerca)\n");
	  printf("=>inserire il numero della scelta:\n");
    printf("----------------------------------\n");

    for( ; ; )
	  {
		  rset = allset;
		  ready = 0;
		  if ( ((ready = select(maxd+1, &rset, NULL, NULL, NULL)) < 0) && (errno == EINTR))
        continue;

      if ((ready < 0) && (errno != EINTR))
      {
        perror("errore in select");
        exit(1);
      }

      if( FD_ISSET (sockUDPsd, &rset) )
      {
        char result[DIM_MAX_PKT];
        struct sockaddr_in addr; 
        socklen_t iplen = sizeof(addr); 
        int n, j;

        /* lettura del buffer di ricezione udp */
        while (1)
        {
          if ( ((n= recvfrom(sockUDPsd, result, DIM_MAX_PKT, 0, (struct sockaddr *)&addr, &iplen)) < 0) && (errno == EINTR) )
            continue;
          if ( (n < 0) && (errno != EINTR) )
          {
            perror("errore in recvfrom del pacchetto udp");
            return;
          }
          else
            break;
        }        

        /* analisi e processamento del messaggio ricevuto */

        /* comando che identifica la ricerca è stata bloccata dal superpeer */
        if( strncmp(result, "stop", 4) == 0 )
        {
          if(ricerca==1)
          {
            printf("%d => FINE RICERCA , ip trovati: %d\n", getpid(), Listlen(&list_result));
            if(Listlen(&list_result)>0)
            {
              Stampa(&list_result);
              printf("%d ==> digitare 1 se si vuole scaricare il file\n", getpid());

              for( ; ; )
              {
  			        if ((fgets(s, 255, stdin) == NULL) && (errno == EINTR)) 
                  continue;           
          			else 
                  break;
              }

              int r = atoi (s);
              if( r == 1 ) 
              {
                struct sockaddr_in *ip_down = GET_LIST( (Listlen(&list_result) - 1), &list_result);     
                printf("%d => AVVIO DEL DOWNLOAD DI %s dal peer %s\n", getpid(), search, inet_ntoa(ip_down->sin_addr));    
                struct timeb inizio,fine;
                double time_elapsed;
                ftime(&inizio);
                int resdown = download (ip_down->sin_addr, search); //restituisce 1 se il download va a buon fine, 0 altrimenti
                ftime(&fine);
                time_elapsed = (fine.millitm - inizio.millitm) * 1.0e-3 + (fine.time - inizio.time);	
                
                if (resdown == 1)                
                  printf("%d => TEMPO IMPIEGATO PER IL DOWNLOAD = %f\n", getpid(), time_elapsed);
                else
                  printf("%d => DOWNLOAD NON COMPLETATO CORRETTAMENTE\n", getpid());
              }
            }
            ricerca = 0;
            cancella_lista(&list_result);  
          }
        }

        /* ricevuto messaggio di "NeW SuperPeer", il peer è stato eletto SP*/
        if( strncmp(result,"nwsp",4) == 0 )
        {  
          printf("%d => ricevuto nwsp da: %s\n", getpid(), inet_ntoa(addr.sin_addr));
          packetUDP* pck = create_packet (result);
          char *res;
          int my_join = 0;      
          
          if((conn != 1) || ((conn == 1) && (IPSP.s_addr != my_ip.s_addr)))
          {
            if( conn == 1)
              leave_UDP(sockUDPsd, IPSP, 5);
            /* avvio codice superpeer */
            if ((pid_SP = fork() ) == 0) 
            {
              Superpeer();
            } 
            IPSP = my_ip; 
            conn = 1;
            sleep(1);
            my_join = join_UDP (sockUDPsd, my_ip, rating , 5);
          }
          else
          {
            printf ("%d => ricevuto nwsp comand, ma il peer corrente è già un SP\n", getpid());
            my_join = join_UDP (sockUDPsd, my_ip, rating , 5);
          }          

          if(my_join == 1)
          {
            conn = 1;
            res="ackE";
          }
          else
          {
            conn = 0;
            res="nakE";
          }
          printf("%d => RISPOSTA A NWSP: %s\n", getpid(), res);	
          /* INVIO RISULTATO OPERAZIONE */
          char* sendbuff;
          if(pck->size == 0)
          {
            sendbuff = create_buffer(res, 0, "");	

            if ( sendto(sockUDPsd, sendbuff, 8 + (pck->size) , 0, (struct sockaddr *)&addr, iplen) < 0) 
            {
              perror("errore in sendto");
	            return ;
            }
          }
          else
          {
            sendbuff = create_buffer(res, 1, "s");	

            if ( sendto(sockUDPsd, sendbuff, 9 , 0, (struct sockaddr *)&addr, iplen) < 0) 
            {
              perror("errore in sendto");
	            return ;
            }
          }
        }

        /* ricevuto messaggio di "leave", il peer deve fare la leave al SP e connettersi all'ip che viene inviato insieme al messaggio di leave */
        if( strncmp(result,"leav",4) == 0 )
        {  
          printf("%d => ricevuto leave da: %s\n", getpid(), inet_ntoa(addr.sin_addr));
          char *res;
          packetUDP* pck;
          pck = create_packet(result);
          struct in_addr newinaddr;
          newinaddr.s_addr = *((long*) pck->dati);

          int join = 0;      
          /* se il peer è connesso a un superpeer deve fare la leave da esso */
          if(conn == 1)
            leave_UDP(sockUDPsd, IPSP, 5);
          if(pck->size == 0)
          {
            conn = 0;
            res = "nak";
          }
          else
          {          
            join = join_UDP (sockUDPsd, newinaddr, rating , 5);
            if(join == 1)
            {
              IPSP = newinaddr;
              conn =  1;
              res = "ack";
            }
            else 
            {
              conn = 0;
              change_SP (0);
              res = "nak";
            }
          }
          printf("%d => RISPOSTA A LEAVE: %s\n", getpid(), res);	
        }

        /* ack di risposta ad una whohas */
        if ( (strncmp(result, "ack", 3) == 0) && (*(result + 3) != 'E') )
        {      
          printf("%d => ricevuto ack da: %s\n", getpid(), inet_ntoa(addr.sin_addr));  
          packetWHHS* pck;
	        pck = create_packet_whhs(result);
          int numero_ip = 0;
          numero_ip = pck->size >> 2;
          printf("%d => numero di ip ricevuti nella risposta alla whohas: %d\n", getpid(), numero_ip);

          /* salvataggio degli ip ricevuti*/
	        struct sockaddr_in ip[numero_ip];
          long *app;

          for (j= 0; j < numero_ip; j++)
          {	
            app = (long *)(pck->dati + j*4);
            ip[j].sin_addr.s_addr = *app;
          }
	
	        printf("%d => LISTA IP RESTITUITI\n", getpid());
	        for (j = 0; j < numero_ip; j++)
	        {
            if(ip[j].sin_addr.s_addr != my_ip.s_addr)
            {      
              Ins_testa(ip[j], &list_result);
		          printf("%d => %d = %s\n", getpid(), j, inet_ntoa(ip[j].sin_addr));	
            }
	        }
        }

        if (--ready <= 0) /* Cicla finchè ci sono ancora descrittori leggibili da controllare */
		     continue;
      }

		  if( FD_ISSET (fileno(stdin), &rset) )
		  {
        while(1)
        {
          char* app = fgets (s, 255, stdin);
	    		if( (app == NULL) && (errno == EINTR))
            continue;
	    		if( (app == NULL) && (errno != EINTR))
            perror("errore nell'fgets");
          else 
            break;
        }

		    int x = atoi(s);
	
        /* leave dal sistema */
  	    if ( x == 1)
	      {	
				  if( ricerca == 1 )
          {
            printf("%d => RICERCA IN CORSO, NON È POSSIBILE EFFETTUARE L'OPERAZIONE!\n", getpid());          
          }
          else
          {
            if( conn == 1 )
              printf("%d => leave : %d\n", getpid(), leave_UDP(sockUDPsd, IPSP, 5) );
            else 
              printf("%d => leave dal sistema\n", getpid());
            
            exitP();
            exit(0);
          }
			  }
			
        /* stampa dei file locali e update di tali dati al SP */
			  if ( x == 2 ) 
			  {
          if( conn == 1)
          {
            printf("%d ==> LISTA DEI FILE DEL PEER\n", getpid());
            /* aggiornamento del file contenente la lista */
            unlink (PATH_DIR);
          	open_dir ( PATH_LOCALE, PATH_DIR, 1 ); 

            /* stampa di tale file */
			      FILE* fd;				
            while(1)
  		      {				
              fd = fopen(PATH_DIR, "r");
	  	        if( (fd == NULL) && (errno == EINTR) ) 
                continue;
              if( (fd == NULL) && (errno != EINTR) ) 
                perror("errore in open del file");
			        else 
                break; 	
			      }
              
            printf("--------------------------------\n");
            int n;
            
				    while ( ((n = fread(&buffer, 1, 1, fd)) > 0))
            {
					    if(n > 0)
                printf("%c", buffer);
				    }
            printf("\n--------------------------------\n");

            /* calcolo nuovo filtro di bloom */
	          BLOOM *bloom = create_bloom_file(PATH_DIR); 
            bloom_destroy(bloom);

            /* update dei dati del peer */
            join_sp = join_UDP(sockUDPsd, IPSP, rating , 5);
            if(join_sp == 0)
            {
              printf("%d => ERRORE nell'update al superpeer\n", getpid());
              join_sp = change_SP(0); 
              if(join_sp == 0)
              {
                printf("%d => nessun SP disponibile per la join/update\n", getpid());
                join_sp = elezione_nuovo_SP();
                if (join_sp != 1)
                {
                  pid_SP = fork();
                  if(pid_SP == 0)
                    Superpeer();
                  join_sp = join_UDP(sockUDPsd, my_ip, rating, 5);
                  if(join_sp == 1)
                  {
                    IPSP = my_ip;
                    conn = 1;
                  }
                }
              }
            }
            else
              conn = 1;
          }
          else
          {
            printf("%d => NON È POSSIBILE FARE L'UPDATE IN QUANTO NON SI È CONNESSI A NESSUN SUPERPEER\n", getpid());
            join_sp = change_SP(0); 
            if(join_sp == 0)
            {
              printf("%d => nessun SP disponibile per la join/update\n", getpid());
              join_sp = elezione_nuovo_SP();
              if (join_sp != 1)
              {
                pid_SP = fork();
                if(pid_SP == 0)
                  Superpeer();
                join_sp = join_UDP(sockUDPsd, my_ip, rating, 5);
                if(join_sp == 1)
                {
                  IPSP = my_ip;
                  conn = 1;
                }
              }
            }
          }
			  }

				/* codice whohas */
			  if ( x == 3 )
			  {
          if( conn != 1 ) 
          {
            printf("%d => PEER NON CONNESSO A NESSUN SUPERPEER IMPOSSIBILE EFFETTUARE LA RICERCA\n", getpid() );
            join_sp = change_SP(0); 
            if(join_sp == 0)
            {
              printf("%d => nessun SP disponibile per la join/update", getpid());
              join_sp = elezione_nuovo_SP();
              if (join_sp != 1)
              {
                pid_SP = fork();
                if(pid_SP == 0)
                  Superpeer();
                join_sp = join_UDP(sockUDPsd, my_ip, rating, 5);
                if(join_sp == 1)
                {
                  IPSP = my_ip;
                  conn = 1;
                }
              }
            }
          }
          else
          {
            if( ricerca == 1 )
            {
              printf("%d => RICERCA IN CORSO, NON È POSSIBILE EFFETTUARE L'OPERAZIONE!\n", getpid() );          
            }
            else
            {
			        printf("%d => quale file si vuole cercare?\n", getpid());
              while(1)
              {
                char* app = fgets ( search, 255, stdin);
	          		if( (app == NULL) && (errno == EINTR))
                  continue;
	          		if( (app == NULL) && (errno != EINTR))
                  perror("errore nell'fgets");
                else 
                  break;
              }

              *(search + strlen(search) - 1) = '\0';          
			        ricerca = 1; //ricerca in corso
              int whhs_sp = whohas_UDP(sockUDPsd, IPSP, search, 5);
              if(whhs_sp == 0)
              {
                conn = 0;
                printf("%d => il superpeer non risponde, tentavo di connessione ad un altro\n", getpid());
                whhs_sp = change_SP(0); 
                if(whhs_sp == 0)
                {
                  printf("%d => nessun SP disponibile per la join\n",getpid());
                  join_sp = elezione_nuovo_SP();
                  if (whhs_sp != 1)
                  {
                    pid_SP = fork();
                    if(pid_SP == 0)
                      Superpeer();
                    whhs_sp = join_UDP(sockUDPsd, my_ip, rating, 5);
                    if(whhs_sp == 1)
                    {
                      IPSP = my_ip;
                      conn = 1;
                    }
                  }
                }
                else 
                {
                  printf("%d => nuovo superpeer collegato, ritentativo di whohas\n", getpid());
                  whhs_sp = whohas_UDP(sockUDPsd, IPSP, search, 5);
                  conn = whhs_sp;
                }
              }
              printf("%d => esito della whohas di '%s'  : %d\n", getpid(), search , whhs_sp );
            }
          }
		    }
         
        /* stop and download */
        if ( x == 4 )
        { 
          if(ricerca == 1)
          {
            int stop;
            stop = stop_UDP (sockUDPsd, IPSP , 5);
            conn = stop;
            ricerca = 0;
            printf("%d ==> FINE RICERCA , ip trovati: %d\n", getpid(), Listlen(&list_result));
            if(Listlen(&list_result) > 0)
            {
              Stampa(&list_result);
              printf("%d => digitare 1 se si vuole scaricare il file\n", getpid());

              for( ; ; )
              {
  			        if ((fgets(s,255,stdin) == NULL) && (errno == EINTR)) 
                  continue;
          			else 
                  break;
              }
              x = atoi (s);
              if( x == 1 ) 
              {
                struct sockaddr_in *ip_down = GET_LIST( (Listlen(&list_result) - 1), &list_result);     
                printf("%d => AVVIO DEL DOWNLOAD DI %s dal peer %s\n", getpid(), search, inet_ntoa(ip_down->sin_addr));    
                struct timeb inizio,fine;
                double time_elapsed;
                ftime(&inizio);
                int resdown = download (ip_down->sin_addr, search); //restituisce 1 se il download va a buon fine, 0 altrimenti
                ftime(&fine);
                time_elapsed = (fine.millitm-inizio.millitm)*1.0e-3 + (fine.time-inizio.time);	//tempo da cui è attivo il peer
                if(resdown == 0)
                {
                  srand((int)time(NULL));
                  int indice = rand() % Listlen(&list_result);
                  ip_down = GET_LIST( indice, &list_result);     
                  printf("%d =>PRIMO TENTATIVO DI DOWNLOAD NON RIUSCITO, AVVIO DEL DOWNLOAD DI %s dal peer %s\n", getpid(), search, inet_ntoa(ip_down->sin_addr));    
                  ftime(&inizio);
                  resdown = download (ip_down->sin_addr, search); //restituisce 1 se il download va a buon fine, 0 altrimenti
                  ftime(&fine);
                  time_elapsed = (fine.millitm-inizio.millitm)*1.0e-3 + (fine.time-inizio.time);	//tempo da cui è attivo il peer
                }
                if(resdown == 1)
                  printf("%d => TEMPO IMPIEGATO PER IL DOWNLOAD = %f\n", getpid(), time_elapsed);
                else 
                  printf("%d => DOWNLOAD NON RIUSCITO CORRETTAMENTE\n", getpid());
              }
            }
            cancella_lista(&list_result);  
            if(stop == 0)
            {
              printf("%d => il superpeer non risponde, provo a connettermi ad un altro\n",getpid());
              stop = change_SP(0); 
              if(stop == 0)
              {
                printf("%d => nessun SP disponibile per la join/update\n",getpid());
                stop = elezione_nuovo_SP();
                if (stop != 1)
                {
                  pid_SP = fork();
                  if(pid_SP == 0)
                    Superpeer();
                  stop = join_UDP(sockUDPsd, my_ip, rating, 5);
                  if(stop == 1)
                  {
                    IPSP = my_ip;
                    conn = 1;
                  }
                }
              }
              else 
              {
                printf("%d => nuovo superpeer collegato\n",getpid());
              }
            }
          }
          else
            printf("%d => nessuna ricerca in corso\n", getpid());    
        }
        printf("----------------------------------\n");
        printf("%d ==> #COMANDI\n" , getpid());
	      if( ricerca == 0 )
          printf("1: LEAVE\n");
		    printf("2: UPDATE\n");
		    if( (ricerca == 0) && (conn == 1) )
          printf("3: WHOHAS\n");
        if( ricerca == 1 )	
          printf("4: STOP (della ricerca)\n");
	      printf("=>inserire il numero della scelta:\n");
        printf("----------------------------------\n");
			  if (--ready <= 0) /* Cicla finchè ci sono ancora descrittori leggibili da controllare */
			    continue;
		  }
    }
    //fine codice select
  }
  else
    printf("%d ==> IMPOSSIBILE CONNETTERSI RIPROVARE PIU TARDI\n", getpid());
	printf ("fine codice del peer\n");
  return;
}


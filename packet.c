#include "packet.h"

/**** FUNZIONI PER PACCHETTI WHHS *********/

packetWHHS* create_packet_whhs (char* buff)
{
	int i;
	packetWHHS* pck=malloc(sizeof(packetWHHS));

	pck->cmd=malloc(5);

	//ricezione del comando
  for (i= 0; i < 4; i++)
      *(pck->cmd+i)=buff[i];

  *(pck->cmd+4)='\0';
  pck->size= *((int*)&buff[4]); //leggo la dimensione
  pck->id= *((int*)&buff[8]); //leggo l'id della query
  pck->dati=malloc(pck->size+1);
	
	for (i=0; i < pck->size; i++)
	{
		//copio i dati letti
		*(pck->dati+i)=buff[12+i];
		//printf("%c",*(pck->dati+i));
	}
	*(pck->dati+pck->size)= '\0';

	printf("*** %d => comando ricevuto=%s\n",getpid(), pck->cmd);
	printf("*** %d => size=%d\n",getpid(), pck->size);
  printf("*** %d => ID=%d\n", getpid(), pck->id);
  printf("*** %d => dati=%s\n", getpid(), pck->dati);
	return pck;		
}

char* create_buffer_whhs (char* cmd, int size, int id, char* data)
{		
  int i;
  char* buff=malloc(12 + size);
	//inizializzo buff come cmd + size + filter
  for (i=0; i < 4; i++)
	   buff[i]=*(cmd+i);

	char* app;
	app=(char*) &size;
  for(i=0; i < 4; i++)
	   buff[4+i]= *(app+i);

  app=(char*) &id;
  for(i=0; i < 4; i++)
	   buff[8+i]= *(app+i);


	for(i=0; i < size; i++)
	{
		buff[12+i]=*(data+i);	
	}

  buff[12+size]= '\n';
	return buff;		
}

void destroy_packet_whhs(packetWHHS* pck)
{
	free(pck->cmd);
	free(pck->dati);
	free(pck);
	return;
}

void print_packet_whhs (packetWHHS* pck)
{
	printf("--STAMPA PACCHETTO UDP\n");
	printf("--comand= %s\n",pck->cmd);
  printf("--id= %d\n",pck->id);
	printf("--size= %d\n",pck->size);
}


/**** FUNZIONI PER PACCHETTI UDP *********/


packetUDP* create_packet (char* buff)
{
        int i;
        packetUDP* pck=malloc(sizeof(packetUDP));

        pck->cmd=malloc(5);

        //ricezione del comando
        for (i= 0; i < 4; i++)
	          *(pck->cmd+i)=buff[i];

	      *(pck->cmd+4)='\0';
        pck->size= *((int*)&buff[4]); //leggo la dimensione
        pck->dati=malloc(pck->size+1);
       
        for(i=0;i<pck->size;i++)
        {
                //copio i dati letti
                *(pck->dati+i)=buff[8+i];
                //printf("%c",*(pck->dati+i));
        }
        *(pck->dati+pck->size)='\0';


        

        printf("*** %d => comando ricevuto=%s\n",getpid(), pck->cmd);
        printf("*** %d => size=%d\n",getpid(), pck->size);
        return pck;             
}

//rilascia la memoria del pacchetto passato
void destroy_packet(packetUDP* pck)
{
        free(pck->cmd);
        free(pck->dati);
        free(pck);
        return;
}

//funzione per la creazione di un buffer leggendolo dai campi di un pacchetto UDP 
char* create_buffer (char* cmd, int size, char* data)
{
        int i;
        char* buff=malloc(8 + size);
        //inizializzo buff come cmd + size + filter

        for (i= 0; i < 4; i++)
            buff[i]=*(cmd + i);
        
        char* app;
        app=(char*) &size;

        for (i= 0; i < 4; i++)
            buff[4+i]= *(app + i);

        for (i= 0; i < size; i++)
        {
            buff[8+i]=*(data + i);    
        }

        return buff;            
}


/**** FUNZIONI PER PACCHETTI TCP *********/

packetTCP* create_packet_TCP (char* buff)
{
	int i;
	packetTCP* pck=malloc(sizeof(packetWHHS));

	pck->cmd=malloc(5);

	//ricezione del comando
  for (i= 0; i < 4; i++)
      *(pck->cmd+i)=buff[i];

  *(pck->cmd+4)='\0';
  pck->size= *((int*)&buff[4]); //leggo la dimensione
  pck->dati=malloc(pck->size+1);
	
	for (i=0; i < pck->size; i++)
	{
		//copio i dati letti
		*(pck->dati+i)=buff[8+i];
		//printf("%c",*(pck->dati+i));
	}

	*(pck->dati+pck->size)= '\0';
	printf("*** %d => comando ricevuto=%s\n",getpid(), pck->cmd);
	printf("*** %d => size=%d\n",getpid(), pck->size);
  return pck;		
}

char* create_buffer_TCP (char* cmd, int size, char* data)
{		
  int i;
  char* buff=malloc(8 + size);
	//inizializzo buff come cmd + size + filter
  for (i=0; i < 4; i++)
	   buff[i]=*(cmd+i);

	char* app;
  app=(char*) &size;

  for(i=0; i < 4; i++)
	   buff[4+i]= *(app + i);


	for(i=0; i < size; i++)
		buff[8+i]=*(data + i);	

	return buff;		
}

void destroy_packet_TCP(packetTCP* pck)
{
	free(pck->cmd);
	free(pck->dati);
	free(pck);
	return;
}

void print_packet_TCP (packetTCP* pck)
{
	printf("--STAMPA PACCHETTO UDP\n");
	printf("--comand= %s\n",pck->cmd);
	printf("--size= %d\n",pck->size);
}


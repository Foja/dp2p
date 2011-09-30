#include "utility.h"
/* prende in input una stringa che identifica un numero decimale e lo restituisce come numero */
double float_convert(char *argv)
{
	int i, j, Lyyy;
	char xxx[32], yyy[32];
	int x, y;
	double z1, z2, risult;
		
	i = 0;
	while ((*(argv + i)) != '.') {
//		printf("i= %d, c= %c\n", i, *(argv[1] + i));
		xxx[i] = *(argv + i);
		i++;
		}
	xxx[i] = '\0';
	x = atoi(xxx);
	//printf("x= %d\n", x);
	i++;	/* salto il carattere '.' */
	j = i;
	Lyyy = 0;
	while ((*(argv + i)) > 0) {
//		printf("i= %d, c= %c\n", i, *(argv[1] + i));
		yyy[i - j] = *(argv + i);
		i++;
		Lyyy++;
		}
	yyy[i - j] = '\0';
	y = atoi(yyy);
	//printf("y= %d\n", y);
	z1 = x;
	//printf("z1= %f\n", z1);
	z2 = y;
	for (i=0; i<Lyyy; i++)
		z2 = z2 / 10.0;
	//printf("z2= %f\n", z2);
	risult = z1 + z2;
	//printf("risult= %f\n", risult);
	return risult;
}



/* concatena a dest i primi n byte di src */
char* mystrncat(char *dest, const char *src, size_t n)
           {
               size_t dest_len = strlen(dest);
               size_t i;
							char *new=malloc(dest_len+n+1);
							for (i = 0 ; i < dest_len ; i++)
              {     
									new[i] = dest[i];		
							}
               for (i = 0 ; i < n && src[i] != '\0' ; i++)
              {     
									new[dest_len + i] = src[i];		
							}
               new[dest_len + i] = '\0';

               return new;
           }

/* calcolo rating caratteristiche di un peer */
double ratingC() {

  struct sysinfo sys_info;

  if(sysinfo(&sys_info) != 0)
    perror("sysinfo");

	//RATING RAM
	unsigned long mem=sys_info.totalram*sys_info.mem_unit;
	float ram = (float) (mem / 1024) / 1024;
  //printf("RAM:%ld\n\n\n\n",sys_info.totalram);
	float rating_ram = (float) ram / RAM_MAX;
	if(rating_ram>1)rating_ram=1;

	printf("%d => rating ram = %f\n", getpid(), rating_ram);

	// Find the value of CPU frequency
	int fd=open("/proc/cpuinfo", O_RDONLY, 0644);
	char c;
	int tf=0;

	double cpu[8];
	//int cont_cpu=0;
	char *string;
	string="";	
	int j=0;
	while(read(fd,&c,1)>0)
	{
		if(c=='G')
		{
			if(read(fd,&c,1)>0)
			{
				if(c=='H')
				{
					tf=-1;
					//printf("\n-->%s\n",string);
					if(strcmp(string,"")!=0)
					{
						cpu[j]=float_convert(string);
						string="";
						j++;			
					}					
				}					
			}
		}		
		if(tf==1 ) 
		{
			//printf("%c", c );
			string=mystrncat(string, &c, 1 );
		}
		if(c=='@')tf=1;				
	}

	//int k;
	//for(k=0;k<j;k++){
	//	printf("%d-->%f\n",k,cpu[k]);
	//}

	double mhz = (double) cpu[0];
	double rating_cpu = (double) mhz / CPU_MAX;
	if(rating_cpu>1)rating_cpu=1;
	printf("%d => rating cpu = %f\n", getpid(), rating_cpu);
	
	double rating=rating_cpu+rating_ram;
  return rating;
}


/* scandisce la directory (ricorsivamente se rec==1) e salva tutti i nomi dei file trovati */
void open_dir(char *dir, char *dest, int rec)
{
  DIR *dirp;
	struct dirent de, *dep;
  int fd1;
  char *c1;
	dep = &de;
	dirp = opendir(dir);
  c1=(char *)malloc(255);
        
  if (!dirp) 
  {
    perror("bad opendir()\n");
	  return;
	}

  if ( (fd1=open(dest, O_CREAT | O_WRONLY , 0666) ) < 0) perror("errore:");
  
  /* inizio scansione directory */
	while ((dep = readdir(dirp)))
  {
		if ( (strcmp(dep->d_name,".") != 0) && (strcmp(dep->d_name,"..") != 0) && (dep->d_type != 4) )
    {
    	//printf("%s\n",dep->d_name);
      lseek(fd1,0,SEEK_END);
		  write(fd1,strcat(dep->d_name,"\n"), strlen(dep->d_name) + 1 );
    }

                    
    if ( (strcmp(dep->d_name,".") != 0) && (strcmp(dep->d_name,"..") != 0) && (*(dep->d_name) != '.') && (dep->d_type == 4) && rec)
    {
    	strcpy(c1,dir);
      c1=strcat(c1,"/");
      c1=strcat(c1,dep->d_name);
      //printf("%s\n",c1);
      open_dir(c1,dest,rec);
    }
	}
	/* fine scansione directory */

  close(fd1);
	closedir(dirp);
        
  return;
}

/* scandisce la directory (ricorsivamente se rec==1) e restituisce 1 se il file è stato trovato, 0 altrimenti */
int search_file(char *dir, char *file, int rec)
{
  char* file_lowercase= toLowerCase (file);
  DIR *dirp;
	struct dirent de, *dep;
  int fd1;
  char *c1;
	dep = &de;
	dirp = opendir(dir);
  c1=(char *)malloc(255);
        
  if (!dirp) 
  {
    perror("bad opendir()\n");
	  return -1;
	}
	
  //inizio scansione directory
	while ((dep = readdir(dirp)))
  {
		//scandisco file per file per vedere se c'è il file cercato
		if ( (strcmp(dep->d_name,".") != 0) && (strcmp(dep->d_name,"..") != 0) && (dep->d_type != 4) )
    {
			char* d_name_lowercase= toLowerCase (dep->d_name);
			if(strcmp(d_name_lowercase , file_lowercase) == 0) 
      {
			  return 1;
      }
    }

    //se vengono trovate delle cartelle viene fatta la chiamata ricorsiva su di esse
    if ( (strcmp(dep->d_name,".") != 0) && (strcmp(dep->d_name,"..") != 0)  && (*(dep->d_name) != '.') && (dep->d_type == 4) && rec)
    {
    	strcpy(c1,dir);
      c1=strcat(c1,"/");
      c1=strcat(c1,dep->d_name);
      if(search_file(c1,file,rec)==1)return 1; //se una chiamata ricorsiva trova il file restituendo 1 anche la funzione principale restituisce 1
    }
	}
	//fine scansione directory

 	closedir(dirp);
  
	//se non è stato trovato il file ne in questa cartella ne in nessuna chiamata ricorsiva la funzione restituisce 0
  return 0; 
}

/* scandisce la directory, ricorsivamente se rec=1 altrimenti scorre solo la cartella corrente, e restituisce il numero dei file */
int count_file(char *dir, int rec)
{
  DIR *dirp;
	struct dirent de, *dep;
  int fd1;
  char *c1;
	dep = &de;
	dirp = opendir(dir);
  c1=(char *)malloc(255);
  int res=0;

  if (!dirp) 
  {
    perror("bad opendir()\n");
	  return;
	}
	
  //inizio scansione directory
	while ((dep = readdir(dirp)))
  {
		//scandisco file per file per vedere se c'è il file cercato
		if ( (strcmp(dep->d_name,".") != 0) && (strcmp(dep->d_name,"..") != 0) && (dep->d_type != 4) )
    {
			res++; //ogni file trovato incremento del contatore
    }

    //se vengono trovate delle cartelle viene fatta la chiamata ricorsiva su di esse
    if ( (strcmp(dep->d_name,".") != 0) && (strcmp(dep->d_name,"..") != 0) &&(dep->d_type == 4) && rec )
    {
    	strcpy(c1,dir);
      c1=strcat(c1,"/");
      c1=strcat(c1,dep->d_name);
      res+=count_file(c1,rec); //il risultato della ricerca è uguale ai file trovati nella cartella fin ora più il numero di file trovati nelle sottocartelle
    }
	}
	//fine scansione directory

 	closedir(dirp);
  
	 return res; 
}



/* scandisce la directory (ricorsivamente se rec==1) e restituisce 1 se il file è stato trovato, 0 altrimenti */
char* search_path_file(char *dir, char *file, int rec)
{
  char* file_lowercase= toLowerCase (file);
  DIR *dirp;
  struct dirent de, *dep;
  int fd1;
  char *c1;
  dep = &de;
  dirp = opendir(dir);
  c1=(char *)malloc(255);
  int loop=1;

      
  if (!dirp) 
  {
    perror("bad opendir()\n");
	  return;
	}

  //inizio scansione directory
	while ((dep = readdir(dirp)) && loop==1 )
  {
		

		//scandisco file per file per vedere se c'è il file cercato
		if ( (strcmp(dep->d_name,".") != 0) && (strcmp(dep->d_name,"..") != 0) && (dep->d_type != 4) )
    {
      char* d_name_lowercase= toLowerCase (dep->d_name);
			if(strcmp(d_name_lowercase , file_lowercase) == 0) 
      {
  			loop==0;
			  return dep->d_name;
      }
    }

    //se vengono trovate delle cartelle viene fatta la chiamata ricorsiva su di esse
    if ( (strcmp(dep->d_name,".") != 0) && (strcmp(dep->d_name,"..") != 0) && (*(dep->d_name) != '.') && (dep->d_type == 4) && rec)
    {

	strcpy(c1,dir);
  c1=strcat(c1,dep->d_name);
	char* result=malloc(1000);
	memset(result, 0, 1000);
	char* res;
	
	res=search_path_file(c1,file,rec);
	int cmp=strcmp(res,"");     

		if(cmp!=0)
		{
			loop==0;
			//printf("percorso file: %s/%s\n", c1,res); 
			result=strcat(result,dep->d_name);
			result=strcat(result,"/");
			result=strcat(result,res);
			return result; //se una chiamata ricorsiva trova il file restituendo 1 anche la funzione principale restituisce 1
		}


        }
   }
	//fine scansione directory

 	closedir(dirp);
  
	//se non è stato trovato il file ne in questa cartella ne in nessuna chiamata ricorsiva la funzione restituisce 0

	return ""; 
}
char* toLowerCase (char* str)
{
  char* res=malloc(strlen(str)+1);
  int i;
  for (i = 0; i<strlen(str); i++)
  *(res+i) = tolower( str[i] );
  *(res+i)='\0';
  return res;
}


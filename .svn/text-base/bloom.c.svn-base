//palarssl.org


#include "bloom.h"
#include "utility.h"




/* funzione che crea un filtro di bloom e lo inizializza con dimensioni e funzioni, questa funzione non inserisce nessun contenuto nel filtro si occupa solo della creazione dello spazio di memoria */
/* PARAMETRI = dimensione del filtro in byte, numero funzioni da usare, puntatori alle funzioni */
BLOOM *bloom_create(size_t size, size_t nfuncs, ...)
{
	BLOOM *bloom;
	va_list l; //questa struttura è usata come parametro per ricevere gli argomenti addizionali di una funzione
	int n;
	
	//creazione dei spazi di memoria per contenere i vari campi del filtro di bloom
	if(!(bloom=malloc(sizeof(BLOOM)))) return NULL;
	if(!(bloom->a=calloc((int)((size+CHAR_BIT-1)/CHAR_BIT), sizeof(char)))) {
		free(bloom);
		return NULL;
	}
	if(!(bloom->funcs=(hashfunc_t*)malloc(nfuncs*sizeof(hashfunc_t)))) {
		free(bloom->a);
		free(bloom);
		return NULL;
	}

	va_start(l, nfuncs);
	for(n=0; n<nfuncs; ++n) {
		bloom->funcs[n]=va_arg(l, hashfunc_t);
	}
	va_end(l);

	bloom->nfuncs=nfuncs;
	bloom->asize=size;

	return bloom;
}

/* distrugge il filtro passato per riferimento liberando la memoria occupata da quest'ultimo */
int bloom_destroy(BLOOM *bloom)
{
	free(bloom->a);
	free(bloom->funcs);
	free(bloom);

	return 0;
}

/* aggiunge un elemento di tipo stringa al filtro */
int bloom_add(BLOOM *bloom, const char *s)
{
	size_t n;															
  s=toLowerCase((char*) s );
	for(n=0; n<bloom->nfuncs; ++n) {
		SETBIT(bloom->a, bloom->funcs[n](s)%bloom->asize);
	}

	return 0;
}

/* controlla se un elemento è presente nel filtro, quest'ultimo però può restituire dei falsi positivi */
int bloom_check(BLOOM *bloom, const char *s)
{

  s=toLowerCase((char*) s );
	size_t n;
	char *p;
	if((p=strchr(s, '\r'))) *p='\0';
	if((p=strchr(s, '\n'))) *p='\0';
	for(n=0; n<bloom->nfuncs; ++n) {
		if(!(GETBIT(bloom->a, bloom->funcs[n](s)%bloom->asize))) return 0;
	}

	return 1;
}

/* semplice funzione hash per il filtro di bloom */
unsigned int sax_hash(const char *key)
{
	unsigned int h=0;

	while(*key) h^=(h<<5)+(h>>2)+(unsigned char)*key++;

	return h;
}

/* semplice funzione hash per il filtro di bloom */
unsigned int sdbm_hash(const char *key)
{
	unsigned int h=0;
	while(*key) h=(unsigned char)*key++ + (h<<6) + (h<<16) - h;
	return h;
}


/* funzioni avanzate del filtro di bloom 
 * creazione filtro di bloom a partire da un file contenente una lista
 * caricamento di un filtro di bloom già salvato
*/

/* crea un file contenente il filtro di bloom creato a partire dai dati di un file passato come parametro */
BLOOM* create_bloom_file(char* path)
{
	FILE *fp; 
	char line[1024];
	char *p;
	BLOOM *bloom;
	
	//controllo sull'input della funzione
	if(path==NULL) {
		fprintf(stderr, "ERROR: No file specified\n");
		return NULL;
	}

	//creazione del filtro
	if(!(bloom=bloom_create(DIM_FILTER, 4, sax_hash, sdbm_hash, sax_hash,  sdbm_hash ))) {
		fprintf(stderr, "ERROR: Could not create bloom filter\n");
		return NULL;
	}
	
	//apertura dello stream del file da leggere
	if(!(fp=fopen(path, "rb"))) {
		fprintf(stderr, "ERROR: Could not open file %s\n", path);
		return NULL;
	}

	//lettura del file e aggiunta nel filtro riga per riga
	int j;
	j=0;
	while (fread (&line[j], 1 , 1 , fp) == 1)
 {
		if(line[j] == '\r')
		{
			line[j]='\0';
			bloom_add(bloom, line);
			j=0;
			continue;
		}
		if(line[j] == '\n') 
		{
			line[j]='\0';
			bloom_add(bloom, line);
			j=0;
			continue;
		}
		j++;
	}
	//printf("%s\n",bloom->a); //stampa fittizia della stringa del filtro
	fclose(fp); //chiusura dello stream di input in quanto non devo più leggere dal file
	
	/* stampa su file la lista bloomata */
/*	int fd1;
	if ( (fd1=open("./filebloom", O_CREAT | O_WRONLY | O_TRUNC , 0666) ) < 0) perror("errore durante la read");

	if(write(fd1,bloom->a,real_dim_filter)<0) perror("errore in write");
	
	if(close(fd1)<0)
	{
		perror ("errore in close");
		exit(1);
	}
*/
	FILE *fd;
	if ( (fd = fopen("./bloom/filebloom", "wb")) == NULL ) perror("errore durante la open");
	int n;
	if((n=fwrite(bloom->a,real_dim_filter,1,fd)) != 1)
	{
		perror("errore in scrittura filtro");
	}
	//printf("numero di caratteri salvati su file = %d",(n*real_dim_filter));
	if((fclose(fd)) != 0) perror("errore nella close");

	return bloom;
}

/* carico un filtro di bloom */
/* crea un file con i dati compressi */
BLOOM* load_bloom_file(char* path)
{
	FILE *fp;
	char line[1024];

	BLOOM *bloom;
	
	if(path==NULL) {
		fprintf(stderr, "ERROR: No file specified\n");
		return NULL;
	}

	//creazione del filtro
	if(!(bloom=bloom_create(DIM_FILTER, 4, sax_hash, sdbm_hash, sax_hash,  sdbm_hash ))) {
		fprintf(stderr, "ERROR: Could not create bloom filter\n");
		return NULL;
	}

	//printf("dimensione --> %d\n",bloom->asize);
	if(!(fp=fopen(path, "rb"))) {
		fprintf(stderr, "ERROR: Could not open file %s\n", path);
		return NULL;
	}

	//copia del contenuto del filtro di bloom
	//viene copiato solo il campo "a" in quanto gli altri campi vengono ricreati essendo tutti uguali nell'implementazione dei filtri nel progetto
	/*while(fgets(line, real_dim_filter , fp)) {
		strcat((char *)(bloom->a),line);
	}*/
	
	
	if(fread(bloom->a,real_dim_filter,1,fp) != 1)
	{
		perror("errore nella read del filtro");
	}
	

	if(fclose(fp)!=0)
	{
		perror ("errore in close");
		exit(1);
	}

	return bloom;
}


BLOOM* create_bloom_struct(char* content)
{
	BLOOM *bloom;

	//creazione del filtro
	if(!(bloom=bloom_create(DIM_FILTER, 4, sax_hash, sdbm_hash, sax_hash,  sdbm_hash ))) {
		fprintf(stderr, "ERROR: Could not create bloom filter\n");
		return NULL;
	}

	bloom->a = content;
	
	return bloom;
}









#ifndef __BLOOM_H__
#define __BLOOM_H__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <features.h>
#include <time.h>

#define DIM_FILTER 8000 //dimensione del filtro di bloom
#define DIM_MAX_PKT 1500 //dimensione massima dei pacchetti UDP ricevibili

#define SETBIT(a, n) (a[n/CHAR_BIT] |= (1<<(n%CHAR_BIT))) //per i filtri di bloom
#define GETBIT(a, n) (a[n/CHAR_BIT] & (1<<(n%CHAR_BIT)))

#define real_dim_filter ((DIM_FILTER+CHAR_BIT-1)/CHAR_BIT)

typedef unsigned int (*hashfunc_t)(const char *);
typedef struct {
	size_t asize;
	unsigned char *a;
	size_t nfuncs;
	hashfunc_t *funcs;
} BLOOM;

/* funzioni di base del filtro di bloom */
BLOOM *bloom_create(size_t size, size_t nfuncs, ...);
int bloom_destroy(BLOOM *bloom);
int bloom_add(BLOOM *bloom, const char *s);
int bloom_check(BLOOM *bloom, const char *s);

/* funzioni hash per la criptazione nei filtri di bloom */
unsigned int sax_hash(const char *key);
unsigned int sdbm_hash(const char *key);

/* funzioni avanzate del filtro di bloom 
 * creazione filtro di bloom a partire da un file contenente una lista
 * caricamento di un filtro di bloom già salvato
*/

BLOOM* create_bloom_struct(char* content);
BLOOM* create_bloom_file(char *c);
BLOOM* load_bloom_file(char *c);
#endif


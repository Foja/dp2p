#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#define RAM_MAX 4096; //valore massimo di ram in MB
#define CPU_MAX 3; //valore massimo di cpu in MHz


double float_convert(char *argv);
char* mystrncat(char *dest, const char *src, size_t n);
double ratingC ();
void open_dir(char *dir , char *dest, int rec);
int search_file(char *dir , char *file, int rec);
int count_file(char *dir, int rec);
char* search_path_file(char *dir, char *file, int rec);
char* toLowerCase (char* str);


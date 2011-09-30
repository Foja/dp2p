#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  int id;
  char* cmd;
  int size;
  char* dati;
} packetWHHS;

typedef struct {
        char* cmd;
        int size;
        char* dati;
} packetUDP;

typedef struct {
        char* cmd;
        int size;
        char* dati;
} packetTCP;

packetUDP* create_packet (char* buff);
char* create_buffer (char* cmd, int size, char* data);
void destroy_packet(packetUDP* pck);
void print_packet (packetUDP* pck);

packetWHHS* create_packet_whhs (char* buff);
char* create_buffer_whhs (char* cmd, int size, int id, char* data);
void destroy_packet_whhs (packetWHHS* pck);
void print_packet_whhs (packetWHHS* pck);

packetTCP* create_packet_TCP (char* buff);
char* create_buffer_TCP (char* cmd, int size, char* data);
void destroy_packet_TCP(packetTCP* pck);
void print_packet_TCP (packetTCP* pck);


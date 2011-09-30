#!bin/sh

gcc -o PEER/peer PEER/peer.c utility.c PEER/func_peer.c bloom.c packet.c
gcc -o PEER/superpeer SUPERPEER/superpeer.c utility.c SUPERPEER/func_SP.c bloom.c packet.c
gcc -o BOOTSTRAP/server BOOTSTRAP/server.c BOOTSTRAP/funz_server.c

all: superpeer peer

superpeer: superpeer.c utility.c func_SP.c bloom.c
	gcc -o superpeer superpeer.c utility.c func_SP.c bloom.c

peer: peer.c utility.c func_peer.c bloom.c
	gcc -o peer peer.c utility.c func_peer.c bloom.c

clean:
	rm peer superpeer

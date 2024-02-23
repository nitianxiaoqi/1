all:server client

server:
	gcc -o server server.c -lsqlite3

client:
	gcc -o client client.c -lsqlite3

arm:
	arm-linux-gnueabihf-gcc server.c -o server -I/home/linux/rootfs/include/ -L/home/linux/rootfs/lib/ -lsqlite3

install:

	cp server msgSys.db ~/rootfs/manage_sys/
clean:
	rm -rf server client
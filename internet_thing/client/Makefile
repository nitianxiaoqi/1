all:client


client:
	arm-linux-gnueabihf-gcc -o client client.c project.c -lpthread

install:
	mv client ~/rootfs/

clean:
	rm -rf client
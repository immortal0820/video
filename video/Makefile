all:
	arm-linux-gcc *.c  -o provideo -L /home/farsight/jpeg_9/lib/ -ljpeg -lpthread -lsqlite3 -I /home/farsight/jpeg_9/include/
	cp provideo /home/farsight/rootfs/video/
	cp *.html  /home/farsight/rootfs/video/
clean:
		rm -rf provideo  
		rm -rf /home/farsight/rootfs/video/provideo


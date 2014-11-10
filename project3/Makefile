CFLAGS=-Wall -g

project: pr ps

pr: packet_receiver.c mm.o
	gcc $(CFLAGS) -o packet_receiver packet_receiver.c mm.o
ps: packet_sender.c mm.o
	gcc $(CFLAGS) -o packet_sender packet_sender.c mm.o
mm.o: mm.c
	gcc -c mm.c
test: mm.o
	gcc $(CFLAGS) -o main_mm main_mm.c mm.o
	gcc $(CFLAGS) -o main_malloc main_malloc.c mm.o
	./main_mm
	./main_malloc
clean:
	rm -rf mm.o packet_sender packet_receiver main_mm main_malloc
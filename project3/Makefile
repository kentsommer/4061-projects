project: pr ps

pr: packet_receiver.c mm.o
	gcc -o packet_receiver packet_receiver.c mm.o
ps: packet_sender.c mm.o
	gcc -o packet_sender packet_sender.c mm.o
mm.o: mm.c
	gcc -c mm.c

clean:
	rm -rf mm.o packet_sender packet_receiver
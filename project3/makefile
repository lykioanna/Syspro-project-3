all: myhttpd mycrawler

myhttpd: myhttpd.o Arguments.o
	gcc myhttpd.o Arguments.o -o myhttpd -lpthread

mycrawler: mycrawler.o ArgumentsCrawler.o queue.o
		gcc mycrawler.o ArgumentsCrawler.o  queue.o -o mycrawler -lpthread

myhttpd.o : myhttpd.c
		gcc -c -g myhttpd.c -lpthread

mycrawler.o : mycrawler.c
		gcc -c -g mycrawler.c -lpthread

Arguments.o: Arguments.c
		gcc -c -g Arguments.c

ArgumentsCrawler.o: ArgumentsCrawler.c
		gcc -c -g ArgumentsCrawler.c

queue.o : queue.c
		gcc -c -g queue.c

clean:
		rm -f *.o myhttpd

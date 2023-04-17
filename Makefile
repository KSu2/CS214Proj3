CC = gcc 
CFLAGS = -std=c99 -g -Wall -fsanitize=address

serve: echoserv1.o
	$(CC) $(CCFLAGS) $^ -o $@

echoserv1.o: echoserv1.c
	$(CC) $(CCFLAGS) -c $^

xmit: xmit.o
	$(CC) $(CCFLAGS) $^ -o $@

xmit.o: xmit.c
	$(CC) $(CCFLAGS) -c $^

ttt: ttt.o
	$(CC) $(CCFLAGS) $^ -o $@

ttt.o: ttt.c
	$(CC) $(CCFLAGS) -c $^

ttts: ttts.o message.o
	$(CC) $(CCFLAGS) $^ -o $@

ttts.o: ttts.c
	$(CC) $(CCFLAGS) -c $^

message: messages.o
	$(CC) $(CCFLAGS) $^ -o $@

message.o: message.c
	$(CC) $(CCFLAGS) -c message.c

clean: 
	rm -rf  *.o myshell
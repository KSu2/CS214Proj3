CC = gcc 
CFLAGS = -std=c99 -g -Wall -fsanitize=address

ttt: ttt.o
	$(CC) $(CCFLAGS) $^ -o $@

ttt.o: ttt.c
	$(CC) $(CCFLAGS) -c $^

ttts: ttts.o message.o ttt_game.o
	$(CC) $(CCFLAGS) -pthread $^ -o $@

ttts.o: ttts.c
	$(CC) $(CCFLAGS) -c $^

message: messages.o
	$(CC) $(CCFLAGS) $^ -o $@

message.o: message.c
	$(CC) $(CCFLAGS) -c message.c

ttt_game: ttt_game.o
	$(CC) $(CCFLAGS) $^ -o $@

ttt_game.o: ttt_game.c
	$(CC) $(CCFLAGS) -c ttt_game.c

test_client: test_client.o
	$(CC) $(CCFLAGS) $^ -o $@

test_client.o: test_client.c
	$(CC) $(CCFLAGS) -c test_client.c

clean: 
	rm -rf  *.o myshell
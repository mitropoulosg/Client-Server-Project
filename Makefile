OBJS    = poller.o functions_server.o
CC      = gcc
FLAGS   =  -Wall -g -c
EXEC 	= poller

poller: $(OBJS)
		$(CC) $(OBJS) -lpthread -o $(EXEC)

poller.o: poller.c
	$(CC) $(FLAGS) poller.c

functions_server.o: functions_server.c
	$(CC) $(FLAGS) functions_server.c

clean:
	rm -f $(OBJS) $(EXEC)

OBJS2   = pollSwayer.o functions_client.o
EXEC2 	= pollSwayer

pollSwayer: $(OBJS2)
		$(CC) $(OBJS2) -lpthread -o $(EXEC2)

pollSwayer.o: pollSwayer.c
	$(CC) $(FLAGS) pollSwayer.c

functions_client.o: functions_client.c
	$(CC) $(FLAGS) functions_client.c

clean2:
	rm -f $(OBJS2) $(EXEC2)


CC=g++
FLAGS=-std=c++11 -Wall -O -ggdb3
SRC_C=client.cpp
SRC_S=server.cpp
SRC_SHARED=tands.cpp util.cpp
OBJ_C=$(SRC_C:.cpp=.o)
OBJ_S=$(SRC_S:.cpp=.o)
OBJ_SHARED=$(SRC_SHARED:.cpp=.o)

assign3: client server gen_man

client: $(OBJ_C) $(OBJ_SHARED)
	$(CC) $(FLAGS) -o client $(OBJ_C) $(OBJ_SHARED)
client.o: client.cpp
	$(CC) $(FLAGS) -c client.cpp

server: $(OBJ_S) $(OBJ_SHARED)
	$(CC) $(FLAGS) -o server $(OBJ_S) $(OBJ_SHARED)
server.o: server.cpp
	$(CC) $(FLAGS) -c server.cpp

tands.o: tands.cpp
	$(CC) $(FLAGS) -c tands.cpp
util.o: util.cpp
	$(CC) $(FLAGS) -c util.cpp

gen_man: server.1 client.1
	groff -man server.1 > server.ps | ps2pdf server.ps server.pdf
	groff -man client.1 > client.ps | ps2pdf client.ps client.pdf
	rm *.ps

clean:
	@rm -f *.o *.ps *.pdf client server
cleanlogs:
	@rm -r logs
	@mkdir logs

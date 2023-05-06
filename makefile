CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-pthread
SOURCES_CLIENT=client_skeleton.c
SOURCES_SERVER=server_skeleton.c
OBJECTS_CLIENT=$(SOURCES_CLIENT:.c=.o)
OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)
EXECUTABLE_CLIENT=client
EXECUTABLE_SERVER=server

all: $(EXECUTABLE_CLIENT) $(EXECUTABLE_SERVER)

$(EXECUTABLE_CLIENT): $(OBJECTS_CLIENT)
	$(CC) $(LDFLAGS) $^ -o $@

$(EXECUTABLE_SERVER): $(OBJECTS_SERVER)
	$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -I. $< -o $@

clean:
	rm -f $(EXECUTABLE_CLIENT) $(EXECUTABLE_SERVER) $(OBJECTS_CLIENT) $(OBJECTS_SERVER)
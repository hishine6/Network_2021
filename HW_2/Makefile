CC = g++
CFLAGS = -Wall
OBJECTS1 = linkstate_20161566.o
OBJECTS2 = distvec_20161566.o
TARGET1 = linkstate_20161566
TARGET2 = distvec_20161566

all: $(TARGET1) $(TARGET2)



$(TARGET1) : $(OBJECTS1)
	      $(CC) $(CFLAGS) -o $(TARGET1) $(OBJECTS1)

$(TARGET2) : $(OBJECTS2)
		$(CC) $(CFLAGS) -o $(TARGET2) $(OBJECTS2)

clean:
	rm -rf $(OBJECTS1) $(OBJECTS2) $(TARGET1) $(TARGET2) $(TARGET1).dSYM $(TARGET2).dSYM

run1:
	./$(TARGET1) topology.txt messages.txt changes.txt

run2:
	./$(TARGET2) topology.txt messages.txt changes.txt

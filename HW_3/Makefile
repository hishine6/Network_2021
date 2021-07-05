CC = g++
CFLAGS = -Wall
OBJECTS1 = crc_encoder_20161566.o
OBJECTS2 = crc_decoder_20161566.o
TARGET1 = crc_encoder_20161566
TARGET2 = crc_decoder_20161566

GENERATOR = 1101
DATAWORD = 8

ERROR_RATE = 0.05
SEED = 1001

all: $(TARGET1) $(TARGET2)

$(TARGET1) : $(OBJECTS1)
	      $(CC) $(CFLAGS) -o $(TARGET1) $(OBJECTS1)

$(TARGET2) : $(OBJECTS2)
		$(CC) $(CFLAGS) -o $(TARGET2) $(OBJECTS2)


run:
	./crc_encoder_20161566 input.txt output_prev.txt $(GENERATOR) $(DATAWORD)	
	./linksim output_prev.txt output_next.txt $(ERROR_RATE) $(SEED)
	./crc_decoder_20161566 output_next.txt recover.txt result.txt $(GENERATOR) $(DATAWORD)
	cat result.txt

com:
	./compare

clean:
	rm -rf $(OBJECTS1) $(OBJECTS2) $(TARGET1) $(TARGET2) $(TARGET1).dSYM $(TARGET2).dSYM


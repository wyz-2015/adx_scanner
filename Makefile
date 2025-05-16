SRC = main.c adx_file.c
INCLUDE = ./
TARGET = adx_scanner

CC = gcc
CFLAGS = -I$(INCLUDE) #-g

$(TARGET) : $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)

#TARGET_I = adx_scanner.i
#.PHONY : debug
#debug : $(SRC)
#	$(CC) $(SRC) -E $(CFLAGS) -o $(TARGET_I)

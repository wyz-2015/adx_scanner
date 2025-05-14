SRC = main.c adx_file.c huge_file_buffer.c
INCLUDE = ./
TARGET = adx_scanner

CC = gcc
CFLAGS = -I$(INCLUDE)

$(TARGET) : $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)

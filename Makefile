SRC = main.c adx_file.c
INCLUDE = ./

TARGET = adx_scanner
ifeq ($(OS),Windows_NT)
	TARGET = adx_scanner.exe
endif

CC = gcc
CFLAGS = -I$(INCLUDE) #-g

$(TARGET) : $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)


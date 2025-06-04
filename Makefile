SRC = main.c adx_file.c
INCLUDE = -I./
TARGET = adx_scanner
CFLAGS = $(INCLUDE)
CC = gcc

LIB = -largp
ifeq ($(OS),Windows_NT)
	TARGET = adx_scanner.exe
	CFLAGS += -static $(LIB)
endif

$(TARGET) : $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)


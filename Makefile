CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

TARGET = minidb

SRC = src/main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

CC = gcc
CFLAGS = -I
DEPS = client.h

SRC = client.o crawler.c
TARGET = crawler


crawler: library $(SRC)
		$(CC) -static -L tidy_library -I tidy_library/include -o crawler crawler.c client.o -ltidys -g
client.o: library client.c
		$(CC) -c -o client.o -L tidy_library -I tidy_library/include client.c -ltidys
library:
		cd tidy_library && cmake . && make
clean:
		rm -f $(TARGET) *.o
		cd tidy_library && make clean

APPNAME   = mbtest.exe

CC        = gcc
CCFLAGS   = -c -g -Wall -IC:\MinGW\msys\1.0\local\include\modbus

LD        = gcc
LDFLAGS   = -LC:\MinGW\msys\1.0\local\lib

OBJS      = mbtest.o

LIBS      = -lmodbus


all: $(APPNAME)

clean:
	rm -f $(OBJS)

cleanall:
	rm -f $(OBJS) $(APPNAME)


$(APPNAME): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -o $@ $<

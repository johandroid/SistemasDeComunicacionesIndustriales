APPNAME   = zbjoin

CC        = arm-linux-gnueabihf-gcc
CCFLAGS   = -c -g -Wall

LD        = arm-linux-gnueabihf-gcc
LDFLAGS   =-L../../lib

OBJS      = zbjoin.o

LIBS      =


all: $(APPNAME)

clean:
	rm -f $(OBJS)

cleanall:
	rm -f $(OBJS) $(APPNAME)

deploy:
	scp $(APPNAME) root@192.168.7.2:/root/sci
	ssh root@192.168.7.2 chmod 755 /root/sci/$(APPNAME)

run:
	ssh root@192.168.7.2 gdbserver :5000 /root/sci/$(APPNAME)


$(APPNAME): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -o $@ $<

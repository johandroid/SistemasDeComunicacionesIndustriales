APPNAME   = mbserver

CC        = arm-linux-gnueabi-gcc
CCFLAGS   = -c -g -Wall

LD        = arm-linux-gnueabi-gcc
LDFLAGS   =-L/home/johan/Documentos/PracticasSisCom

OBJS      = mbserver.o

LIBS      =-lmodbus


all: $(APPNAME)

clean:
	rm -f $(OBJS)

cleanall:
	rm -f $(OBJS) $(APPNAME)

deploy:
	scp $(APPNAME) root@192.168.7.2:/home/root/sci
	ssh root@192.168.7.2 chmod 755 /home/root/sci/$(APPNAME)

run:
	ssh root@192.168.7.2 gdbserver :5000 /home/root/sci/$(APPNAME)


$(APPNAME): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -o $@ $<

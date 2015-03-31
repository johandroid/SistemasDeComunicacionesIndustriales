LINK_TARGET = mbmonitor
OBJS =  $(LINK_TARGET).o

REBUILDABLES = $(OBJS) $(LINK_TARGET)

all : $(LINK_TARGET)
	#file $(LINK_TARGET)
	@echo done

$(LINK_TARGET) : $(OBJS)
	gcc -g -o $@ $^ -lmodbus

%.o : %.c
	gcc -g -o $@ -c $<

clean : 
	rm -f $(REBUILDABLES)

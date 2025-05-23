serv = server
cl = client
lib = lib

all: se/$(serv) cl/$(cl)

se/$(serv): se/$(serv).c
	gcc se/$(serv).c lib/se_$(lib).c -o se/$(serv) 

cl/$(cl): cl/$(cl).c
	gcc cl/$(cl).c lib/cl_$(lib).c -o cl/$(cl) -lncurses -ltinfo

clean:
	rm -f se/$(serv) cl/$(cl)

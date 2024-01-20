COPT:=-g
server: server.c
	gcc ${COPT} -o $@ $^ -lpthread

clean: 
	rm server
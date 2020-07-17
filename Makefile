CC=gcc
CFLAGS=
SRCS= $(wildcard progs/*.c)
EXECS=$(SRCS:%.c=%)

all: shell progs

shell: shell.c
	$(CC) $(CFLAGS) $^ -o $@

progs: $(EXECS)
	
.PHONY: clean
clean:
	/bin/rm -f shell
	/bin/rm -rf -f $(EXECS)
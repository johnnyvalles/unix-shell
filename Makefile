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
	/bin/rm shell
	/bin/rm -rf $(EXECS)
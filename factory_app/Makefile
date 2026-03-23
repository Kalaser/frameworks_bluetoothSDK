CC ?= cc
CFLAGS ?= -Wall -Wextra -Werror -std=c11 -O2

SRCS := main.c runner.c report.c cases/case_smoke.c platform/platform_stub.c
OBJS := $(SRCS:.c=.o)

all: ft_runner

ft_runner: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) ft_runner

.PHONY: all clean

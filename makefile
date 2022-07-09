#regles de construction
.SUFFIXES:	.o .c

.c.o:
	$(CC) -c $(CFLAGS) $<

TARGET	= ProgrammeA ProgrammeB chat_child

SRCS	=$(CFILES)

OBJS	=$(CFILES:.c=.o)


DEFINES = -D_POSIX_SOURCE

OPT	=-Wall

CFLAGS	=$(DEFINES) $(INCLUDES) $(OPT)

CC	=gcc

CFILES	= ProgrammeA.c ProgrammeB.c chat_child.c

default:	$(TARGET)

clean:
	rm  $(TARGET)

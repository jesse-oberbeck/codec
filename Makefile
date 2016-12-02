.DEFAULT_GOAL := default #found at stackoverflow.com/questions/2057689/how-make-app-knows-default-target-to-build-if-no-target-is-specified

TARGET=decode encode

CPPFLAGS+=-Wall -Wextra -Wpedantic -Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline

decode: codec_functions.o
encode: codec_functions.o

CFLAGS+=-std=c11

LDLIBS+=-lm

.PHONY: clean debug profile

default: decode encode

clean:
	rm decode encode codec_functions.o
debug:CFLAGS+=-g
debug:$(TARGET)

profile: CFLAGS+=-pg
profile: LDFLAGS+=-pg
profile: $(TARGET)

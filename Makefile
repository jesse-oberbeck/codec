.DEFAULT_GOAL := default #found at stackoverflow.com/questions/2057689/how-make-app-knows-default-target-to-build-if-no-target-is-specified

TARGET=decoder encoder

CPPFLAGS+=-Wall -Wextra -Wpedantic -Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline

decoder: decode_functions.o

CFLAGS+=-std=c11 -g

LDLIBS+=-lm

.PHONY: clean debug profile default

default: decoder encoder

clean:$(RM) $(TARGET)
debug: CFLAGS+=-g
debug: $(TARGET)
profile: CFLAGS+=-pg
profile: LDFLAGS+=-pg
profile: $(TARGET)

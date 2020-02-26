GCC = gcc
SOURCES = puissance.c
OPT = -o
TARGET = puissance
LIBS = -lm

all:
	@$(GCC) $(SOURCES) $(OPT) $(TARGET) $(LIBS)

clean:
	@rm $(TARGET)

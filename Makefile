RM	= rm -f
CC	= clang
CFLAGS	= -Wall -O2 -std=c11 -I./deps
LDFLAGS	= -lm -lglfw -lGL

SOURCES	= \
	main.c \
	deps/glad.c

OBJECTS	= $(SOURCES:.c=.o)

DEPENDS	= $(OBJECTS:.o=.d)

EXECUTABLE= main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) -c -MMD $(CFLAGS) $< -o $@

run: all
	./$(EXECUTABLE)

time: all
	time ./$(EXECUTABLE)

clean:
	$(RM) $(OBJECTS) $(DEPENDS)

distclean: clean
	$(RM) $(EXECUTABLE)

-include $(DEPENDS)

# $@ target of the rule
# $< first prerequisite



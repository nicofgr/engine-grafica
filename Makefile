CC = clang
CPPFLAGS = -I./include/  $(shell pkg-config --cflags freetype2)
#CFLAGS = -std=c99 -Wall -Wextra -Werror -fsanitize=address
CFLAGS = -std=c99 -Wall -Wextra -fsanitize=address
LIBS = -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm $(shell pkg-config --libs freetype2)

SOURCES = $(shell find src -name "*.c")
OBJECTS = $(SOURCES:src/%.c=obj/%.o)

TARGET = saida.out

.PHONY: all clear run

all: $(TARGET)

# The Linker Step
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(TARGET)

# The Compilation Step (compiles each .c into a .o)
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

release: CFLAGS = -std=c99 -Wall -Wextra -O3
release: clean $(TARGET)

perf: CFLAGS = -std=c99 -Wall -Wextra -fno-omit-frame-pointer -O2 -g
perf: all
	sudo perf record -g ./$(TARGET)
	sudo perf report --dsos=$(TARGET)

gprof: CFLAGS = -std=c99 -Wall -Wextra -pg -g
gprof: clean all
	./$(TARGET)
	gprof ./$(TARGET) gmon.out > analysis.txt
	@rm -f gmon.out

run: all
	./$(TARGET)
clean:
	rm -rf obj $(TARGET) analysis.txt gmon.out perf.data perf.data.old




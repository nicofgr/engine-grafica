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

build: 
	#clang -I./include/ -std=c99 -Wall ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -o  saida.out
	#clang -I./include/ -std=c99 -Wall -Werror -fsanitize=address ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -o  saida.out
	$(CC)  $(CFLAGS) -fsanitize=address $(SOURCES) $(LIBS) -o  $(TARGET)
	#Turn -fsanitize off for release build

perf:
	$(CC) $(CFLAGS) $(SOURCES) $(LIBS) -p -fno-omit-frame-pointer -o $(TARGET)
	sudo perf record -g ./saida.out
	sudo perf report --dsos=saida.out

gprof:
	$(CC) $(CFLAGS) $(SOURCES) $(LIBS) -pg -o gprof.out
	./gprof.out
	gprof ./gprof.out gmon.out > analysis.txt
	rm ./gprof.out
	rm gmon.out

release:
	$(CC) $(CFLAGS) $(SDIR) $(LIBS) -o  $(TARGET)

run: all
	./saida.out

clean:
	rm -rf obj $(TARGET)




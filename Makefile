CC = clang
CFLAGS = -std=c99 -Wall $(shell pkg-config --cflags freetype2)
LIBS = -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm $(shell pkg-config --libs freetype2)
TARGET = saida.out

build: 
	#clang -I./include/ -std=c99 -Wall ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -o  saida.out
	#clang -I./include/ -std=c99 -Wall -Werror -fsanitize=address ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -o  saida.out
	$(CC) -I./include/ $(CFLAGS) -fsanitize=address ./src/*.c $(LIBS) -o  $(TARGET)
	#Turn -fsanitize off for release build

perf:
	$(CC) -I./include/ $(CFLAGS) ./src/*.c $(LIBS) -p -fno-omit-frame-pointer -o $(TARGET)
	sudo perf record -g ./saida.out
	sudo perf report --dsos=saida.out

gprof:
	$(CC) -I./include/ $(CFLAGS) ./src/*.c $(LIBS) -pg -o gprof.out
	./gprof.out
	gprof ./gprof.out gmon.out > analysis.txt
	rm ./gprof.out
	rm gmon.out

run: build
	./saida.out

clean:
	rm *.out

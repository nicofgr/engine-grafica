build: 
	#clang -I./include/ -std=c99 -Wall ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -o  saida.out
	#clang -I./include/ -std=c99 -Wall -Werror -fsanitize=address ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -o  saida.out
	clang -I./include/ -std=c99 -Wall -fsanitize=address ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -o  saida.out
	#Turn -fsanitize off for release build

perf:
	clang -I./include/ -std=c99 -Wall ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -p -fno-omit-frame-pointer -o saida.out
	sudo perf record -g ./saida.out
	sudo perf report --dsos=saida.out

gprof:
	clang -I./include/ -std=c99 -Wall ./src/*.c -lSDL2 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -pg -o gprof.out
	./gprof.out
	gprof ./gprof.out gmon.out > analysis.txt
	rm ./gprof.out
	rm gmon.out

run: build
	./saida.out

clean:
	rm *.out

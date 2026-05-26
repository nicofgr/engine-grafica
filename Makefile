CC  = clang
CXX = clang++

CPPFLAGS = -I./include/ -I/usr/local/include -I./src/shared/ $(shell pkg-config --cflags freetype2)
#CFLAGS = -std=c99 -Wall -Wextra -Werror -fsanitize=address
CFLAGS = -std=c99 -Wall -Wextra -MMD -MP -fsanitize=address
CXXFLAGS = -std=c++11 -Wall -Wextra

SERVER_LIBS = -lpthread -lm -L/usr/local/lib -lGameNetworkingSockets -lstdc++
CLIENT_LIBS = -lpthread -lm -L/usr/local/lib -lGameNetworkingSockets -lstdc++ -lSDL2 -lGL -lX11 -lXrandr -lXi -ldl  $(shell pkg-config --libs freetype2)

SHARED_SRCS = $(wildcard src/shared/*.c)
SHARED_CXX_SRCS = $(wildcard src/shared/*.cpp)
SERVER_SRCS = $(wildcard src/server/*.c) $(wildcard src/server/*/*c)
CLIENT_SRCS = $(wildcard src/client/*.c) $(wildcard src/client/*/*c)

SHARED_OBJS = $(SHARED_SRCS:src/%.c=obj/%.o) $(SHARED_CXX_SRCS:src/%.cpp=obj/%.o)

SERVER_OBJS = $(SERVER_SRCS:src/%.c=obj/%.o) $(SHARED_OBJS)
CLIENT_OBJS = $(CLIENT_SRCS:src/%.c=obj/%.o) $(SHARED_OBJS)

ALL_OBJS = $(sort $(SERVER_OBJS) $(CLIENT_OBJS))
DEPS = $(ALL_OBJS:.o=.d)

TARGET_DIR = bin
SERVER_TARGET = $(TARGET_DIR)/server.out
CLIENT_TARGET = $(TARGET_DIR)/client.out

#SOURCES = $(shell find src -name "*.c")
#OBJECTS = $(SOURCES:src/%.c=obj/%.o)
#DEPS = $(OBJECTS:.o=.d)
#TARGET = saida.out

.PHONY: all clear run release perf gprof

all: $(SERVER_TARGET) $(CLIENT_TARGET)

# The Linker Step
#$(TARGET): $(OBJECTS)
#	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	@mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) $(SERVER_OBJS) $(SERVER_LIBS) -o $@
	@echo "✔ Server compiled successufully: $@"

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) $(CLIENT_LIBS) -o $@
	@echo "✔ Client compiled successufully: $@"

# The Compilation Step (compiles each .c into a .o)
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

release: CFLAGS = -std=c99 -Wall -Wextra -O3
release: clean $(TARGET)

# Profiling stuff ================================================

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




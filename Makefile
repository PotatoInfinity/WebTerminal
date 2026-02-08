CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
TARGET = server

# Source files
SRCS = main.cpp mongoose.c
OBJS = main.o mongoose.o

# Headers that trigger recompile
DEPS = mongoose.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c main.cpp

mongoose.o: mongoose.c $(DEPS)
	$(CC) -O2 -Wall -c mongoose.c

# Dependency download rules
mongoose.c:
	curl -O https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.c

mongoose.h:
	curl -O https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.h

clean:
	rm -f $(TARGET) $(OBJS)

dist-clean: clean
	rm -f mongoose.c mongoose.h server_stdout.log

run: $(TARGET)
	./$(TARGET)

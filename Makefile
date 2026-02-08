CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
TARGET = server

all: $(TARGET)

$(TARGET): main.o mongoose.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.o mongoose.o

main.o: main.cpp mongoose.h
	$(CXX) $(CXXFLAGS) -c main.cpp

mongoose.o: mongoose.c mongoose.h
	$(CC) -O2 -Wall -c mongoose.c

mongoose.c:
	curl -O https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.c

mongoose.h:
	curl -O https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.h

main.cpp: mongoose.h

clean:
	rm -f $(TARGET) mongoose.c mongoose.h server_stdout.log

run: $(TARGET)
	./$(TARGET)

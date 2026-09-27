CXX = g++
CXXFLAGS = -Wall -Wextra -Iinclude -std=c++17
TARGET = parkassist_service

SRC = src/main.cpp \
	src/ServiceCore.cpp \
	src/DataReceiver.cpp \
	src/DataTransmiter.cpp \
	src/Queue.cpp \
	src/Logging.cpp \
	src/UARTStreamReceiver.cpp \
	src/UARTStreamSender.cpp

OBJ = $(patsubst src/%.cpp,build/%.o,$(SRC))

$(TARGET):$(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

build/%.o: src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(TARGET)
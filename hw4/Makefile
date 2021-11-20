CC = g++
TARGET = render
OBJECTS = main.o model.o material.o light.o

CXXFLAGS = -std=c++17 -O3 -Wall -g
LDFLAGS = -lm -ldl -lglut -lGL -lGLU

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean run

clean:
	rm $(TARGET) $(OBJECTS)

run: $(TARGET)
	./$^

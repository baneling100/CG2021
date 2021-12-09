CC = g++
TARGET = render
OBJECTS = main.o material.o light.o model.o export.o ray_trace.o utils.o

CXXFLAGS = -std=c++17 -O3 -Wall -g
LDFLAGS = -lm -ldl -lglut -lGL -lGLU -pthread

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean run

clean:
	rm $(TARGET) $(OBJECTS)

run: $(TARGET)
	./$^ 4

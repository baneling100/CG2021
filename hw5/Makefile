CC = g++
TARGET = render
OBJECTS = main.o material.o light.o model.o ray_tracing.o

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

CXXFLAGS += -g -Wall
CC := g++
OUTPUT := login
SOURCES  = $(wildcard *.cpp)

all: $(SOURCES) pch.h.gch
	$(CC) $(CXXFLAGS) $(SOURCES) -o $(OUTPUT)
	
pch.h.gch:
	gcc pch.h

.PHONY = all
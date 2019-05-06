all: sample2D

sample2D: h.cpp glad.c
	g++ -o shoot h.cpp glad.c -lGL -lglfw -ldl

clean:
	rm shoot

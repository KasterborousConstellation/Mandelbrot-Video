.PHONY: all main mandellib
COMPILER = gcc 
FLAGS = -Wall -std=c99 -pedantic

FILE_PATH = /media/paul/Kingston/
FILENAME = mandelbrot.mp4
OPTFLAGS = -O3 -march=native
SECONDS = 10
FPS = 30
ACCURACY = 300
SPEED = 3
SCALE = 1 
all: main

main: mandellib.o m_theme.o m_encoder.o
	$(COMPILER) -o main mandellib.o m_theme.o m_encoder.o main.c -lavcodec -lavformat -lavutil -lswscale $(FLAGS) $(OPTFLAGS)
	./main $(SECONDS) $(FPS) $(ACCURACY) $(FILE_PATH) $(FILENAME) $(SPEED) $(SCALE)
mandellib: mandellib.c mandellib.o
	$(COMPILER) -c mandellib.c $(OPTFLAGS)
m_theme: m_theme.c m_theme.o
	$(COMPILER) -c m_theme.c
m_encoder: m_encoder.c m_encoder.o
	$(COMPILER) -c m_encoder.c $(OPTFLAGS)
play: 
	ffplay $(FILE_PATH)$(FILENAME)
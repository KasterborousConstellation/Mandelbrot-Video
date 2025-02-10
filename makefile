.PHONY: all main mandellib
COMPILER = gcc 
FLAGS = -Wall -Wextra -std=c99 -pedantic

FILE_PATH = /media/paul/Kingston/
FILENAME = mandelbrot.mp4
OPTFLAGS = -Ofast -march=native
SECONDS = 10
FPS = 60
ACCURACY = 200
SPEED = 3

all: main

main: mandellib.o m_theme.o
	$(COMPILER) -o main mandellib.o m_theme.o main.c -lavcodec -lavformat -lavutil -lswscale $(OPTFLAGS)
	./main $(SECONDS) $(FPS) $(ACCURACY) $(FILE_PATH) $(FILENAME) $(SPEED)
mandellib: mandellib.c mandellib.o
	$(COMPILER) -c mandellib.c $(OPTFLAGS)
m_theme: m_theme.c m_theme.o
	$(COMPILER) -c m_theme.c
play: 
	ffplay $(FILE_PATH)$(FILENAME)
.PHONY: all main mandellib
COMPILER = gcc 
FLAGS = -Wall -std=c99 -pedantic

FILE_PATH = /home/paul/mandel
FILENAME = mandelbrot.mp4
OPTFLAGS = -O3 -march=native
SECONDS = 60
FPS = 144
ACCURACY = 10000
SPEED = 3
SCALE = 1
all: main

main: m_theme.o m_encoder.o fileUtils.o m_compute.o
	$(COMPILER) -o main m_theme.o m_encoder.o m_compute.o fileUtils.o main.c -lavcodec -lavformat -lavutil -lswscale -lOpenCL -lm $(FLAGS) $(OPTFLAGS)
	./main $(SECONDS) $(FPS) $(ACCURACY) $(FILE_PATH) $(FILENAME) $(SPEED) $(SCALE)
m_theme: m_theme.c m_theme.o
	$(COMPILER) -c m_theme.c
m_encoder: m_encoder.c m_encoder.o
	$(COMPILER) -c m_encoder.c  $(OPTFLAGS)
fileUtils: fileUtils.c fileUtils.o
	$(COMPILER) -c fileUtils.c $(OPTFLAGS)
m_compute: m_compute.c m_compute.o
	$(COMPILER) -c m_compute.c $(OPTFLAGS)
play: 
	ffplay $(FILE_PATH)$(FILENAME)
linecount:
	wc -l *.c *.h makefile *.cl

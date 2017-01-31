LIBS = -lglut -lGL -lGLU -pedantic -Dcimg_use_vt100 -Dcimg_display=1 -lm -lX11 -lpthread -ljpeg -ltiff
COMP = g++
CFLAGS = -Wall -std=c++11

all: MazeViewer

MazeViewer: MazeViewer.o
	$(COMP) $(CFLAGS) -o MazeViewer MazeViewer.o $(LIBS)

# The default way to convert .c files into .o files.
%.o: %.cpp
	$(COMP) $(CFLAGS) -c -o $@ $<

clean:
	rm *.o MazeViewer

LIBS = -lglut -lGL -lGLU -pedantic -Dcimg_use_vt100 -Dcimg_display=1 -lm -lX11 -lpthread -ljpeg -ltiff
COMP = g++ -std=c++11
CFLAGS = -Wall

all: MazeViewer

MazeViewer: MazeViewer.o
	$(COMP) $(CFLAGS) -o MazeViewer MazeViewer.o $(LIBS)

# The default way to convert .c files into .o files.
.c.o:		; $(CC) -c $(CFLAGS) $<


clean:
	rm *.o MazeViewer

LIBS = -lglut -lGL -lGLU
COMP = g++
CFLAGS = -Wall

all: MazeViewer

MazeViewer: MazeViewer.o
	$(COMP) $(CFLAGS) -o MazeViewer MazeViewer.o $(LIBS)

# The default way to convert .c files into .o files.
.c.o:		; $(CC) -c $(CFLAGS) $<


clean:
	rm *.o MazeViewer

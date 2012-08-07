OBJS= prj4.o tmatrix.o tray.o tvector.o

CFLAGS=-g
LD=g++ -g

all: $(OBJS)
	$(LD) $(OBJS) -o prj4 -L/usr/X11R6/lib -lglut -lGL -lGLU -lm

clean:
	@echo Cleaning up...
	@rm $(OBJS) prj4
	@echo Done.

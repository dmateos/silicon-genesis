objects = \
	main.o \
	sdlio.o \
	cellvmcb.o \
	cellvm.o \
	cellconf.o \

flags = -lglfw3 -lglew -lassimp -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

all: silicon-genesis

silicon-genesis: $(objects)
	$(CC) $(flags) -o $@ $(objects)

main.o: main.c
	$(CC) -c -o $@ main.c

stlio.o: sdlio.c
	$(CC) -c -o $@ stlio.c

cellvmcb.o: cellvmcb.c
	$(CC) -c -o $@ cellvmcb.c

cellvm.o: cellvm.c
	$(CC) -c -o $@ cellvm.c

cellconf.o: cellconf.c
	$(CC) -c -o $@ cellconf.c

clean:
	rm -rf *.o coneway

objects = \
	src/main.o \
	src/sdlio.o \
	src/cellvmcb.o \
	src/cellvm.o \
	src/cellconf.o \

flags = -lglfw3 -lglew -lassimp -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

all: silicon-genesis

silicon-genesis: $(objects)
	$(CC) $(flags) -o $@ $(objects)

src/main.o: src/main.c
	$(CC) -c -o $@ src/main.c

src/stlio.o: src/sdlio.c
	$(CC) -c -o $@ src/stlio.c

src/cellvmcb.o: src/cellvmcb.c
	$(CC) -c -o $@ src/cellvmcb.c

src/cellvm.o: src/cellvm.c
	$(CC) -c -o $@ src/cellvm.c

src/cellconf.o: src/cellconf.c
	$(CC) -c -o $@ src/cellconf.c

clean:
	rm -rf src/*.o silicon-genesis

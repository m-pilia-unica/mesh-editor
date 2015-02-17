all:
	if [ ! -e ./bin ]; then; mkdir ./bin; fi
	gcc -o ./bin/main main.c frontend.c backend.c \
		-lglut -lGL -lGLU -lm 

debug:
	if [ ! -e ./bin ]; then; mkdir ./bin; fi
	gcc -o ./bin/Debug/main main.c frontend.c backend.c \
		-lglut -lGL -lGLU -lm \
		-D __DEBUG__

doc:
	doxygen Doxyfile

clean:
	rm -rf ./doc
	rm -f ./bin/*

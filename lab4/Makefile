all: run

run: compile
	mpirun -n 8 hipercub

compile: difuzie_hipercub.c
	mpicc difuzie_hipercub.c -o hipercub

clean:
	rm -rf hipercub

all: run

NODES=4
run: compile
	@echo "Map Reduce Implementation"
	mkdir -p data-interim
	mpirun -n $(NODES) map_reduce
	@echo ""

debug: compile
	mpirun -n $(NODES) map_reduce


compile: *.c
	mpicc -g *.c -o map_reduce 

clean:
	rm -rf map_reduce
	rm -rf data-interim
	rm -rf data-output/*
	mkdir -p data-interim

all: run

NODES=8
run: compile
	@echo "Map Reduce Implementation"
	mkdir -p data-interim/reduce
	mpirun -n $(NODES) map_reduce
	@echo ""

debug: compile
	mpirun -n $(NODES) map_reduce


compile: *.c
	mpicc *.c -o map_reduce 

clean:
	rm -rf map_reduce
	rm -rf data-interim
	mkdir -p data-interim/reduce

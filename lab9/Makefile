all: run

NODES=16
run: compile
	@echo "Merge sort of $(NODES) elements"
	mpirun -n $(NODES) cannon
	@echo ""

debug: compile
	mpirun -n $(NODES) cannon


compile: cannon.c
	mpicc cannon.c -o cannon 

clean:
	rm -rf cannon

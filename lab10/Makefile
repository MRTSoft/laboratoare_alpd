all: run

NODES=8
run: compile
	@echo "Rezolvare ecuatii "
	mpirun -n $(NODES) equation
	@echo ""

debug: compile
	mpirun -n $(NODES) equation


compile: equation.c
	mpicc equation.c -o equation 

clean:
	rm -rf equation

all: run

NODES=4
run: compile
	@echo "Alegere lider din $(NODES) noduri"
	mpirun -n $(NODES) lider
	@echo ""

debug: compile
	mpirun -n $(NODES) lider

compile: lider.c
	mpicc lider.c -o lider 

clean:
	rm -rf lider

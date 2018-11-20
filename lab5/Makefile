all: run

run: compile
	@echo "Dispersion from root to all nodes"
	mpirun -n 9 dispersion
	@echo ""
	@echo "====================="
	@echo ""
	@echo "Gather of messages to root"
	mpirun -n 9 gather

compile: graph_dispersion.c graph_gather.c
	mpicc graph_dispersion.c -o dispersion
	mpicc graph_gather.c -o gather

clean:
	rm -rf dispersion gather

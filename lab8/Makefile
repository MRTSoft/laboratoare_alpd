all: run

NODES=8
run: compile
	@echo "Merge sort of $(NODES) elements"
	mpirun -n $(NODES) merge_sort | grep ^FINAL | sort
	@echo ""

debug: compile
	mpirun -n $(NODES) merge_sort


compile: merge_sort.c
	mpicc merge_sort.c -o merge_sort 

clean:
	rm -rf merge_sort

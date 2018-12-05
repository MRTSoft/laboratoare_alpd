all: run

run: compile
	@echo "Merge sort of 16 elements"
	mpirun -n 8 merge_sort
	@echo ""

compile: merge_sort.c
	mpicc merge_sort.c -o merge_sort 

clean:
	rm -rf merge_sort

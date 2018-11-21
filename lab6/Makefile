all: compile
	@echo "= = = =     MP     = = = ="
	@echo "- - - - Algoritm 2 - - - -"
	./compress
	@echo ""
	@echo "- - - - Algoritm 3 - - - -"
	./prefix

compile: compress prefix
	@echo "- - Compilation DONE - -"


compress: compress.c
	gcc -g compress.c -fopenmp -o compress

prefix: prefix.c
	gcc -g prefix.c -fopenmp -o prefix

clean:
	rm -f prefix compress

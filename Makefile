all: adj_matrix persist_example

adj_matrix:
	$(MAKE) -C test/ adj_matrix
	cp test/adj_matrix ./build/
	rm test/adj_matrix

persistent_example:
	$(MAKE) -C test/ persistent_example
	cp test/persistent_example ./build/
	rm test/persistent_example

clean:
	rm -f ./build/*
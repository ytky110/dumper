bin/dumper: src/dumper.cc
	mkdir -p bin
	g++ $^ -o $@

.PHONY: clean

clean:
	rm -f bin/*

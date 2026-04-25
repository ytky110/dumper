bin/dumper: src/dumper.cc
	g++ $^ -o $@

.PHONY: clean

clean:
	rm -f bin/*

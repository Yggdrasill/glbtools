all:
	+${MAKE} -C src/
	mv src/glbextract .

clean:
	rm glbextract

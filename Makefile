all:
	+${MAKE} -C src/
	mv src/glbextract .
	mv src/glbcreate .

clean:
	rm glbextract
	rm glbcreate

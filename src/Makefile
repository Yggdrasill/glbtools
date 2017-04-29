all:
	${CC} -c fat.c glb.c
	${CC} -c crypt.c encrypt.c decrypt.c
	${CC} -c glbextract.c glbcreate.c

	${CC} glbextract.o fat.o crypt.o decrypt.o glb.o -o glbextract
	${CC} glbcreate.o fat.o crypt.o encrypt.o glb.o -o glbcreate

	rm *.o

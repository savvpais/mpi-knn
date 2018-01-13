all: blockingKnnMpi nonblockingKnnMpi serialKnn

blockingKnnMpi:blockingKnnMpi.c
	mpicc blockingKnnMpi.c -O3 -o blck -lm

nonblockingKnnMpi:nonblockingKnnMpi.c
	mpicc nonblockingKnnMpi.c -O3 -o nblck -lm

serialKnn:serialKnn.c
	gcc serialKnn.c -o serial -O3 -lm

clear:
	rm serial blck nblck

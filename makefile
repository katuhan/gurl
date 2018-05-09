build:  
	cd lib; make 
	cd src; make

rebuild:clean build

clean: 
	cd lib; make clean
	cd src; make clean

.PHONY: build rebuild clean

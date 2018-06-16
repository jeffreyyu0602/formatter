rename: main.o
	gcc -o rename main.o

main.o: main.c
	gcc -c main.c

new:
	make clean
	make rename

init:
	make rename
	echo "alias rename=`pwd`/rename" >> ~/.bash_profile
	source ~/.bash_profile

clean:
	rm -rf *.o rename

rename:
	gcc -g -std=c99 main.c

new:
	make clean
	make rename

clean:
	rm -rf a.out a.out.dSYM

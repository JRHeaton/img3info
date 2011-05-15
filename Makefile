all: clean img3info

img3info:
	cc -O3 -Wall -Werror main.c -o img3info
	
clean:
	rm -f img3info
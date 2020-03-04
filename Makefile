build:
	gcc -g TEMA1.c -o atm -Wall -Wextra
run: build
	./atm
clean:
	rm -rf atm

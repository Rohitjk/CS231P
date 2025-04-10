all:
	gcc -o simulator.bin -lm main.c simulator.c
clean:
	rm -f simulator.bin

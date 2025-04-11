all:
	gcc -o simulator.bin -lm main.c simulator.c
	./run_all.sh
	python plot_simulation.py
clean:
	rm -f simulator.bin *.csv *.png

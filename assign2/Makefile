uni:
		gcc uni_threads_simulator.c -o a.out -pthread  -fsanitize=address
		./a.out
	
san:
		gcc uni_threads_simulator.c -pthread -g -fsanitize=address -fsanitize=leak -o a.out
		./a.out

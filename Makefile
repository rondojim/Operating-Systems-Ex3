all: chef saladmaker

chef: chef.c
	 gcc -o chef chef.c -pthread

saladmaker: saladmaker.c
	 gcc -o saladmaker saladmaker.c -pthread


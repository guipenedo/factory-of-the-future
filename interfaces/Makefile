test: peripherals_network.o bme280.o test.o
	gcc -Wall -o test test.o peripherals_network.o bme280.o -lwiringPi -std=gnu99
test.o: test.c bme280.h peripherals_network.h
	gcc -Wall -c test.c -lwiringPi -std=gnu99
peripherals_network.o: peripherals_network.h
	gcc -Wall -c peripherals_network.c -lwiringPi -std=gnu99
bme280.o: bme280.c bme280.h bme280_defs.h
	gcc -Wall -c bme280.c -std=gnu99
clean:
	rm peripherals_network.o bme280.o test.o

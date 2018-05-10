.DEFAULT_GOAL := pgm1
.PHONY: clean run
CFLAGS=-Wall -g -lpthread -lrt
clean:
	rm -f *.o out/*
run:
	./pgm1 out/producer_time.txt out/producer_items.txt out/consumer_time.txt out/consumer_items.txt

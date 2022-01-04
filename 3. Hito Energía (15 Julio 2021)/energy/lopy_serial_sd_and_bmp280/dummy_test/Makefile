CC=g++
DEPS=~/Arduino/libraries/SensorPayload/SensorPayload.cpp
SOURCE=dummy_test.cpp
INCLUDES=~/Arduino/libraries/SensorPayload/


all: dummy

dummy:
	$(CC) $(DEPS) $(SOURCE) -I $(INCLUDES) -o dummy

.PHONY: clean run

clean:
	rm -rf dummy bytes.txt

run:
	./dummy 2> bytes.txt
	cat bytes.txt| awk '{print $$2}'| sed s/0x// | xargs

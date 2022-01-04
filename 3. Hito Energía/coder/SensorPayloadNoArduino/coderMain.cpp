#include "SensorPayload.h"
#include <iostream>
using namespace std;

SensorPayload sp(49);
block_type dataBlock;
float base_val = 23.1;
uint32_t base_ts = 1609300000;
uint8_t sensor = 1;
int encode_res = 0;
int pos = 0;

void baseFill() {
	cout << "Base Encoding";
	dataBlock.count = 0;
	for(int i=0; i<7; ++i) {
		dataBlock.data[i].sensor = sensor;
		dataBlock.data[i].val = base_val + i*0.5;
		dataBlock.data[i].ts = base_ts + i*60;
		++dataBlock.count;
	}
    pos = 0;
	while(encode_res = sp.encodeReadings(&dataBlock, pos, 0)) {
		pos += encode_res;
	}
    cout << pos;
    cout << "Base terminado";
}

void repeatedFill() {
    cout << "Repeated Encoding";
	dataBlock.count = 0;
    for(int i=0; i<7; ++i) {
		dataBlock.data[i].sensor = sensor;
		dataBlock.data[i].val = base_val;
		dataBlock.data[i].ts = base_ts + i*60;
		++dataBlock.count;
    }
    for(int i=7; i<12; ++i) {
		dataBlock.data[i].sensor = sensor;
		dataBlock.data[i].val = base_ts + i*0.1;
		dataBlock.data[i].ts = sensor + i*60;
		++dataBlock.count;
    }
    pos = 0;
    while(encode_res = sp.encodeReadings(&dataBlock, pos, 1)) {
		pos += encode_res;
    }
    cout << pos;
    cout << "Repeated terminado";
}

void diferentialFill() {
    cout << "Diferential Encoding";
	dataBlock.count = 0;
    for(int i=0; i<6; ++i) {
		dataBlock.data[i].sensor = sensor;
		dataBlock.data[i].val = base_val + i*0.5;
		dataBlock.data[i].ts = base_ts + i*60;
		++dataBlock.count;
    }
    for(int i=1; i<6; ++i) {
		dataBlock.data[i+5].sensor = sensor;
		dataBlock.data[i+5].val = base_val - i*0.5;
		dataBlock.data[i+5].ts = base_ts + (i+5)*60;
		++dataBlock.count;
    }
    dataBlock.data[11].sensor = sensor;
    dataBlock.data[11].val = base_val + 15;
    dataBlock.data[11].ts = base_ts + (11)*60;
    ++dataBlock.count;
    pos = 0;
    while(encode_res = sp.encodeReadings(&dataBlock, pos, 2)) {
		pos += encode_res;
    }
    cout << pos;
    cout << "Diferential terminado";
}

int main() {
    cout << sp.availableSize();
    repeatedFill();
}
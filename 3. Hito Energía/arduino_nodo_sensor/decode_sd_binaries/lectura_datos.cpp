#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>

namespace fs = std::filesystem;
using namespace std;

// Structs ---------------------------------------------
#pragma pack(push, 1)
typedef struct sensor_reading {
    float val;
    uint32_t ts;
    uint8_t sensor;
} reading_type;
#pragma pack(pop)

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 2)/sizeof(reading_type);
//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 2 - DATA_DIM*sizeof(reading_type);

#pragma pack(push, 1)
typedef struct data_block { // Greiman
    reading_type data[DATA_DIM]; // Readings
    uint16_t count; // Number of readings on block
    uint8_t fill[FILL_DIM]; // Bytes to fill 512 bytes
} block_type;
#pragma pack(pop)
// -----------------------------------------------------

void convertBinFile(fs::path pt, string writeto) {
    ifstream rf(pt, ifstream::binary | ifstream::in);
    if(!rf) {
        cout << "ERROR: no se pudo trabajar con el archivo\n";
    }
    string filename = pt.filename();
    string fnamenoext = filename.substr(0, filename.find_first_of('.'));
    rf.seekg(0, ios::end);
    int len = rf.tellg();
    rf.seekg(0, ios::beg);
    int nblocks = len/512;
    // Archivo de escritura
    string newfilepath = writeto + fnamenoext + ".csv";
    ofstream wf(newfilepath, ofstream::out | ofstream::trunc);
    wf << "sensor_id,timestamp,value\n";
    while(nblocks--) {
        uint16_t count;
        rf.read((char *)&count, 2);
        uint8_t sensor;
        uint32_t ts;
        float val;
        for(int i=0; i<count; ++i) {
            rf.read((char *)&val, 4);
            rf.read((char *)&ts, 4);
            rf.read((char *)&sensor, 1);
            wf << unsigned(sensor) << ','
               << ts << ','
               << val << '\n';
        }
        uint32_t nodatasize = 512 - 2 - count * 9;
        uint32_t tellg = rf.tellg();
        rf.seekg(tellg + nodatasize);
    }
    rf.close();
    wf.close();
}

int main() {
    cout << "Ingrese el nombre de la carpeta con los archivos binarios a leer: ";
    string datadir;
    cin >> datadir;
    fs::path cwd = fs::current_path();
    fs::path datapath = cwd / datadir;
    if(fs::exists(datapath)) {
        fs::path csvpath = cwd / (datadir + "_csv/");
        if(!fs::exists(csvpath)) {
            fs::create_directory(csvpath);
        }
        for(auto& p: fs::directory_iterator(datapath)) {
            convertBinFile(p.path(), csvpath);
        }
        cout << "Archivos csv generados en la ruta " << csvpath << '\n';

    }
    else {
        cout << "ERROR: la carpeta " << datadir << " no existe. ¿Ingresó bien el nombre de la carpeta? \n";
    }
    return 0;    
}
#include <queue>
#include <algorithm>
#include "dem.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <list>
#include <unordered_map>
#include <iostream>
#include <chrono>
#include <string>
#include "gdal.h"

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::getline;
using std::fstream;
using std::ifstream;
using std::priority_queue;
using std::binary_function;

int FillDEM_Wang(const char* inputFile, const char* outputFilledPath);

int main() {
    GDALAllRegister();

    std::string filename = "D:\\GIS_Data\\cook_dem_3m_m.tif";
    std::string outputFilename = "D:\\GIS_Data\\pp1.tif";

    int m = 3;

    if (m == 1) {
        HP2_fillDEM(filename.c_str(), outputFilename.c_str());
    }
    else if (m == 2) {
        fillDEM(filename.c_str(), outputFilename.c_str());
    }
    else if(m == 3){
        FillDEM_Wang2(filename.c_str(), outputFilename.c_str());
    }
    else if(m == 4){
        Hash_Wang(filename.c_str(), outputFilename.c_str());
    }
    else {
        AVL_W(filename.c_str(), outputFilename.c_str());
    }
    
   
    return 0;
}
#ifndef UTILS_HEAD_H
#define UTILS_HEAD_H

#include "gdal_priv.h"
#include <queue>
#include <algorithm>
#include "dem.h"
#include <iostream>

void calculateStatistics(const CDEM& dem, double* min, double* max, double* mean, double* stdDev);

extern int	ix[8];
extern int	iy[8];
inline int Get_rowTo(int dir, int row)
{
	return(row + ix[dir]);
}
inline int Get_colTo(int dir, int col) {
	return(col + iy[dir]);
}
void setNoData(unsigned char* data, int length, unsigned char noDataValue);
void setNoData(float* data, int length, float noDataValue);
void setFlag(int index, unsigned char* flagArray);
bool isProcessed(int index, const unsigned char* flagArray);
bool  CreateGeoTIFF(const char* path, int height, int width, void* pData, GDALDataType type, double* geoTransformArray6Eles,
	double* min, double* max, double* mean, double* stdDev, double nodatavalue);
bool readTIFF(const char* path, GDALDataType type, CDEM& dem, double* geoTransformArray6Eles);
CDEM* diff(CDEM& demA, CDEM& demB);
void CreateDiffImage(const char* demA, const char* demB, char* resultPath, GDALDataType type, double nodatavalue);
extern const unsigned char value[8];
class Flag
{
public:
	size_t width, height;
	std::vector<unsigned char> flagArray;
public:
	~Flag()
	{
		Free();
	}
	bool Init(size_t w, size_t h) {
		if (w == 0 || h == 0) {
			std::cerr << "Invalid dimensions: width or height is zero." << std::endl;
			return false;
		}

		// 检查 width * height 是否溢出
		if (h > (std::numeric_limits<size_t>::max() - 7) / w) {
			std::cerr << "Overflow detected in width * height calculation." << std::endl;
			return false;
		}

		size_t total_bits = w * h;
		size_t array_size = (total_bits + 7) / 8;

		try {
			flagArray.resize(array_size, 0);
			width = w;
			height = h;
			return true;
		}
		catch (const std::bad_alloc& e) {
			std::cerr << "Memory allocation failed: " << e.what() << std::endl;
			return false;
		}
		catch (...) {
			std::cerr << "Unknown error during memory allocation." << std::endl;
			return false;
		}
	}
	void Free() {
		flagArray.clear();    // 清空内容（可选）
		flagArray.shrink_to_fit(); // 释放内存（可选）
	}
	void SetFlag(size_t row, size_t col) {
		if (row >= height || col >= width) return;
		size_t index = row * width + col;
		flagArray[index / 8] |= value[index % 8];
	}
	void SetFlags(size_t row, size_t col, Flag& flag)
	{
		size_t index = row * width + col;
		size_t bIndex = index / 8;
		size_t bShift = index % 8;
		flagArray[bIndex] |= value[bShift];
		flag.flagArray[bIndex] |= value[bShift];
	}
	int IsProcessed(int row, int col)
	{
		//if the cell is outside the DEM, is is regared as processed
		if (row < 0 || row >= height || col < 0 || col >= width) return true;
		size_t index = row * width + col;
		return flagArray[index / 8] & value[index % 8];
	}
	int IsProcessedDirect(int row, int col) {
		if (row < 0 || col < 0 || row >= height || col >= width) return true;
		size_t index = row * width + col;
		return flagArray[index / 8] & value[index % 8];
	}
};

#endif



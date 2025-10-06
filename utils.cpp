#include "utils.h"
#include "gdal_priv.h"
#include "dem.h"
#include <string>

//create a new GeoTIFF file
//定义一个函数，用于创建一个GeoTIFF文件。
//参数包括文件路径、图像高度和宽度、数据指针、数据类型、地理变换数组、
//统计信息（最小值、最大值、均值、标准差）和无数据值。
bool CreateGeoTIFF(const char* path, int height, int width, void* pData, GDALDataType type, double* geoTransformArray6Eles,
	double* min, double* max, double* mean, double* stdDev, double nodatavalue)
{
	// 注册所有GDAL驱动
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	// 获取GTiff驱动
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (poDriver == nullptr) {
		printf("Failed to get GTiff driver.\n");
		return false;
	}

	// 创建数据集
	char** papszOptions = NULL;
	GDALDataset* poDataset = poDriver->Create(path, width, height, 1, type, papszOptions);
	if (poDataset == nullptr) {
		printf("Failed to create dataset.\n");
		return false;
	}

	// 设置地理变换参数
	if (geoTransformArray6Eles != NULL) {
		poDataset->SetGeoTransform(geoTransformArray6Eles);
	}

	// 获取数据集的第一个波段
	GDALRasterBand* poBand = poDataset->GetRasterBand(1);
	if (poBand == nullptr) {
		printf("Failed to get raster band.\n");
		GDALClose((GDALDatasetH)poDataset);
		return false;
	}

	// 设置波段的无数据值
	poBand->SetNoDataValue(nodatavalue);

	// 设置统计信息
	if (min != NULL && max != NULL && mean != NULL && stdDev != NULL) {
		poBand->SetStatistics(*min, *max, *mean, *stdDev);
	}

	// 将数据写入波段
	CPLErr err = poBand->RasterIO(GF_Write, 0, 0, width, height, pData, width, height, type, 0, 0);
	if (err != CE_None) {
		printf("Failed to write raster data.\n");
		GDALClose((GDALDatasetH)poDataset);
		return false;
	}

	// 关闭数据集
	GDALClose((GDALDatasetH)poDataset);

	return true;
}
//read a DEM GeoTIFF file 
//定义一个函数，用于读取GeoTIFF文件。参数包括文件路径、数据类型、DEM对象引用和地理变换数组
bool readTIFF(const char* path, GDALDataType type, CDEM& dem, double* geoTransformArray6Eles)
{
	//声明GDAL数据集指针，注册驱动，设置配置选项，打开GeoTIFF文件。
	GDALDataset* poDataset;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	poDataset = (GDALDataset*)GDALOpen(path, GA_ReadOnly);
	//如果打开失败，则打印错误信息并返回失败。
	if (poDataset == NULL)
	{
		printf("Failed to read the GeoTIFF file\n");
		return false;
	}

	GDALRasterBand* poBand;
	poBand = poDataset->GetRasterBand(1);
	//检查数据类型是否匹配。
	GDALDataType dataType = poBand->GetRasterDataType();
	if (dataType != type)
	{
		return false;
	}
	//检查地理变换数组是否为空。
	if (geoTransformArray6Eles == NULL)
	{
		printf("Transformation parameters can not be NULL\n");
		return false;
	}

	//初始化地理变换数组，并获取地理变换参数。
	memset(geoTransformArray6Eles, 0, 6);
	poDataset->GetGeoTransform(geoTransformArray6Eles);

	const size_t width = static_cast<size_t>(poDataset->GetRasterXSize());
	const size_t height = static_cast<size_t>(poDataset->GetRasterYSize());

	//设置DEM对象的宽度和高度。
	dem.SetWidth(width);
	dem.SetHeight(height);

	//分配内存给DEM对象。
	if (!dem.Allocate()) return false;

	//从波段读取数据到DEM对象。
	poBand->RasterIO(GF_Read, 0, 0, width, height,
		(void*)dem.getDEMdata(), width, height, dataType, 0, 0);

	//关闭数据集并返回成功标志。
	GDALClose((GDALDatasetH)poDataset);
	return true;
}
/*
*	neighbor index
*	5  6  7
*	4     0
*	3  2  1
*/
//定义8邻域索引数组，用于图像处理中的邻域操作。
int	ix[8] = { 0, 1, 1, 1, 0,-1,-1,-1 };
int	iy[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };

//为无符号字符类型数据设置无数据值
void setNoData(unsigned char* data, int length, unsigned char noDataValue)
{
	if (data == NULL || length == 0)
	{
		return;
	}

	for (int i = 0; i < length; i++)
	{
		data[i] = noDataValue;
	}
}

//为浮点类型数据设置无数据值。
void setNoData(float* data, int length, float noDataValue)
{
	for (int i = 0; i < length; i++)
	{
		data[i] = noDataValue;
	}
}

//定义一个无符号字符数组，用途未在代码中直接体现，可能用于某种权重或掩膜操作。
const unsigned char value[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

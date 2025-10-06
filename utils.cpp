#include "utils.h"
#include "gdal_priv.h"
#include "dem.h"
#include <string>

//create a new GeoTIFF file
//����һ�����������ڴ���һ��GeoTIFF�ļ���
//���������ļ�·����ͼ��߶ȺͿ�ȡ�����ָ�롢�������͡�����任���顢
//ͳ����Ϣ����Сֵ�����ֵ����ֵ����׼���������ֵ��
bool CreateGeoTIFF(const char* path, int height, int width, void* pData, GDALDataType type, double* geoTransformArray6Eles,
	double* min, double* max, double* mean, double* stdDev, double nodatavalue)
{
	// ע������GDAL����
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	// ��ȡGTiff����
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (poDriver == nullptr) {
		printf("Failed to get GTiff driver.\n");
		return false;
	}

	// �������ݼ�
	char** papszOptions = NULL;
	GDALDataset* poDataset = poDriver->Create(path, width, height, 1, type, papszOptions);
	if (poDataset == nullptr) {
		printf("Failed to create dataset.\n");
		return false;
	}

	// ���õ���任����
	if (geoTransformArray6Eles != NULL) {
		poDataset->SetGeoTransform(geoTransformArray6Eles);
	}

	// ��ȡ���ݼ��ĵ�һ������
	GDALRasterBand* poBand = poDataset->GetRasterBand(1);
	if (poBand == nullptr) {
		printf("Failed to get raster band.\n");
		GDALClose((GDALDatasetH)poDataset);
		return false;
	}

	// ���ò��ε�������ֵ
	poBand->SetNoDataValue(nodatavalue);

	// ����ͳ����Ϣ
	if (min != NULL && max != NULL && mean != NULL && stdDev != NULL) {
		poBand->SetStatistics(*min, *max, *mean, *stdDev);
	}

	// ������д�벨��
	CPLErr err = poBand->RasterIO(GF_Write, 0, 0, width, height, pData, width, height, type, 0, 0);
	if (err != CE_None) {
		printf("Failed to write raster data.\n");
		GDALClose((GDALDatasetH)poDataset);
		return false;
	}

	// �ر����ݼ�
	GDALClose((GDALDatasetH)poDataset);

	return true;
}
//read a DEM GeoTIFF file 
//����һ�����������ڶ�ȡGeoTIFF�ļ������������ļ�·�����������͡�DEM�������ú͵���任����
bool readTIFF(const char* path, GDALDataType type, CDEM& dem, double* geoTransformArray6Eles)
{
	//����GDAL���ݼ�ָ�룬ע����������������ѡ���GeoTIFF�ļ���
	GDALDataset* poDataset;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	poDataset = (GDALDataset*)GDALOpen(path, GA_ReadOnly);
	//�����ʧ�ܣ����ӡ������Ϣ������ʧ�ܡ�
	if (poDataset == NULL)
	{
		printf("Failed to read the GeoTIFF file\n");
		return false;
	}

	GDALRasterBand* poBand;
	poBand = poDataset->GetRasterBand(1);
	//������������Ƿ�ƥ�䡣
	GDALDataType dataType = poBand->GetRasterDataType();
	if (dataType != type)
	{
		return false;
	}
	//������任�����Ƿ�Ϊ�ա�
	if (geoTransformArray6Eles == NULL)
	{
		printf("Transformation parameters can not be NULL\n");
		return false;
	}

	//��ʼ������任���飬����ȡ����任������
	memset(geoTransformArray6Eles, 0, 6);
	poDataset->GetGeoTransform(geoTransformArray6Eles);

	const size_t width = static_cast<size_t>(poDataset->GetRasterXSize());
	const size_t height = static_cast<size_t>(poDataset->GetRasterYSize());

	//����DEM����Ŀ�Ⱥ͸߶ȡ�
	dem.SetWidth(width);
	dem.SetHeight(height);

	//�����ڴ��DEM����
	if (!dem.Allocate()) return false;

	//�Ӳ��ζ�ȡ���ݵ�DEM����
	poBand->RasterIO(GF_Read, 0, 0, width, height,
		(void*)dem.getDEMdata(), width, height, dataType, 0, 0);

	//�ر����ݼ������سɹ���־��
	GDALClose((GDALDatasetH)poDataset);
	return true;
}
/*
*	neighbor index
*	5  6  7
*	4     0
*	3  2  1
*/
//����8�����������飬����ͼ�����е����������
int	ix[8] = { 0, 1, 1, 1, 0,-1,-1,-1 };
int	iy[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };

//Ϊ�޷����ַ�������������������ֵ
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

//Ϊ����������������������ֵ��
void setNoData(float* data, int length, float noDataValue)
{
	for (int i = 0; i < length; i++)
	{
		data[i] = noDataValue;
	}
}

//����һ���޷����ַ����飬��;δ�ڴ�����ֱ�����֣���������ĳ��Ȩ�ػ���Ĥ������
const unsigned char value[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

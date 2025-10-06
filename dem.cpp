#include "dem.h" // ����CDEM�������  
#include "utils.h" // ���ܰ���һЩ���ߺ�������setNoData  
#include <limits>   // ���ͷ�ļ���ʹ�� numeric_limits
#include <new>      // ���ͷ�ļ���ʹ�� nothrow
#include <iostream> // ���ͷ�ļ���ʹ�� cerr
// CDEM���Allocate���������ڷ����ڴ���߳�����  
bool CDEM::Allocate()
{
	delete[] pDem; // �ͷ�֮ǰ������ڴ棨����У�  
	// ���ߴ���Ч��
	if (width == 0 || height == 0) {
		std::cerr << "Invalid DEM dimensions: width or height is zero." << std::endl;
		return false;
	}
	// ��� width * height �Ƿ����
	if (height > std::numeric_limits<size_t>::max() / width) {
		std::cerr << "Overflow in DEM allocation: width * height exceeds limits." << std::endl;
		return false;
	}

	const size_t total_size = width * height;
	pDem = new (std::nothrow) float[total_size];
	if (!pDem) {
		std::cerr << "Failed to allocate DEM memory." << std::endl;
		return false;
	}

	setNoData(pDem, total_size, NO_DATA_VALUE);
	return true;
}

// CDEM���freeMem�����������ͷ��ڴ�  
void CDEM::freeMem()
{
	delete[] pDem; // �ͷ��ڴ�  
	pDem = NULL; // ��ָ����ΪNULL����������ָ��  
}

// CDEM���initialElementsNodata���������ڽ�����Ԫ�س�ʼ��ΪNO_DATA_VALUE  
void CDEM::initialElementsNodata()
{
	setNoData(pDem, width * height, NO_DATA_VALUE); // ���ù��ߺ������г�ʼ��  
}

// CDEM���asFloat���������ڻ�ȡָ������λ�õĸ߳�ֵ  
float CDEM::asFloat(int row, int col) const
{
	return pDem[row * width + col]; // �������м������������ظ߳�ֵ  
}

// CDEM���Set_Value��������������ָ������λ�õĸ߳�ֵ  
void CDEM::Set_Value(int row, int col, float z)
{
	pDem[row * width + col] = z; // �������м������������ø߳�ֵ  
}

// CDEM���is_NoData���������ڼ��ָ������λ���Ƿ�ΪNO_DATA_VALUE  
bool CDEM::is_NoData(int row, int col) const
{
	if (fabs(pDem[row * width + col] - NO_DATA_VALUE) < 0.00001) return true; // �Ƚ��Ƿ�ӽ�NO_DATA_VALUE  
	return false;
}

// CDEM���Assign_NoData���������ڽ�����Ԫ������ΪNO_DATA_VALUE  
void CDEM::Assign_NoData()
{
	for (int i = 0; i < width * height; i++)
		pDem[i] = NO_DATA_VALUE; // �������鲢����ֵ  
}

// CDEM���Get_NY���������ڻ�ȡ�߶ȣ�������  
size_t CDEM::Get_NY() const
{
	return height;
}

// CDEM���Get_NX���������ڻ�ȡ��ȣ�������  
size_t CDEM::Get_NX() const
{
	return width;
}

// CDEM���getDEMdata���������ڻ�ȡ�߳����ݵ�ָ��  
float* CDEM::getDEMdata() const
{
	return pDem;
}

// CDEM���SetHeight�������������ø߶�  
void CDEM::SetHeight(size_t height)
{
	this->height = height;
}

// CDEM���SetWidth�������������ÿ��  
void CDEM::SetWidth(size_t width)
{
	this->width = width;
}

// CDEM���readDEM���������ڴ��ļ���ȡ�߳�����  
void CDEM::readDEM(const std::string& filePath)
{
	std::ifstream is;
	is.open(filePath, std::ios::binary); // �Զ�����ģʽ���ļ�  
	is.read((char*)pDem, sizeof(float) * width * height); // ��ȡ���ݵ��ڴ�  
	is.close(); // �ر��ļ�  
}

// CDEM���is_InGrid���������ڼ��ָ������λ���Ƿ���������  
bool CDEM::is_InGrid(int row, int col) const
{
	if ((row >= 0 && row < height) && (col >= 0 && col < width))
		return true;
	return false;
}

// CDEM���getLength���������ڸ��ݷ�����㳤�ȣ������ǿ��ǶԽ��ߵ������  
float CDEM::getLength(unsigned int dir)
{
	if ((dir & 0x1) == 1) // �����������λ��1����������  
	{
		return 1.41421f; // �Խ��߳��ȣ���2��  
	}
	else return 1.0f; // ����Ϊ1��ˮƽ��ֱ����  
}

// CDEM���getDirction���������ڼ�������  
unsigned char CDEM::getDirction(int row, int col, float spill)
{
	// ���������ͳ�ʼ��  
	int iRow, iCol;
	float iSpill, max, gradient;
	unsigned char steepestSpill;
	max = 0.0f;
	steepestSpill = 255; // ��ʼ��Ϊ��Чֵ  
	unsigned char lastIndexINGridNoData = 0; // ��¼���һ���������ڵ�ΪNO_DATA�ķ�������  

	// ����8������  
	for (int i = 0; i < 8; i++)
	{
		iRow = Get_rowTo(i, row); // ���ݷ���͵�ǰ���м���Ŀ����  
		iCol = Get_colTo(i, col); // ���ݷ���͵�ǰ���м���Ŀ����  

		// ���Ŀ��λ���Ƿ����������Ҳ���NO_DATA  
		if (is_InGrid(iRow, iCol) && !is_NoData(iRow, iCol) && (iSpill = asFloat(iRow, iCol)) < spill)
		{
			gradient = (spill - iSpill) / getLength(i); // �����ݶ�  
			if (max < gradient) // �ҵ�����ݶ�  
			{
				max = gradient;
				steepestSpill = i; // ��¼��������  
			}
		}
		// ��¼���һ���������ڵ�ΪNO_DATA�ķ�������  
		if (!is_InGrid(iRow, iCol) || is_NoData(iRow, iCol))
		{
			lastIndexINGridNoData = i;
		}
	}

	// ���ؽ��������ҵ�������ݶȷ����򷵻ظ÷��򣬷��򷵻����һ���������ڵ�ΪNO_DATA�ķ���  
	return steepestSpill != 255 ? dir[steepestSpill] : dir[lastIndexINGridNoData];
}
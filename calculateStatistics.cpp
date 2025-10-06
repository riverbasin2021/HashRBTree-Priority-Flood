#include "calculateStatistics.h"
#include <algorithm>
#include <numeric>
#include <iostream> // ������׼�����������  
using namespace std;
void calculateStatistics(const CDEM& dem, double* min, double* max, double* mean, double* stdDev)
{
	int width = dem.Get_NX();
	int height = dem.Get_NY();

	int validElements = 0;
	double minValue = 0.0, maxValue = 0.0;
	double sum = 0.0;
	double sumSqurVal = 0.0;
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (!dem.is_NoData(row, col))
			{
				double value = dem.asFloat(row, col);

				if (validElements == 0)
				{
					minValue = maxValue = value;
				}
				// ������ЧԪ�ؼ�����
				validElements++;
				if (minValue > value)
				{
					minValue = value;
				}
				if (maxValue < value)
				{
					maxValue = value;
				}

				sum += value;
				// ����ֵ��ƽ����
				sumSqurVal += (value * value);
			}
		}
	}

	double meanValue = sum / validElements;
	// �����׼�ʹ�÷��ʽ��
	double stdDevValue = sqrt((sumSqurVal / validElements) - (meanValue * meanValue));
	*min = minValue;
	*max = maxValue;
	*mean = meanValue;
	*stdDev = stdDevValue;
	//cout << min << " " << max << " " << mean << endl;
}

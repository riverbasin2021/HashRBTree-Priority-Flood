﻿#include <iostream> // 包含标准输入输出流库  
#include <string> // 包含字符串类库  
#include <fstream> // 包含文件输入输出流库  
#include <queue> // 包含队列容器库  
#include <algorithm> // 包含算法库  
#include "dem.h" // 包含DEM处理相关的头文件  
#include "Node.h" // 包含节点类的头文件  
#include "utils.h" // 包含工具函数的头文件  
#include <time.h> // 包含时间处理的头文件  
#include <list> // 包含双向链表容器库（虽然在这段代码中未直接使用）  
#include <stack> // 包含栈容器库（虽然在这段代码中未直接使用）  
#include <unordered_map> // 包含无序映射容器库（虽然在这段代码中未直接使用）  
#include <chrono>

using namespace std; // 使用标准命名空间  

// 定义Node的向量类型  
typedef std::vector<Node> NodeVector;
// 定义优先级队列，使用Node作为元素，NodeVector作为底层容器，Node::Greater作为比较函数  
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;

long e1=0, e2=0;

int FillDEM_Wang(const char* inputFile, const char* outputFilledPath)
{
	CDEM dem;
	// 定义一个数组来存储地理变换参数
	double geoTransformArgs[6];
	cout << "Reading tiff file..." << endl;
	// 读取TIFF文件到DEM对象中，如果失败则输出错误信息并返回0
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs))
	{
		cout << "error!" << endl;
		return 0;
	}

	const size_t width = static_cast<size_t>(dem.Get_NX());
	const size_t height = static_cast<size_t>(dem.Get_NY());
	cout << "DEM Width:" << width << "  Height:" << height << endl;

	// 定义一个标志对象，用于标记DEM中的元素是否已处理
	Flag flag;
	if (!flag.Init(width, height)) {
		printf("Failed to allocate memory!\n");
		return 0;
	}


	cout << "Using Wang & Liu (2006) method to fill DEM" << endl;
	//time_t timeStart, timeEnd;
	//timeStart = time(NULL);
	auto timeStart = std::chrono::high_resolution_clock::now();
	int priorityQueueCount = 0; // 优先队列处理的节点数量
	PriorityQueue queue;
	// 定义有效元素计数器
	int validElementsCount = 0;
	// push border cells into the PQ
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			Node tmpNode;
			if (!dem.is_NoData(row, col))
			{
				validElementsCount++;
				for (int i = 0; i < 8; i++)
				{
					int iRow, iCol;
					iRow = Get_rowTo(i, row);
					iCol = Get_colTo(i, col);
					if (!dem.is_InGrid(iRow, iCol) || dem.is_NoData(iRow, iCol))
					{
						tmpNode.col = col;
						tmpNode.row = row;
						tmpNode.spill = dem.asFloat(row, col);
						queue.push(tmpNode);
						priorityQueueCount++;
						e1++;
						flag.SetFlag(row, col);
						break;
					}
				}
			}
			else
			{
				flag.SetFlag(row, col);
			}
		}
	}
	int percentFive = validElementsCount / 20;

	int count = 0;
	int iRow, iCol;
	float iSpill;
	while (!queue.empty())
	{
		count++;
		if (count % percentFive == 0)
		{
			int percentNum = count / percentFive;
			cout << "Progress:" << percentNum * 5 << "%\r";
		}
		Node tmpNode = queue.top();
		queue.pop();
		e2++;
		int row = tmpNode.row;
		int col = tmpNode.col;
		float spill = tmpNode.spill;


		for (int i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, row);
			iCol = Get_colTo(i, col);

			// 如果邻居单元格未处理
			if (!flag.IsProcessed(iRow, iCol))
			{
				// 获取邻居单元格的值
				iSpill = dem.asFloat(iRow, iCol);
				if (iSpill <= spill)
				{
					iSpill = spill;
				}
				dem.Set_Value(iRow, iCol, iSpill);
				flag.SetFlag(iRow, iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = iSpill;
				queue.push(tmpNode);
				priorityQueueCount++;
			}

		}

	}
	//timeEnd = time(NULL);
	auto timeEnd = std::chrono::high_resolution_clock::now();
	//double consumeTime = difftime(timeEnd, timeStart);
	std::chrono::duration<double> consumeTime = timeEnd - timeStart;
	cout << "Time used:" << consumeTime.count() << " seconds" << endl;
	cout << "Priority Queue Contribution: " << priorityQueueCount << endl;
	cout << e1 << endl;
	cout << e2 << endl;
	// 计算统计量并创建输出文件  
	double min, max, mean, stdDev;
	calculateStatistics(dem, &min, &max, &mean, &stdDev);
	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(),
		(void*)dem.getDEMdata(), GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	
	return 1;
}
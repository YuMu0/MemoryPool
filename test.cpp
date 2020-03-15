#include <iostream>
#include <random>
#include <vector>
#include <ctime>

// 头文件包含
#include "MP_Allocator.h"

template<class T>
//using MyAllocator = std::allocator<T>;	//STL的空间适配器
using MyAllocator = MP_Allocator<T>;
using Point2D = std::pair<int, int>;

const int TestSize = 10000;
const int PickSize = 1000;

int main()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, TestSize);

	// 测试开始时间
	time_t create_vecints_start = clock();
	// vector创建
	using IntVec = std::vector<int, MyAllocator<int>>;
	std::cout << std::endl << "creat vecints:" << std::endl;
	std::vector<IntVec, MyAllocator<IntVec>> vecints(TestSize);

	std::cout << std::endl << "vecints resize" << std::endl;
	for (int i = 0; i < TestSize; i++)
		vecints[i].resize(dis(gen));
	// 测试结束时间， 下面的都是雷同的
	time_t create_vecints_end = clock();
	std::cout << "The time it takes to creat vector<int> of size " << TestSize << " is "
		<< (create_vecints_end - create_vecints_start) << " ms." << std::endl;

	time_t create_vecpts_start = clock();
	using PointVec = std::vector<Point2D, MyAllocator<Point2D>>;
	std::cout << std::endl << "creat vecpts:" << std::endl;
	std::vector<PointVec, MyAllocator<PointVec>> vecpts(TestSize);
	std::cout << std::endl << "vecpts resize" << std::endl;
	for (int i = 0; i < TestSize; i++)
		vecpts[i].resize(dis(gen));
	time_t create_vecpts_end = clock();
	std::cout << "The time it takes to creat vector<Print2D> of size " << TestSize << " is "
		<< (create_vecpts_end - create_vecpts_start) << " ms." << std::endl;

	// vector resize
	time_t resize_start = clock();
	for (int i = 0; i < PickSize; i++)
	{
		int idx = dis(gen) - 1;
		int size = dis(gen);
		vecints[idx].resize(size);
		vecpts[idx].resize(size);
	}
	time_t resize_end = clock();
	std::cout << "The time it takes to resize " << PickSize << " vector<Print2D> and vector<int> is "
		<< (resize_end - resize_start) << " ms." << std::endl;
	// vector element assignment
	{
		int val = 10;
		int idx1 = dis(gen) - 1;
		int idx2 = vecints[idx1].size() / 2;
		vecints[idx1][idx2] = val;
		if (vecints[idx1][idx2] == val)
			std::cout << "correct assignment in vecints: " << idx1 << std::endl;
		else
			std::cout << "incorrect assignment in vecints: " << idx1 << std::endl;

	}

	{
		Point2D val(11, 15);
		int idx1 = dis(gen) - 1;
		int idx2 = vecpts[idx1].size() / 2;
		vecpts[idx1][idx2] = val;
		if (vecpts[idx1][idx2] == val)
			std::cout << "correct assignment in vecpts: " << idx1 << std::endl;
		else
			std::cout << "incorrect assignment in vecpts: " << idx1 << std::endl;
	}
	std::cout << "end" << std::endl;
	system("pause");
	return 0;
}
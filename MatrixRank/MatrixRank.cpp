#include <ctime>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>

using namespace std;

struct Matrix
{
	size_t row;
	size_t column;
	size_t size;
	size_t beginPosition;
	size_t endPosition;
	vector<vector<double>>* copy;
	istream* in;
};

DWORD WINAPI ReadMatrix(CONST LPVOID data)
{
	auto matrix = (Matrix*)data;
	double size;
	vector<double> row;

	for (size_t i = 0; i < matrix->size; i++)
	{
		row.clear();
		for (size_t j = 0; j < matrix->size; j++)
		{
			(*(matrix->in)) >> size;
			row.push_back(size);
		}
		(*(matrix->copy)).push_back(row);
	}

	DWORD dwResult = 0;
	return dwResult;
}

DWORD WINAPI FindMinorsMatrix(CONST LPVOID data)
{
	auto matrix = (Matrix*)data;

	for (size_t i = matrix->beginPosition; i < matrix->endPosition; ++i)
	{
		if (i != matrix->row && !(abs((*(matrix->copy))[i][matrix->column]) < 0.1))
		{
			for (size_t j = matrix->column + 1; j < (*(matrix->copy)).size(); ++j)
			{
				(*(matrix->copy))[i][j] -= (*(matrix->copy))[matrix->row][j] * (*(matrix->copy))[i][matrix->column];
			}
		}
	}

	ExitThread(0);
	DWORD dwResult = 0;
	return dwResult;
}

void InitMatrix(istream& inputFile, double& threadCount, vector<vector<double>>& inputData, size_t& matrixSize)
{
	inputFile >> matrixSize;
	threadCount = (threadCount > matrixSize) ? matrixSize : threadCount;

	auto* matrix = new Matrix;
	matrix->in = &inputFile;
	matrix->copy = &inputData;
	matrix->size = matrixSize;

	auto matrixData = (LPVOID)matrix;
	HANDLE* handle = new HANDLE;
	*handle = CreateThread(NULL, 0, &ReadMatrix, matrixData, 0, NULL);
	WaitForMultipleObjects(1, handle, true, INFINITE);
}

int CalculateRank(vector<vector<double>> inputData, size_t& size, double threadCount)
{
	vector<vector<double>> matrixCopy(inputData);
	int result = size;
	vector<bool> processed(size);

	for (size_t i = 0; i < size; i++)
	{
		size_t j;

		for (j = 0; j < size; j++)
		{
			double number = matrixCopy[j][i];
			if (!processed[j] && !(abs(number) < 0.1))
				break;
		}

		if (j == size)
		{
			--result;
			continue;
		}

		processed[j] = true;
		for (size_t k = i + 1; k < size; k++)
			matrixCopy[j][k] = matrixCopy[j][k] / matrixCopy[j][i];

		int step = size / threadCount;
		int threadsAmount = threadCount;
		HANDLE* handles = new HANDLE[threadCount];

		for (size_t l = 0; threadsAmount != 0; l += step)
		{
			--threadsAmount;
			auto* matrix = new Matrix;
			matrix->copy = &matrixCopy;
			matrix->column = i;
			matrix->row = j;
			matrix->beginPosition = l;

			matrix->endPosition = (threadsAmount != 0) ? (l + step) : size;
			auto lpVoidMatrix = (LPVOID)matrix;
			handles[threadsAmount] = CreateThread(NULL, 0, &FindMinorsMatrix, lpVoidMatrix, 0, NULL);
		}
		WaitForMultipleObjects(threadCount, handles, true, INFINITE);
	}
	return result;
}

int main(int argc, char* argv[])
{
	HANDLE process = GetCurrentProcess();
	SetProcessAffinityMask(process, 0b1);
	double startTime = clock();

	if (argc != 3)
	{
		cout << "Program.exe <InputFile> <ThreadCount>\n";
		return 1;
	}

	ifstream inputFile(argv[1]);
	if (!inputFile.is_open())
	{
		cout << "<InputFile> not opened\n";
		return 1;
	}

	auto threadCount = stod(argv[2]);
	if (threadCount > 8) {
		cout << "<ThreadCount> must be less than 9\n";
		return 1;
	}

	vector<vector<double>> inputMatrix;
	size_t matrixSize = 0;

	InitMatrix(inputFile, threadCount, inputMatrix, matrixSize);
	cout << "Matrix rank: " << CalculateRank(inputMatrix, matrixSize, threadCount) << endl;

	double endTime = clock();
	cout << "With time: " << endTime - startTime << endl;

	return 0;
}

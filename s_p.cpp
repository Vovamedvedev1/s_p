#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "mpi.h"
using namespace std;

// Функция для перебора комбинаций в заданном диапазоне
void getResult(long long& count, int*& nominals, int*& structureNominals, int count_nominals, int sum, long long startCombination, long long endCombination, int rank)
{
    string name = "example" + to_string(rank);
    ofstream ofs(name);
    if (!ofs.is_open())
        throw "Error open file";
    ofs << "ПРОЦЕСС " << rank << endl;
    int tempSum;
    long long tempCombination;
    for (long long currentCombination = startCombination; currentCombination < endCombination; currentCombination++) {
        tempCombination = currentCombination;
        tempSum = 0;
        for (int i = 0; i < count_nominals; i++)
        {
            structureNominals[i] = tempCombination % (sum / nominals[i] + 1);
            tempCombination /= (sum / nominals[i] + 1);
            tempSum += nominals[i] * structureNominals[i];
        }
        if (tempSum == sum)
        {
            count++;
            ofs << "Решение " << count << ": ";
            for (int i = 0; i < count_nominals; ++i)
                ofs << nominals[i] << ":" << structureNominals[i] << " ";
            ofs << endl;
        }
    }
    ofs.close();
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "Russian");
    int rank, size;
    double start, stop;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int sum;
    int count_nominals;
    int* re = new int[2];
    int* nominals = nullptr;
    if (rank == 0) {
        cout << "Enter the amount you want to dial: "; cin >> sum;
        cout << "Enter the number of coin denominations: "; cin >> count_nominals;
        nominals = new int[count_nominals];
        cout << "Enter the coin denominations in rubles: ";
        for (int i = 0; i < count_nominals; i++) {
            cin >> nominals[i];
        }
        re[0] = sum;
        re[1] = count_nominals;
    } 
    MPI_Bcast(&re[0], 2, MPI_INT, 0, MPI_COMM_WORLD);
    sum = re[0];
    count_nominals = re[1];
    delete[]re;
    int* structureNominals = new int[count_nominals];
    for (int i = 0; i < count_nominals; i++)
        structureNominals[i] = 0;
    if (rank != 0)
        nominals = new int[count_nominals];
    MPI_Bcast(nominals, count_nominals, MPI_INT, 0, MPI_COMM_WORLD);
    long long totalCombinations = 1;
    for (int i = 0; i < count_nominals; ++i)
        totalCombinations *= (sum / nominals[i] + 1);
    long long cS = totalCombinations / size;
    long long startCombination = rank * cS;
    long long endCombination = (rank == size - 1) ? totalCombinations : startCombination + cS;
    long long localCount = 0;
    long long globalCount = 0;
    if (rank == 0)
        start = MPI_Wtime();
    try
    {
        getResult(localCount, nominals, structureNominals, count_nominals, sum, startCombination, endCombination, rank);
    }
    catch (const char* ex)
    {
        std::cout << ex << endl;
        MPI_Abort(MPI_COMM_WORLD, 0);
        return 0;
    }

    MPI_Reduce(&localCount, &globalCount, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0)
    {
        ofstream finalFile("final_output.txt");
        if (finalFile.is_open())
        {
            for (int i = 0; i < size; i++) {
                string name = "example" + to_string(i);
                ifstream localFile(name);
                if (localFile.is_open())
                {
                    string line;
                    while (getline(localFile, line))
                        finalFile << line << endl;
                    localFile.close();
                    if (remove(name.c_str()) != 0)
                    {
                        cout << "Ошибка при удалении файла " << name << endl;
                        MPI_Abort(MPI_COMM_WORLD, 0);
                        return 0;
                    }
                }
                else
                {
                    cout << "Ошибка при открытии файла " << name << endl;
                    MPI_Abort(MPI_COMM_WORLD, 0);
                    return 0;
                }
            }
            finalFile.close();
            cout << "Все файлы объединены в final_output.txt" << endl;
        }
        else
        {
            cout << "Ошибка при открытии файла final_output.txt" << endl;
            MPI_Abort(MPI_COMM_WORLD, 0);
            return 0;
        }
        stop = MPI_Wtime();
        cout << "-------- The total number of options to dial " << sum << " руб: " << globalCount << "----------" << endl;
        cout << setprecision(10) << fixed << "TIME: " << fixed << stop - start << endl;
    }
    delete[] nominals;
    delete[] structureNominals;
    MPI_Finalize();
    return 0;
}
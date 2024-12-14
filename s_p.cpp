#include <iostream>
#include <fstream>
#include <iomanip>
#include "mpi.h"
using namespace std;

void getResult(long long& count, int* nominals, int count_nominals, int sum, ofstream& file)
{
    int* structureNominals = new int[count_nominals];
    for (int i = 0; i < count_nominals; i++)
        structureNominals[i] = 0;
    int tempSum;
    int k;
    while (true)
    {
        tempSum = 0;
        for (int i = 0; i < count_nominals; i++)
            tempSum += nominals[i] * structureNominals[i];
        if (tempSum == sum)
        {
            count++;
            file << "Решение " << count << ": ";
            for (int i = 0; i < count_nominals; ++i)
                file << nominals[i] << ":" << structureNominals[i] << " ";
            file << endl;
        }
        k = count_nominals - 1;
        while (k >= 0 && structureNominals[k] == sum / nominals[k])
        {
            structureNominals[k] = 0;
            k--;
        }
        if (k < 0)
            break;
        structureNominals[k]++;
    }
}


int main(int argc, char** argv)
{
    setlocale(LC_ALL, "Russian");
    int rank, size;
    double start, stop;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int sum;   //искомая сумма
    cout << "Введите сумму, которую требуется набрать: "; cin >> sum;
    int count_nominals;   //количество номиналов монет
    cout << "Введите количество номиналов монет: "; cin >> count_nominals;
    int* nominals = new int[count_nominals];   //номиналы монет
    cout << "Введите номиналы монет в рублях: ";
    for (int i = 0; i < count_nominals; i++)
        cin >> nominals[i];
    ofstream file("result.txt");
    if (file.is_open())
    {
        long long count = 0;
        start = MPI_Wtime();
        getResult(count, nominals, count_nominals, sum, file);
        stop = MPI_Wtime();
        cout << "-------- Количество вариантов набрать 100 руб: " << count << "----------" << endl;
        cout << setprecision(10) << fixed << "Время, затраченное на перебор: " << fixed << stop - start << endl;
    }
    else
    {
        cout << "Ошибка открытия файла!!!" << endl;
        return 0;
    }
    file.close();
    cout << "Результат записан !!!" << endl;
    delete[]nominals;
    MPI_Finalize();
    return 0;
}
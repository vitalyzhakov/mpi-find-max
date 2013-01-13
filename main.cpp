/* 
 * File:   main.cpp
 * Author: zhakov
 * 
 * Ищем максимум в массиве средствами MPI
 *
 * Created on 13 Январь 2013 г., 14:51
 */

#include <mpi.h>
#include <cstdlib>
#include <stdio.h>


/**
 * Инициализация верктора случайными значениями
 * 
 * @param a - вектор, который нужно заполнить
 * @param n - длина вектора
 * @return double - реальный максимум
 */
double initRandVector(double *a, int n) {
    double real_max;
    real_max = 0;
    for (int i = 0; i < n; i++) {
        a[i] = rand() / (double) RAND_MAX;
        if (a[i] > real_max) {
            real_max = a[i];
        }
    }
    return real_max;
}

/**
 * Точка входа
 * 
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char** argv) {
    
    MPI::Init(argc, argv); //Nachalo MPI-sekcii

    int procCount, procIndex; //ProcNum - kolichestvo processov, ProcRank - nomer processa

    
    procCount = MPI::COMM_WORLD.Get_size(); //Uznaem kolichestvo processov
    procIndex = MPI::COMM_WORLD.Get_rank(); //Uznaem nomer tekuschego processa

    int size = 64000000;
    if (argc == 2) { //Возможно, аргумент послан из командной строки
        size = atoi(argv[1]);
    }

    double startParallel, stopParallel; //Переменные для подсчёта времени
    


    double *a; //Исходный массив
    double *procA;
    double procMax;
    double calculatedMax;
    double realMax;

    if (procIndex == 0) { //Zabivaem randomnii massiv v pamyat host-processa
        a = (double *) malloc(size * sizeof (double));
        realMax = initRandVector(a, size);
    }

    startParallel = MPI_Wtime();
    int recieveSize = size / procCount;
    if (procIndex == 0) {
        printf("Array size per process is %d\n", size / procCount);
    }

    procA = (double *) malloc(size * sizeof (double) / procCount);

    //Rassilaem ravnie porcii massiva po processam
    MPI::COMM_WORLD.Scatter(a, recieveSize, MPI::DOUBLE, procA, recieveSize, MPI::DOUBLE, 0);
    
    //Ishem local maksimum v etih porciyah
    procMax = procA[0];
    for (int i = 0; i < recieveSize; i++) {
        if (procA[i] > procMax) {
            procMax = procA[i];
        }
    }
    
    //Sobiraem local maksimumi na host processe i nahodim sredi nih maksimum
    MPI::COMM_WORLD.Reduce(&procMax, &calculatedMax, 1, MPI::DOUBLE, MPI::MAX, 0);
    
    if (procIndex == 0) {
        if (calculatedMax == realMax) {
            printf("Max parallel=%f\n", calculatedMax);
        } else {
            printf("No Success Calculation Maximum! U must recalculate this!");
        }
    }




    stopParallel = MPI_Wtime();
    if (procIndex == 0) {
        printf("Execution time= %f, process count= %d, size= %d\n", stopParallel - startParallel);
    }

    MPI::Finalize(); //Zavershaem parallelnuyu sekciyu
    return 0;
}
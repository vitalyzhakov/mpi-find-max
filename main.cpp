/* 
 * File:   main.cpp
 * Author: zhakov
 * 
 * Ищем максимум в массиве средствами MPI
 *
 * Created on 13 Январь 2013 г., 14:51
 */

#include <cstdlib>
#include <stdio.h>
#include <mpi.h>

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

    int size = 1024000;
    if (argc == 2) { //Возможно, аргумент послан из командной строки
        size = atoi(argv[1]);
    }

    double StartPar, StopPar, StartSeq, StopSeq; //Variables 4 time
    int ProcNum, ProcRank; //ProcNum - kolichestvo processov, ProcRank - nomer processa

    MPI_Init(&argc, &argv); //Nachalo MPI-sekcii

    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum); //Uznaem kolichestvo processov
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank); //Uznaem nomer tekuschego processa

    if (ProcRank == 0) {
        printf("Razmer massiva na 1 process = %d\n", size);
    }


    double *a;
    double *procA;
    double procMax;
    double Max;
    double ideal_max;

    if (ProcRank == 0) //Zabivaem randomnii massiv v pamyat host-processa
    {
        srand(time(0));
        a = (double *) malloc(size * ProcNum * sizeof (double));
        ideal_max = randInit(a, size * ProcNum);
    }

    StartPar = MPI_Wtime();

    procA = (double *) malloc(size / ProcNum * sizeof (double));

    //Rassilaem ravnie porcii massiva po processam
    MPI_Scatter(a, size, MPI_DOUBLE, procA, size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //Ishem local maksimum v etih porciyah
    procMax = procA[0];
    int i;
    for (i = 0; i < size; i++)
        if (procA[i] > procMax) procMax = procA[i];

    //Sobiraem local maksimumi na host processe i nahodim sredi nih maksimum
    MPI_Reduce(&procMax, &Max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (ProcRank == 0) {
        if (Max == ideal_max)
            printf("Max parallel=%f\n", Max);
        else
            printf("No Success Calculation Maximum! U must recalculate this!");
    }

    StopPar = MPI_Wtime();
    if (ProcRank == 0) printf("Time parallel = %f\n", StopPar - StartPar);

    MPI_Finalize(); //Zavershaem parallelnuyu sekciyu

    return 0; //VOZVRASHAEM OVAL oO
}

/**
 * Инициализация верктора случайными значениями
 * 
 * @param a - вектор, который нужно заполнить
 * @param n - длина вектора
 * @return double - реальный максимум
 */
double randInit(double *a, int n) {
    double real_max;
    srand(time(0));
    real_max = 0;
    for (int i = 0; i < n; i++) {
        a[i] = rand() / (double) RAND_MAX;
        if (a[i] > real_max) {
            real_max = a[i];
        }
    }
    return real_max;
}
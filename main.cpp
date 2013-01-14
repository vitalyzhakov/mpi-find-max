/* 
 * File:   main.cpp
 * Author: zhakov
 * 
 * Ищем максимум в массиве средствами MPI
 * Генерируем элементы на процессах, 
 * потом считаем результат в процессе и собираем его
 *
 * Created on 13 Январь 2013 г., 14:51
 */

#include <mpi.h>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

//Режимы заполнения вектора
#define FILL_MODE_NOT_RANDOM 0
#define FILL_MODE_RANDOM 1

/**
 * Заполняем вектор
 * 
 * @param vector - вектор, который нужно заполнить
 * @param vSize - размер этого вектора
 * @param mode - режим заполнения (случайный/не случайный)
 * @param procIndex - номер процесса в коммуникаторе MPI
 * @return 
 */
int initVector(double *vector, unsigned int vSize, unsigned int fillMode, unsigned int procIndex) {

    for (unsigned int i = 0; i < vSize; i++) {
        if (fillMode == FILL_MODE_RANDOM) {
            vector[i] = rand() / (double) RAND_MAX;
        } else {
            vector[i] = procIndex;
        }
    }
    return 0;
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
    
    //Размер вектора
    unsigned int vSize = 1000000;
    if (argc == 2) { //Возможно, аргумент послан из командной строки
        vSize = atoi(argv[1]);
    }

    //Режим заполнения вектора
    unsigned int fillMode = FILL_MODE_NOT_RANDOM;
    if (argc == 3) {
        if (strcmp("-random", argv[3])) {
            fillMode = FILL_MODE_RANDOM;
        }
    }

    int procCount, procIndex; //ProcNum - kolichestvo processov, ProcRank - nomer processa


    procCount = MPI::COMM_WORLD.Get_size(); //Uznaem kolichestvo processov
    procIndex = MPI::COMM_WORLD.Get_rank(); //Uznaem nomer tekuschego processa

    

    double startParallel, stopParallel; //Переменные для подсчёта времени
    startParallel = MPI_Wtime();


    double *dataVector; //Исходный массив
    double procMax;
    double calculatedMax;
    double realMax;

    //Генерим в массивы случайные числа
    dataVector = (double *) malloc(vSize * sizeof (double));
    realMax = initVector(dataVector, vSize, fillMode, procIndex);


    //unsigned int recieveSize = size / procCount;
    if (procIndex == 0) {
        printf("Array size per process is %u\n", vSize);
    }


    //Ищем локальный максимум в этих процессах
    procMax = dataVector[0];
    for (unsigned int i = 0; i < vSize; i++) {
        if (dataVector[i] > procMax) {
            procMax = dataVector[i];
        }
    }

    //Редукция: собираем локальные максимумы на каждом процессе в процесс с номером 0
    MPI::COMM_WORLD.Reduce(&procMax, &calculatedMax, 1, MPI::DOUBLE, MPI::MAX, 0);

    //Проверка на правильность, если включен соответствующий режим генерации
    if (procIndex == 0) {
        //Мы можем проверить, если массивы заполнялись числами в соответствии с номерами процессов
        if (fillMode == FILL_MODE_NOT_RANDOM) {
            if (calculatedMax == procCount - 1) {
                printf("Max parallel = %f\n", calculatedMax);
            } else {
                printf("No Success Calculation Maximum! You must recalculate this!");
                return 1;
            }
        }//Иначе цифры случайные - ничего не делаем
    }


    //Заканчиваем сбор времени
    stopParallel = MPI_Wtime();
    //Выводим строку-результат на нулевом процессе
    if (procIndex == 0) {
        printf("Execution time= %f, process count= %d, size= %u\n",
                stopParallel - startParallel,
                procCount,
                vSize
                );
    }

    //Zavershaem parallelnuyu sekciyu
    MPI::Finalize();
    
    return 0;
}
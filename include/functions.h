#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "RingBuffer.h"
#include <iomanip>

using namespace std;

// Первая глобальная шаблонная функция
// Вывод одного значения
template <typename T>
void printValue(const T &value)
{
    cout << value;
}

// Полная специализация функции для double
template <>
inline void printValue<double>(const double &value)
{
    cout << fixed
              << setprecision(2)
              << value;
}

// Вторая глобальная шаблонная функция
// Вывод содержимого кольцевого буфера
template <typename T, size_t Capacity, bool UseMemcpy>
void printBuffer(const RingBuffer<T, Capacity, UseMemcpy> &buffer)
{
    auto elements = buffer.getOrderedData();

    cout << "[ ";

    for (size_t i = 0; i < buffer.size(); i++)
    {
        printValue(elements[i]);

        cout << " ";
    }

    cout << "]";

    cout << " ("
              << buffer.size()
              << "/"
              << Capacity
              << ")";
}

#endif

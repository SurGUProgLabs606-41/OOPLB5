// Шаблонные функции для вывода данных
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "RingBuffer.h"
#include <iomanip>

using namespace std;

/**
 * @brief Выводит одно значение в стандартный поток вывода.
 *
 * Общая версия использует оператор << соответствующего типа. Для double
 * ниже определена отдельная специализация с фиксированной точностью.
 * Глобальная шаблонная функция.
 *
 * @tparam T Тип выводимого значения.
 * @param value Константная ссылка на значение для отображения.
 */
template <typename T>
void printValue(const T &value)
{
    cout << value;
}

/**
 * @brief Выводит значение типа double с двумя знаками после запятой.
 * @param value Значение с плавающей точкой для отображения.
 * @note Манипуляторы fixed и setprecision влияют на последующий вывод
 * чисел с плавающей точкой в этом потоке.
 * Явная специализация шаблонной функции printValue для типа double.
 */
template <>
inline void printValue<double>(const double &value)
{
    cout << fixed
         << setprecision(2) // Устанавливает 2 знака после запятой.
         << value;
}

/**
 * @brief Выводит все элементы кольцевого буфера в логическом порядке.
 *
 * Сначала получает упорядоченную копию данных, затем выводит только
 * фактически занятые позиции. В конце показывает заполненность в формате
 * «количество элементов / вместимость».
 * 
 * Глобальная шаблонная функция
 *
 * @tparam T Тип элементов буфера.
 * @tparam Capacity Вместимость буфера.
 * @tparam UseMemcpy Выбранная специализация кольцевого буфера.
 * @param buffer Константная ссылка на буфер, который требуется вывести.
 * Сам класс имеет 3 параметра.
 * Не Capacity, потому что буфер может быть частично заполнен, а Capacity - это максимальная вместимость.
 */
template <typename T, size_t Capacity, bool UseMemcpy>
void printBuffer(const RingBuffer<T, Capacity, UseMemcpy> &buffer)
{
    auto elements = buffer.getOrderedData(); // Копия элемента в логическом порядке.

    cout << "[ ";

    for (size_t i = 0; i < buffer.size(); i++) // Вывод тодько фактически занятых элементов.
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

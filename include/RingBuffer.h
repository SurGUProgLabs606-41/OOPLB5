#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <iostream>
#include <array>
#include <cstring>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <string>

using namespace std;

// Общая статистика для всех кольцевых буферов
inline static size_t maxCapacity = 0;

// Предварительное объявление шаблонного класса
template <
    typename T,
    size_t Capacity,
    bool UseMemcpy = is_trivially_copyable<T>::value>
class RingBuffer;

// Предварительное объявление дружественной функции
template <typename T, size_t Capacity, bool UseMemcpy>
double calculateAverage(
    const RingBuffer<T, Capacity, UseMemcpy> &buffer);

// Базовый класс кольцевого буфера
template <typename T, size_t Capacity>
class RingBufferCore
{
    static_assert(
        Capacity > 0,
        "Capacity must be greater than zero");

    static_assert(
        is_arithmetic<T>::value ||
            is_same<T, string>::value,
        "Only arithmetic types and string are supported");

    static_assert(
        is_default_constructible<T>::value &&
            is_copy_assignable<T>::value,
        "T must be default constructible and copy assignable");

protected:
    T data[Capacity]{};

    size_t head = 0;
    size_t count = 0;

public:
    using value_type = T;

    // Дружественная шаблонная функция
    template <typename U, size_t N, bool M>
    friend double calculateAverage(
        const RingBuffer<U, N, M> &buffer);

    RingBufferCore()
    {
        if (Capacity > maxCapacity)
        {
            maxCapacity = Capacity;
        }
    }

    // Добавление элемента
    void push(const T &value)
    {
        if (count < Capacity)
        {
            size_t index = (head + count) % Capacity;

            data[index] = value;
            count++;
        }
        else
        {
            data[head] = value;

            head = (head + 1) % Capacity;
        }
    }

    // Удаление первого элемента
    void pop()
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        head = (head + 1) % Capacity;
        count--;
    }

    // Получение первого элемента
    T &front()
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        return data[head];
    }

    const T &front() const
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        return data[head];
    }

    // Получение последнего элемента
    T &back()
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        return data[(head + count - 1) % Capacity];
    }

    const T &back() const
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        return data[(head + count - 1) % Capacity];
    }

    // Количество элементов
    size_t size() const
    {
        return count;
    }

    // Проверка пустоты
    bool empty() const
    {
        return count == 0;
    }

    // Получение максимальной зафиксированной ёмкости
    static size_t getMaxCapacity()
    {
        return maxCapacity;
    }

    // Получение элементов в обычном порядке
    array<T, Capacity> getOrderedData() const
    {
        array<T, Capacity> result{};

        for (size_t i = 0; i < count; i++)
        {
            result[i] = data[(head + i) % Capacity];
        }

        return result;
    }
};

#endif

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

// Основной шаблон для тривиально копируемых типов
template <typename T, size_t Capacity, bool UseMemcpy>
class RingBuffer
    : public RingBufferCore<T, Capacity>
{
public:
    using Base = RingBufferCore<T, Capacity>;
    using Base::Base;
};

// Частичная специализация для тривиально копируемых типов
template <typename T, size_t Capacity>
class RingBuffer<T, Capacity, true>
    : public RingBufferCore<T, Capacity>
{
public:
    using Base = RingBufferCore<T, Capacity>;
    using Base::Base;

    // Оптимизированное получение элементов через memcpy
    array<T, Capacity> getOrderedData() const
    {
        array<T, Capacity> result{};

        size_t currentCount = this->count;

        if (currentCount == 0)
        {
            return result;
        }

        // Количество элементов до конца массива
        size_t firstPart = min(
            currentCount,
            Capacity - this->head);

        // Количество элементов в начале массива
        size_t secondPart = currentCount - firstPart;

        // Копирование первой части
        memcpy(
            result.data(),
            this->data + this->head,
            firstPart * sizeof(T));

        // Копирование второй части
        if (secondPart > 0)
        {
            memcpy(
                result.data() + firstPart,
                this->data,
                secondPart * sizeof(T));
        }

        return result;
    }
};

// Частичная специализация для нетривиально копируемых типов
template <typename T, size_t Capacity>
class RingBuffer<T, Capacity, false>
    : public RingBufferCore<T, Capacity>
{
public:
    using Base = RingBufferCore<T, Capacity>;
    using Base::Base;
};

#endif

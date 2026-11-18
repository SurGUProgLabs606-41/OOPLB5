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

/**
 * @brief Хранит общую статистику всех созданных кольцевых буферов.
 *
 * Переменная используется всеми специализациями RingBufferCore. В ней
 * сохраняется наибольшая вместимость Capacity, встречавшаяся у созданных
 * объектов. Значение не является количеством буферов или элементов.
 */
inline static size_t maxCapacity = 0;

/**
 * @brief Предварительное объявление шаблонного кольцевого буфера.
 * @tparam T Тип элементов, которые будут храниться в буфере.
 * @tparam Capacity Максимальное количество элементов в одном буфере.
 * @tparam UseMemcpy Выбирает реализацию получения упорядоченных данных:
 * true для тривиально копируемых типов, false для остальных типов.
 */
template <
    typename T,
    size_t Capacity,
    bool UseMemcpy = is_trivially_copyable<T>::value>
class RingBuffer;

/**
 * @brief Предварительное объявление функции вычисления среднего значения.
 * @tparam T Тип элементов буфера.
 * @tparam Capacity Вместимость буфера.
 * @tparam UseMemcpy Признак выбранной специализации RingBuffer.
 * @param buffer Константная ссылка на буфер, элементы которого обрабатываются.
 * @return Среднее арифметическое элементов в виде double.
 */
template <typename T, size_t Capacity, bool UseMemcpy>
double calculateAverage(
    const RingBuffer<T, Capacity, UseMemcpy> &buffer);

/**
 * @brief Реализует основное хранение и операции кольцевого буфера.
 *
 * Элементы располагаются во внутреннем массиве фиксированного размера.
 * Индекс head указывает на первый логический элемент, а count хранит
 * текущее количество элементов. Когда буфер заполнен, push перезаписывает
 * самый старый элемент и сдвигает head по кругу.
 *
 * @tparam T Тип хранимых элементов. Поддерживаются арифметические типы
 * и std::string.
 * @tparam Capacity Фиксированная вместимость буфера; должна быть больше нуля.
 */
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
    /** @brief Массив фиксированной вместимости для физического хранения элементов. */
    T data[Capacity]{};

    /** @brief Индекс первого элемента в физическом массиве. */
    size_t head = 0;
    /** @brief Текущее количество элементов, находящихся в буфере. */
    size_t count = 0;

public:
    using value_type = T;

    /**
     * @brief Разрешает calculateAverage обращаться к внутренним данным буфера.
     * @tparam U Тип элементов обрабатываемого буфера.
     * @tparam N Вместимость обрабатываемого буфера.
     * @tparam M Признак специализации обрабатываемого буфера.
     */
    template <typename U, size_t N, bool M>
    friend double calculateAverage(
        const RingBuffer<U, N, M> &buffer);

    /**
     * @brief Создаёт пустой буфер и обновляет общую максимальную вместимость.
     *
     * Все элементы массива инициализируются значениями по умолчанию,
     * head и count начинают с нуля. Если Capacity больше ранее
     * зарегистрированного максимума, обновляет maxCapacity.
     */
    RingBufferCore()
    {
        if (Capacity > maxCapacity)
        {
            maxCapacity = Capacity;
        }
    }

    /**
     * @brief Добавляет значение в конец буфера.
     *
     * Если свободное место есть, значение записывается после последнего
     * элемента. Если буфер заполнен, новое значение заменяет самый старый
     * элемент, после чего head перемещается на следующую позицию по кругу.
     *
     * @param value Значение, которое требуется добавить.
     */
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

    /**
     * @brief Удаляет первый (самый старый) элемент буфера.
     *
     * Физические данные не сдвигаются: изменяются только head и count.
     *
     * @throws std::out_of_range Если буфер пуст.
     */
    void pop()
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        head = (head + 1) % Capacity;
        count--;
    }

    /**
     * @brief Возвращает ссылку на первый элемент буфера.
     * @return Изменяемая ссылка на самый старый элемент.
     * @throws std::out_of_range Если буфер пуст.
     */
    T &front()
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        return data[head];
    }

    /**
     * @brief Возвращает константную ссылку на первый элемент буфера.
     * @return Константная ссылка на самый старый элемент.
     * @throws std::out_of_range Если буфер пуст.
     */
    const T &front() const
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        return data[head];
    }

    /**
     * @brief Возвращает ссылку на последний добавленный элемент.
     * @return Изменяемая ссылка на последний элемент.
     * @throws std::out_of_range Если буфер пуст.
     */
    T &back()
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        return data[(head + count - 1) % Capacity];
    }

    /**
     * @brief Возвращает константную ссылку на последний элемент буфера.
     * @return Константная ссылка на последний элемент.
     * @throws std::out_of_range Если буфер пуст.
     */
    const T &back() const
    {
        if (count == 0)
        {
            throw out_of_range("Buffer is empty");
        }

        return data[(head + count - 1) % Capacity];
    }

    /**
     * @brief Возвращает количество элементов, фактически находящихся в буфере.
     * @return Число элементов от 0 до Capacity включительно.
     */
    size_t size() const
    {
        return count;
    }

    /**
     * @brief Проверяет, содержит ли буфер хотя бы один элемент.
     * @return true, если буфер пуст; иначе false.
     */
    bool empty() const
    {
        return count == 0;
    }

    /**
     * @brief Возвращает максимальную вместимость среди созданных буферов.
     * @return Наибольшее значение Capacity, зарегистрированное конструкторами.
     */
    static size_t getMaxCapacity()
    {
        return maxCapacity;
    }

    /**
     * @brief Копирует элементы в обычном логическом порядке.
     *
     * Результат начинается с самого старого элемента и заканчивается
     * самым новым. Неиспользуемые позиции результирующего массива
     * остаются инициализированными значениями по умолчанию.
     *
     * @return Массив фиксированного размера Capacity с упорядоченными данными.
     */
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

/**
 * @brief Основной шаблон оболочки кольцевого буфера.
 * @tparam T Тип хранимых элементов.
 * @tparam Capacity Максимальное количество элементов.
 * @tparam UseMemcpy Признак выбора оптимизированной реализации.
 *
 * Наследует базовые операции от RingBufferCore. Для тривиально копируемых
 * типов применяется частичная специализация ниже.
 */
template <typename T, size_t Capacity, bool UseMemcpy>
class RingBuffer
    : public RingBufferCore<T, Capacity>
{
public:
    using Base = RingBufferCore<T, Capacity>;
    using Base::Base;
};

/**
 * @brief Специализация буфера для тривиально копируемых типов.
 * @tparam T Тип элементов, для которого допустимо побайтовое копирование.
 * @tparam Capacity Вместимость буфера.
 *
 * Переопределяет getOrderedData и использует memcpy для копирования
 * одной или двух непрерывных частей кольцевого массива.
 */
template <typename T, size_t Capacity>
class RingBuffer<T, Capacity, true>
    : public RingBufferCore<T, Capacity>
{
public:
    using Base = RingBufferCore<T, Capacity>;
    using Base::Base;

    /**
     * @brief Возвращает элементы в логическом порядке с помощью memcpy.
     *
     * Если данные не пересекают конец внутреннего массива, выполняется
     * одно копирование. Если кольцо обернулось, данные копируются двумя
     * частями: от head до конца массива и затем с начала массива.
     *
     * @return Массив размера Capacity с элементами от самого старого к новому.
     */
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

/**
 * @brief Специализация буфера для нетривиально копируемых типов.
 * @tparam T Тип элементов, для которых нельзя применять побайтовое копирование.
 * @tparam Capacity Вместимость буфера.
 *
 * Использует обычную реализацию getOrderedData из RingBufferCore,
 * копирующую элементы по одному с учетом циклического расположения.
 */
template <typename T, size_t Capacity>
class RingBuffer<T, Capacity, false>
    : public RingBufferCore<T, Capacity>
{
public:
    using Base = RingBufferCore<T, Capacity>;
    using Base::Base;
};

/**
 * @brief Вычисляет среднее арифметическое элементов кольцевого буфера.
 *
 * Функция доступна только для арифметических типов. Обходит логические
 * элементы от head с учетом перехода через конец массива. Для вычисления
 * сумма преобразуется к double, чтобы результат не ограничивался
 * целочисленным типом исходных элементов.
 *
 * @tparam T Арифметический тип элементов буфера.
 * @tparam Capacity Вместимость буфера.
 * @tparam UseMemcpy Признак специализации буфера.
 * @param buffer Константная ссылка на буфер для вычисления среднего.
 * @return Среднее арифметическое всех элементов типа double.
 * @throws std::out_of_range Если буфер пуст.
 * @note Для std::string функция не подходит; это проверяется через static_assert.
 */
template <typename T, size_t Capacity, bool UseMemcpy>
double calculateAverage(
    const RingBuffer<T, Capacity, UseMemcpy> &buffer)
{
    static_assert(
        is_arithmetic<T>::value,
        "Average calculation requires an arithmetic type");

    const RingBufferCore<T, Capacity> &base = buffer;

    if (base.count == 0)
    {
        throw out_of_range("Buffer is empty");
    }

    double sum = 0;

    for (size_t i = 0; i < base.count; i++)
    {
        sum += static_cast<double>(
            base.data[(base.head + i) % Capacity]);
    }

    return sum / base.count;
}

#endif

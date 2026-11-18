#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <string>
#include <stdexcept>

#include "RingBuffer.h"
#include "functions.h"

using namespace std;

/**
 * @brief Наборы буферов разных типов и фиксированной вместимости.
 *
 * В каждом массиве зарезервировано место максимум для десяти объектов.
 * Фактическое число загруженных объектов хранится в отдельных счетчиках.
 */
RingBuffer<int, 5> intBuffers5[10];
RingBuffer<double, 5> doubleBuffers5[10];
RingBuffer<float, 7> floatBuffers7[10];
RingBuffer<int, 10> intBuffers10[10];
RingBuffer<double, 10> doubleBuffers10[10];

/** @brief Число загруженных буферов RingBuffer<int, 5>. */
size_t intCount5 = 0;
/** @brief Число загруженных буферов RingBuffer<double, 5>. */
size_t doubleCount5 = 0;
/** @brief Число загруженных буферов RingBuffer<float, 7>. */
size_t floatCount7 = 0;
/** @brief Число загруженных буферов RingBuffer<int, 10>. */
size_t intCount10 = 0;
/** @brief Число загруженных буферов RingBuffer<double, 10>. */
size_t doubleCount10 = 0;

/**
 * @brief Подсчитывает общее количество загруженных буферов всех типов.
 * @return Сумма пяти счетчиков загруженных объектов.
 */
size_t getTotalCount()
{
    return intCount5 +
           doubleCount5 +
           floatCount7 +
           intCount10 +
           doubleCount10;
}

/**
 * @brief Считывает значения одной строки файла и создает один буфер.
 *
 * Значения читаются из уже подготовленного потока line. Созданный буфер
 * записывается в первый свободный элемент массива buffers, после чего
 * счетчик bufferCount увеличивается.
 *
 * @tparam T Тип элементов загружаемого буфера.
 * @tparam Capacity Вместимость одного буфера.
 * @param line Поток с оставшейся частью строки: в нем находятся значения.
 * @param count Количество значений, которое нужно считать из строки.
 * @param buffers Массив, в который будет помещен созданный буфер.
 * @param bufferCount Счетчик занятых элементов массива; изменяется функцией.
 * @throws std::runtime_error Если значений больше вместимости, превышен
 * лимит объектов или данные не удалось корректно прочитать.
 */
template <typename T, size_t Capacity>
void loadBuffer(
    istringstream &line,
    size_t count,
    RingBuffer<T, Capacity> buffers[],
    size_t &bufferCount)
{
    if (count > Capacity)
    {
        throw runtime_error("Слишком много элементов в буфере");
    }

    if (bufferCount >= 10)
    {
        throw runtime_error("Превышено допустимое количество буферов");
    }

    RingBuffer<T, Capacity> buffer;

    for (size_t i = 0; i < count; i++)
    {
        T value;

        if (!(line >> value))
        {
            throw runtime_error("Некорректные данные в файле");
        }

        buffer.push(value);
    }

    buffers[bufferCount] = buffer;
    bufferCount++;
}

/**
 * @brief Загружает из data.txt все десять объектов кольцевого буфера.
 *
 * Каждая непустая строка задает тип, вместимость, количество элементов
 * и сами значения. По первым двум параметрам выбирается подходящий массив
 * и инстанцирование loadBuffer. Проверяются формат строк, поддерживаемые
 * комбинации типа и вместимости, лишние значения и итоговое количество
 * загруженных объектов.
 *
 * @return true, если файл прочитан и загружено ровно десять буферов;
 * false, если файл не открылся или при чтении обнаружена ошибка.
 */
bool loadData()
{
    ifstream file("../data/data.txt");

    if (!file.is_open())
    {
        cout << "Ошибка: не удалось открыть файл data/data.txt\n";
        return false;
    }

    string line;

    try
    {
        while (getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            if (getTotalCount() >= 10)
            {
                throw runtime_error("В файле слишком много буферов");
            }

            istringstream stream(line);

            string type;
            size_t capacity;
            size_t count;

            if (!(stream >> type >> capacity >> count))
            {
                throw runtime_error("Некорректная строка в файле");
            }

            if (type == "int" && capacity == 5)
            {
                loadBuffer<int, 5>(
                    stream, count, intBuffers5, intCount5);
            }
            else if (type == "double" && capacity == 5)
            {
                loadBuffer<double, 5>(
                    stream, count, doubleBuffers5, doubleCount5);
            }
            else if (type == "float" && capacity == 7)
            {
                loadBuffer<float, 7>(
                    stream, count, floatBuffers7, floatCount7);
            }
            else if (type == "int" && capacity == 10)
            {
                loadBuffer<int, 10>(
                    stream, count, intBuffers10, intCount10);
            }
            else if (type == "double" && capacity == 10)
            {
                loadBuffer<double, 10>(
                    stream, count, doubleBuffers10, doubleCount10);
            }
            else
            {
                throw runtime_error(
                    "Неподдерживаемый тип данных или размер буфера");
            }

            string extra;

            if (stream >> extra)
            {
                throw runtime_error("Лишние данные в строке файла");
            }
        }
    }
    catch (const exception &error)
    {
        cout << "Ошибка загрузки: " << error.what() << endl;
        return false;
    }

    if (getTotalCount() != 10)
    {
        cout << "Ошибка: ожидалось 10 буферов, загружено "
             << getTotalCount() << endl;

        return false;
    }

    return true;
}

/**
 * @brief Выводит содержимое всех загруженных буферов.
 *
 * Буферы отображаются последовательно в порядке групп: int/5, double/5,
 * float/7, int/10 и double/10. Номер выводимого буфера общий для всех групп.
 */
void showAllBuffers()
{
    cout << "\n===== ВСЕ БУФЕРЫ =====\n";

    size_t number = 1;

    for (size_t i = 0; i < intCount5; i++)
    {
        cout << "\nБуфер " << number++ << ": ";
        printBuffer(intBuffers5[i]);
        cout << endl;
    }

    for (size_t i = 0; i < doubleCount5; i++)
    {
        cout << "\nБуфер " << number++ << ": ";
        printBuffer(doubleBuffers5[i]);
        cout << endl;
    }

    for (size_t i = 0; i < floatCount7; i++)
    {
        cout << "\nБуфер " << number++ << ": ";
        printBuffer(floatBuffers7[i]);
        cout << endl;
    }

    for (size_t i = 0; i < intCount10; i++)
    {
        cout << "\nБуфер " << number++ << ": ";
        printBuffer(intBuffers10[i]);
        cout << endl;
    }

    for (size_t i = 0; i < doubleCount10; i++)
    {
        cout << "\nБуфер " << number++ << ": ";
        printBuffer(doubleBuffers10[i]);
        cout << endl;
    }
}

/**
 * @brief Запрашивает у пользователя номер буфера и проверяет его.
 * @param index Переменная, в которую записывается выбранный номер (от 1).
 * @return true, если введено целое число в допустимом диапазоне; иначе false.
 * @note При ошибке формата ввода состояние cin очищается, остаток строки
 * удаляется, чтобы последующие запросы ввода могли работать корректно.
 */
bool selectBuffer(size_t &index)
{
    size_t total = getTotalCount();

    cout << "Введите номер буфера (1-" << total << "): ";

    if (!(cin >> index))
    {
        cin.clear();
        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n');

        cout << "Некорректный ввод!\n";
        return false;
    }

    if (index < 1 || index > total)
    {
        cout << "Неверный номер буфера!\n";
        return false;
    }

    return true;
}

/**
 * @brief Считывает и добавляет значение в выбранный массив буферов.
 *
 * Тип значения T определяется типом выбранного массива. Если буфер уже
 * заполнен, его метод push заменит самый старый элемент новым.
 *
 * @tparam T Тип элементов буферов в массиве.
 * @tparam Capacity Вместимость каждого буфера массива.
 * @param buffers Массив буферов нужного типа.
 * @param index Индекс объекта внутри массива (нумерация с нуля).
 */
template <typename T, size_t Capacity>
void addToBuffer(
    RingBuffer<T, Capacity> buffers[],
    size_t index)
{
    T value;

    if (!(cin >> value))
    {
        cin.clear();
        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n');

        cout << "Некорректное значение!\n";
        return;
    }

    buffers[index].push(value);

    cout << "Элемент успешно добавлен!\n";

    cout << "Содержимое буфера: ";
    printBuffer(buffers[index]);
    cout << endl;
}

/**
 * @brief Выбирает один из десяти буферов и добавляет в него элемент.
 *
 * Пользователь вводит общий номер буфера. По счетчикам групп общий номер
 * преобразуется в индекс конкретного массива, после чего вызывается
 * шаблонная функция addToBuffer соответствующего типа.
 */
void addElement()
{
    size_t index;

    if (!selectBuffer(index))
    {
        return;
    }

    cout << "Введите значение: ";

    if (index <= intCount5)
    {
        addToBuffer(intBuffers5, index - 1);
    }
    else if (index <= intCount5 + doubleCount5)
    {
        addToBuffer(
            doubleBuffers5,
            index - intCount5 - 1);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7)
    {
        addToBuffer(
            floatBuffers7,
            index - intCount5 - doubleCount5 - 1);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7 + intCount10)
    {
        addToBuffer(
            intBuffers10,
            index - intCount5 - doubleCount5 - floatCount7 - 1);
    }
    else
    {
        addToBuffer(
            doubleBuffers10,
            index - intCount5 - doubleCount5 - floatCount7 - intCount10 - 1);
    }
}

/**
 * @brief Удаляет первый элемент выбранного буфера и показывает результат.
 * @tparam T Тип элементов буферов.
 * @tparam Capacity Вместимость каждого буфера массива.
 * @param buffers Массив буферов одного типа и вместимости.
 * @param index Индекс выбранного объекта в массиве, начиная с нуля.
 * @note Если буфер пуст, исключение из pop перехватывается и выводится
 * пользователю; программа продолжает работу.
 */
template <typename T, size_t Capacity>
void removeFromBuffer(
    RingBuffer<T, Capacity> buffers[],
    size_t index)
{
    try
    {
        buffers[index].pop();

        cout << "Первый элемент удалён!\n";

        cout << "Содержимое буфера: ";
        printBuffer(buffers[index]);
        cout << endl;
    }
    catch (const exception &error)
    {
        cout << "Ошибка: " << error.what() << endl;
    }
}

/**
 * @brief Запрашивает номер буфера и удаляет из него самый старый элемент.
 *
 * Общий номер преобразуется в индекс одного из пяти массивов. Само удаление
 * выполняется шаблонной функцией removeFromBuffer.
 */
void removeElement()
{
    size_t index;

    if (!selectBuffer(index))
    {
        return;
    }

    if (index <= intCount5)
    {
        removeFromBuffer(intBuffers5, index - 1);
    }
    else if (index <= intCount5 + doubleCount5)
    {
        removeFromBuffer(
            doubleBuffers5,
            index - intCount5 - 1);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7)
    {
        removeFromBuffer(
            floatBuffers7,
            index - intCount5 - doubleCount5 - 1);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7 + intCount10)
    {
        removeFromBuffer(
            intBuffers10,
            index - intCount5 - doubleCount5 - floatCount7 - 1);
    }
    else
    {
        removeFromBuffer(
            doubleBuffers10,
            index - intCount5 - doubleCount5 - floatCount7 - intCount10 - 1);
    }
}

/**
 * @brief Показывает первый и последний элементы переданного буфера.
 * @tparam T Тип хранимых элементов.
 * @tparam Capacity Вместимость буфера.
 * @param buffer Ссылка на буфер, из которого нужно получить крайние элементы.
 * @note Если буфер пуст, front/back выбрасывают исключение, которое
 * перехватывается и выводится как сообщение об ошибке.
 */
template <typename T, size_t Capacity>
void showBufferFrontBack(
    RingBuffer<T, Capacity> &buffer)
{
    try
    {
        cout << "Первый элемент: ";
        printValue(buffer.front());

        cout << "\nПоследний элемент: ";
        printValue(buffer.back());

        cout << endl;
    }
    catch (const exception &error)
    {
        cout << "Ошибка: " << error.what() << endl;
    }
}

/**
 * @brief Выбирает буфер по общему номеру и выводит его крайние элементы.
 *
 * После проверки номера определяет соответствующий массив и передает
 * выбранный объект функции showBufferFrontBack.
 */
void showFrontBack()
{
    size_t index;

    if (!selectBuffer(index))
    {
        return;
    }

    if (index <= intCount5)
    {
        showBufferFrontBack(intBuffers5[index - 1]);
    }
    else if (index <= intCount5 + doubleCount5)
    {
        showBufferFrontBack(
            doubleBuffers5[index - intCount5 - 1]);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7)
    {
        showBufferFrontBack(
            floatBuffers7[index - intCount5 - doubleCount5 - 1]);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7 + intCount10)
    {
        showBufferFrontBack(
            intBuffers10[index - intCount5 - doubleCount5 - floatCount7 - 1]);
    }
    else
    {
        showBufferFrontBack(
            doubleBuffers10[index - intCount5 - doubleCount5 - floatCount7 - intCount10 - 1]);
    }
}

/**
 * @brief Вычисляет и выводит среднее значение элементов буфера.
 * @tparam T Арифметический тип элементов.
 * @tparam Capacity Вместимость буфера.
 * @param buffer Константная ссылка на буфер для вычисления среднего.
 * @note Функция calculateAverage требует арифметический тип и непустой буфер.
 * Исключения перехватываются здесь, чтобы не завершать программу.
 */
template <typename T, size_t Capacity>
void showBufferAverage(
    const RingBuffer<T, Capacity> &buffer)
{
    try
    {
        cout << "Среднее значение: "
             << calculateAverage(buffer)
             << endl;
    }
    catch (const exception &error)
    {
        cout << "Ошибка: " << error.what() << endl;
    }
}

/**
 * @brief Запрашивает номер буфера и выводит среднее арифметическое его элементов.
 *
 * По общему номеру определяется тип и вместимость буфера, затем выбранный
 * объект передается в шаблонную функцию showBufferAverage.
 */
void showAverage()
{
    size_t index;

    if (!selectBuffer(index))
    {
        return;
    }

    if (index <= intCount5)
    {
        showBufferAverage(intBuffers5[index - 1]);
    }
    else if (index <= intCount5 + doubleCount5)
    {
        showBufferAverage(
            doubleBuffers5[index - intCount5 - 1]);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7)
    {
        showBufferAverage(
            floatBuffers7[index - intCount5 - doubleCount5 - 1]);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7 + intCount10)
    {
        showBufferAverage(
            intBuffers10[index - intCount5 - doubleCount5 - floatCount7 - 1]);
    }
    else
    {
        showBufferAverage(
            doubleBuffers10[index - intCount5 - doubleCount5 - floatCount7 - intCount10 - 1]);
    }
}

/**
 * @brief Показывает вместимость и текущую заполненность одного буфера.
 * @tparam T Тип элементов буфера.
 * @tparam Capacity Максимальное количество элементов.
 * @param buffer Константная ссылка на выбранный буфер.
 * @note Capacity известна на этапе компиляции, а buffer.size() возвращает
 * фактическое количество хранящихся в данный момент элементов.
 */
template <typename T, size_t Capacity>
void showBufferCapacity(const RingBuffer<T, Capacity> &buffer)
{
    cout << "Вместимость буфера: "
         << Capacity << endl;

    cout << "Количество элементов: "
         << buffer.size() << "/" << Capacity << endl;
}

/**
 * @brief Запрашивает номер буфера и выводит его вместимость.
 *
 * Общий номер преобразуется в индекс соответствующего массива. Для
 * выбранного объекта вызывается showBufferCapacity.
 */
void showCapacity()
{
    size_t index;

    if (!selectBuffer(index))
    {
        return;
    }

    if (index <= intCount5)
    {
        showBufferCapacity(intBuffers5[index - 1]);
    }
    else if (index <= intCount5 + doubleCount5)
    {
        showBufferCapacity(doubleBuffers5[index - intCount5 - 1]);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7)
    {
        showBufferCapacity(
            floatBuffers7[index - intCount5 - doubleCount5 - 1]);
    }
    else if (index <= intCount5 + doubleCount5 + floatCount7 + intCount10)
    {
        showBufferCapacity(
            intBuffers10[index - intCount5 - doubleCount5 - floatCount7 - 1]);
    }
    else
    {
        showBufferCapacity(
            doubleBuffers10[index - intCount5 - doubleCount5 - floatCount7 - intCount10 - 1]);
    }
}

/**
 * @brief Выводит список доступных пользователю команд.
 *
 * Функция только отображает пункты меню и приглашение для ввода;
 * обработка выбранного пункта выполняется в main.
 */
void showMenu()
{
    cout << "\n========== КОЛЬЦЕВОЙ БУФЕР ==========\n";

    cout << "1. Показать все буферы\n";
    cout << "2. Добавить элемент\n";
    cout << "3. Удалить первый элемент\n";
    cout << "4. Показать первый и последний элементы\n";
    cout << "5. Вычислить среднее значение\n";
    cout << "6. Показать вместимость выбранного буфера\n";
    cout << "0. Выход\n";

    cout << "Выберите действие: ";
}

/**
 * @brief Точка входа в программу.
 *
 * Сначала загружает буферы из файла. Если загрузка прошла успешно,
 * запускает цикл меню и выполняет выбранные пользователем операции
 * до ввода команды выхода. Некорректный ввод очищается и запрашивается
 * повторно.
 *
 * @return 0 при штатном завершении; 1, если не удалось загрузить данные.
 */
int main()
{
    if (!loadData())
    {
        cout << "Не удалось загрузить буферы!\n";
        return 1;
    }

    cout << "Успешно загружено "
         << getTotalCount()
         << " буферов!\n";

    int choice = -1;

    do
    {
        showMenu();

        if (!(cin >> choice))
        {
            cin.clear();

            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n');

            cout << "Некорректный ввод!\n";
            continue;
        }

        switch (choice)
        {
        case 1:
            showAllBuffers();
            break;

        case 2:
            addElement();
            break;

        case 3:
            removeElement();
            break;

        case 4:
            showFrontBack();
            break;

        case 5:
            showAverage();
            break;

        case 6:
            showCapacity();
            break;

        case 0:
            cout << "Программа завершена.\n";
            break;

        default:
            cout << "Неизвестный пункт меню!\n";
            break;
        }

    } while (choice != 0);

    return 0;
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <string>
#include <stdexcept>

#include "RingBuffer.h"
#include "functions.h"

using namespace std;

// Массивы кольцевых буферов
RingBuffer<int, 5> intBuffers5[10];
RingBuffer<double, 5> doubleBuffers5[10];
RingBuffer<float, 7> floatBuffers7[10];
RingBuffer<int, 10> intBuffers10[10];
RingBuffer<double, 10> doubleBuffers10[10];

// Количество загруженных буферов каждого типа
size_t intCount5 = 0;
size_t doubleCount5 = 0;
size_t floatCount7 = 0;
size_t intCount10 = 0;
size_t doubleCount10 = 0;

// Общее количество буферов
size_t getTotalCount()
{
    return intCount5 +
           doubleCount5 +
           floatCount7 +
           intCount10 +
           doubleCount10;
}

// Загрузка одного буфера из файла
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

// Загрузка 10 буферов из файла
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

// Вывод всех буферов
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

int main()
{
    if (!loadData())
        return 1;

    cout << "Успешно загружено " << getTotalCount() << " буферов!\n";
    showAllBuffers();
    return 0;
}

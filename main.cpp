#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdexcept>

template<typename T>
long sizeInBytes(const T& variable) {
    return sizeof(variable);
}

template<>
long sizeInBytes<std::string>(const std::string& str) {
    return sizeof(str[0]) * str.size();
};

template<typename T>
class ListNode {
public:
    // Конструктор
    ListNode() : nextPointer(-1), size(0), is_delete(false) {}

    // Конструктор копирования
    ListNode(const ListNode& obj)
    {
        is_delete = obj.is_delete;
        size = obj.size;
        data = obj.data;
        nextPointer = obj.nextPointer;
    }

    // Деструктор
    ~ListNode() {}

    // Перегрузка оператора << для записи объекта в поток
    friend std::ostream& operator<<(std::ostream& os, const ListNode<T>& node)
    {
        os.write(reinterpret_cast<const char*>(&node.is_delete), sizeof(bool));
        os.write(reinterpret_cast<const char*>(&node.size), sizeof(long));
        os.write(node.data.c_str(), node.size);
        os.write(reinterpret_cast<const char*>(&node.nextPointer), sizeof(long));
        return os;
    }

    // Перегрузка оператора >> для чтения объекта из потока
    friend std::istream& operator>>(std::istream& is, ListNode<T>& node)
    {
        is.read(reinterpret_cast<char*>(&node.is_delete), sizeof(bool));
        is.read(reinterpret_cast<char*>(&node.size), sizeof(long));
        std::string bufer;
        bufer.resize(node.size);
        is.read(&bufer[0], node.size);
        node.data = bufer;
        is.read(reinterpret_cast<char*>(&node.nextPointer), sizeof(long));
        return is;
    }

    T get_data() { return data; } // Получение данных
    void set_data(T obj) { data = obj; } // Изменение данных
    long get_size() { return size; } // Получение размера данных
    void set_size(T obj) { size = sizeInBytes(obj); } // Изменение размера данных
    long get_nextPointer() { return nextPointer; } // Получение ссылки на следующий элемент
    void set_nextPointer(long obj) { nextPointer = obj; } // Изменение ссылки на следующий элемент

    // Вывод всех полей
    void print_all()
    {
        std::cout << is_delete << std::endl;
        std::cout << size << std::endl;
        std::cout << data << std::endl;
        std::cout << nextPointer << std::endl << std::endl << std::endl;
    }

private:
    bool is_delete; // Проверка на удаление
    long size; // длинна хранения данных (чтоб стоки норм хранились)
    T data; // данные
    long nextPointer; // Ссылка на следующий элемент
};

template<typename T>
class BinaryFile {
public:
    // Конструктор
    BinaryFile(const char* filename) : file(filename, std::ios::binary | std::ios::in | std::ios::out)
    {

        //nums = 0; // Элементов нет
        this->filename = filename; // Копируем заданное название
        if (!file.is_open()) // Создание файла, если он не существует
        {
            file.open(filename, std::ios::binary | std::ios::out);
            file.close();
            file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
            headPointer = -1; // По умолчанию первого элемента нет
            file.seekg(0, std::ios::beg);// Устанавливаем указатель на начало файла
        }
        else
        {
            file.seekg(0, std::ios::beg);// Устанавливаем указатель на начало файла
            file.read(reinterpret_cast<char*>(&headPointer), sizeof(long));
        }
    }

    // Деструктор
    ~BinaryFile() { file.close(); }

    // Добавление элемента в конец списка
    void addObjectToPos(const T& obj, int index)
    {
        file.clear(); // Очищаем от ссылок
        ListNode<T> node;
        node.set_data(obj); // Задаём данные
        node.set_size(obj); // Задаём размер данных

        if (index == 0) // Если это первый элемент
        {
            file.seekp(0, std::ios::end);
            long currentPosition = file.tellp(); // Запоминаем позицию конца файла
            node.set_nextPointer(headPointer);
            file << node; // Используем оператор << для записи элемента списка в файл
            headPointer = currentPosition;
            file.seekg(0, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&headPointer), sizeof(long));
            //nums++;
        }
        else
        {
            file.clear(); // Очистка файла от ссылок
            file.seekg(0, std::ios::beg); // Ищем первый элемент
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка

            //long prevPosition = head; // Позиция конца предыдущего элемента
            //long currentPosition = head; // Позиция начла удаляемого элемента
            long prevNextPosition = head; // Позиция конца удаляемого элемента

            ListNode<T> currentNode;
            for (int i = 0; i < index; i++) // Находим элемент, предшествующий индексу вставки
            {
                if (file.eof() || file.tellg() == -1) {
                    std::cerr << "Index out of bounds" << std::endl;
                    return;
                }
                file >> currentNode;
                prevNextPosition = file.tellp();
                prevNextPosition -= sizeof(long);
                file.seekg(currentNode.get_nextPointer(), std::ios::beg);
            }
            file.clear();
            if (currentNode.get_nextPointer() == -1)
            {
                file.seekp(0, std::ios::end);
                long currentPosition = file.tellp(); // Запоминаем текущую позицию
                file << node; // Используем оператор << для записи элемента списка в файл
                file.seekp(prevNextPosition, std::ios::beg);
                file.write(reinterpret_cast<const char*>(&currentPosition), sizeof(long));
                //nums++;
                return;
            }
            long NextPosition = file.tellp(); // Старая ссылка предшествующего элемента
            file.seekp(0, std::ios::end);
            long currentPosition = file.tellp(); // Запоминаем позицию конца файла
            node.set_nextPointer(NextPosition);
            file << node; // Используем оператор << для записи элемента списка в файл
            file.seekp(prevNextPosition, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&currentPosition), sizeof(long));
            //nums++;
        }
    }

    // Добавление по индексу
    void addObject(const T& obj)
    {
        file.clear();
        ListNode<T> node;
        node.set_data(obj);
        node.set_size(obj);

        if (headPointer != -1) {
            file.seekg(0, std::ios::beg); // Ищем первый элемент
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка
            ListNode<T> currentNode;
            long prevNextPosition = -1;
            // Находим последний элемент в списке
            while (!file.eof() && file.tellg() != -1)
            {
                file >> currentNode;
                prevNextPosition = file.tellp();
                prevNextPosition -= sizeof(long);
                file.seekg(currentNode.get_nextPointer(), std::ios::beg);
            }
            file.clear();
            file.seekp(0, std::ios::end);
            long currentPosition = file.tellp(); // Запоминаем текущую позицию
            file << node; // Используем оператор << для записи элемента списка в файл
            file.seekp(prevNextPosition, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&currentPosition), sizeof(long));
            //nums++;
        }
        if (headPointer == -1) {
            // Если это первый элемент, обновляем указатель на начало списка
            headPointer = 4;
            file.seekg(0, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&headPointer), sizeof(long));
            file << node; // Используем оператор << для записи элемента списка в файл
            //nums++;
        }
    }

    // Удаление по индексу
    void deleteObject(int index)
    {
        file.clear();
        file.seekg(0, std::ios::beg);
        ListNode<T> deleteNode;

        if (index == 0)
        {
            file.clear();
            if (file.eof() || file.tellg() == -1) {
                std::cerr << "Index out of bounds" << std::endl;
                return;
            }
            file.seekg(0, std::ios::beg); // Ищем первый элемент
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка
            bool is_delete = true;
            file.write(reinterpret_cast<const char*>(&is_delete), sizeof(bool));
            file.seekg(head, std::ios::beg);
            file >> deleteNode;
            long deletenextPointer = deleteNode.get_nextPointer();
            file.seekg(0, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&deletenextPointer), sizeof(long));
            headPointer = deletenextPointer;
            //nums--;
        }
        else
        {
            file.clear(); // Очистка файла от ссылок
            file.seekg(0, std::ios::beg); // Ищем первый элемент
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка
            long prevPosition = head; // Позиция конца предыдущего элемента
            long currentPosition = head; // Позиция начла удаляемого элемента
            long prevNextPosition = head; // Позиция конца удаляемого элемента
            ListNode<T> currentNode;
            for (int i = 0; i <= index; i++) // Находим элемент, предшествующий индексу вставки
            {
                if (file.eof() || file.tellg() == -1) {
                    std::cerr << "Index out of bounds" << std::endl;
                    return;
                }
                prevPosition = prevNextPosition;
                currentPosition = file.tellg();
                file >> currentNode;
                prevNextPosition = file.tellg();
                file.seekg(currentNode.get_nextPointer(), std::ios::beg);
            }
            file.clear();
            file.seekp(currentPosition, std::ios::beg); // Ставим на начало перед удаляемым
            bool is_delete = true;
            file.write(reinterpret_cast<const char*>(&is_delete), sizeof(bool));
            long newlastPointer = currentNode.get_nextPointer();
            file.seekp(prevPosition - sizeof(long), std::ios::beg);
            file.write(reinterpret_cast<const char*>(&newlastPointer), sizeof(long));
            //nums--;
        }
    }

    // Удаление последнего элемента
    void deleteLastObject()
    {
        file.clear();
        if (headPointer == -1) {
            std::cerr << "No elements to delete" << std::endl;
            return;
        }
        file.clear(); // Очистка файла от ссылок
        file.seekg(0, std::ios::beg); // Ищем первый элемент
        long head = -1;
        file.read(reinterpret_cast<char*>(&head), sizeof(long));
        file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка
        long prevPosition = head; // Позиция конца предыдущего элемента
        long currentPosition = head; // Позиция начла удаляемого элемента
        long prevNextPosition = head; // Позиция конца удаляемого элемента
        ListNode<T> currentNode; // Удаляемый элемент
        int num = -1;
        // Находим последний элемент в списке
        while (!file.eof() && file.tellg() != -1)
        {
            prevPosition = prevNextPosition;
            currentPosition = file.tellg();
            file >> currentNode;
            prevNextPosition = file.tellg();
            file.seekg(currentNode.get_nextPointer(), std::ios::beg);
            //num++;
        }
        file.clear();
        if (currentPosition == -1)
        {
            std::cerr << "Error finding last element" << std::endl;
            return;
        }

        if (num == 0) // Если в списке только один элемент
        {
            // Очищаем список
            file.seekp(currentPosition, std::ios::beg);
            bool is_delete = true;
            file.write(reinterpret_cast<const char*>(&is_delete), sizeof(bool));
            headPointer = -1;
            file.seekp(0, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&headPointer), sizeof(long));
            //nums--;
        }
        else // Обновляем ссылку предпоследнего элемента
        {
            file.seekp(currentPosition, std::ios::beg); // Ставим на начало перед удаляемым
            bool is_delete = true;
            file.write(reinterpret_cast<const char*>(&is_delete), sizeof(bool));
            file.seekp(prevPosition - sizeof(long), std::ios::beg);
            long newlastPointer = -1;
            file.write(reinterpret_cast<const char*>(&newlastPointer), sizeof(long));
            //nums--;
        }
    }

    // Сортировка
    void bubbleSort()
    {
        if (headPointer == -1) {
            std::cerr << "No elements" << std::endl;
            return;
        }
        file.clear(); // Очистка файла от ссылок
        ListNode<T> currentNode;
        file.seekg(0, std::ios::beg); // Ищем первый элемент
        long head = -1;
        file.read(reinterpret_cast<char*>(&head), sizeof(long));

        file >> currentNode;
        if (file.eof()) return;


        file.clear(); // Очистка файла от ссылок
        file.seekg(0, std::ios::beg); // Ищем первый элемент
        head = -1;
        file.read(reinterpret_cast<char*>(&head), sizeof(long));


        int nums = 0;
        while (!file.eof() && file.tellg() != -1)
        {
            file >> currentNode;
            file.seekg(currentNode.get_nextPointer(), std::ios::beg);
            nums++;
        }
        file.clear();
        file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка

        for (int i = 0; i < nums; i++)
        {
            file.clear();
            for (int j = 1; j < nums; j++)
            {
                file.clear();
                file.seekg(0, std::ios::beg); // Ищем первый элемент
                head = -1;
                file.read(reinterpret_cast<char*>(&head), sizeof(long));
                file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка
                ListNode<T> firstNode;
                ListNode<T> secondNode;

                long prevsecond = 0;
                long firstNodeStart = head;
                long firstNodeEnd = 0;
                long secondNodeStart = head;
                long secondNodeEnd = 0;
                int k = 0;
                while (k <= j)
                {
                    if (k > 1) prevsecond = secondNodeEnd - sizeof(long);

                    secondNode = firstNode;
                    secondNodeStart = firstNodeStart;
                    secondNodeEnd = firstNodeEnd;
                    firstNodeStart = file.tellg();
                    file >> firstNode;
                    firstNodeEnd = file.tellg();
                    file.seekg(firstNode.get_nextPointer(), std::ios::beg);
                    k++;
                }
                file.clear();
                if (firstNode.get_data() < secondNode.get_data())
                {
                    long pos = firstNode.get_nextPointer();
                    file.seekp(secondNodeEnd - sizeof(long), std::ios::beg);
                    file.write(reinterpret_cast<const char*>(&pos), sizeof(long));

                    file.seekp(prevsecond, std::ios::beg);
                    file.write(reinterpret_cast<const char*>(&firstNodeStart), sizeof(long));

                    file.seekp(firstNodeEnd - sizeof(long), std::ios::beg);
                    file.write(reinterpret_cast<const char*>(&secondNodeStart), sizeof(long));
                }
            }
        }
    }

    // Метод для обновления файла
    void update()
    {
        if (headPointer == -1) {
            std::cerr << "No elements" << std::endl;
            return;
        }
        file.clear();
        try {
            // Открыть новый бинарный файл для записи
            const char* newFileName = "data1.bin";
            std::fstream newFile(newFileName, std::ios::binary);
            if (!newFile.is_open()) {
                // Создание файла, если он не существует
                newFile.open(newFileName, std::ios::binary | std::ios::out);
                newFile.close();
                newFile.open(newFileName, std::ios::binary | std::ios::in | std::ios::out);
            }

            // Считать данные из старого файла и записать их в новый файл
            file.seekg(0, std::ios::beg); // Ищем первый элемент
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка
            newFile.seekg(0, std::ios::beg);
            long newhead = 4;
            newFile.write(reinterpret_cast<const char*>(&newhead), sizeof(long));
            ListNode<T> currentNode;
            while (!file.eof() && file.tellg() != -1)
            {
                file >> currentNode;
                file.seekg(currentNode.get_nextPointer(), std::ios::beg);

                long prevPosition = newFile.tellg();
                prevPosition -= sizeof(long);
                long currentPosition = newFile.tellg();
                newFile.seekg(prevPosition, std::ios::beg);
                newFile.write(reinterpret_cast<const char*>(&currentPosition), sizeof(long));
                currentNode.set_nextPointer(-1);
                newFile << currentNode;
            }
            file.clear();
            newFile.clear();

            // Закрыть оба файла
            file.close();
            newFile.close();

            // Удаляем старый файл
            if (std::remove(filename) != 0) {
                throw std::runtime_error("Failed to delete old binary file.");
            }

            // Переименовываем новый файл
            if (std::rename(newFileName, filename) != 0) {
                throw std::runtime_error("Failed to rename new binary.");
            }

            // Открываем текущий файл в режиме чтения и записи
            file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
            if (!file.is_open()) {
                throw std::runtime_error("The current binary file could not be opened for reading or writing.");
            }
            headPointer = 4;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    // Метод для просмотра объектов
    void viewObjects()
    {
        if (headPointer == -1) {
            std::cerr << "No elements" << std::endl;
            return;
        }
        file.clear(); // Очистка файла от ссылок
        file.seekg(0, std::ios::beg); // Ищем первый элемент
        long head = -1;
        file.read(reinterpret_cast<char*>(&head), sizeof(long));
        file.seekg(head, std::ios::beg); // Перемещаем указатель в начало списка
        while (!file.eof() && file.tellg() != -1)
        {
            ListNode<T> node;
            file >> node; // Используем оператор >> для чтения элемента списка из файла
            file.seekp(node.get_nextPointer(), std::ios::beg); // Перемещяем указатель на следующий элемент
            if (!file.eof())
            {
                //node.print_all();
                std::cout << node.get_data() << std::endl;
            }
        }
        //std::cout << nums << std::endl;
        std::cout << std::endl;
    }

private:
    std::fstream file; // Файл для списка
    long headPointer; // Указатель на начало списка
    //int nums; // Число элементов
    const char* filename; // Имя файла
};

void menu(BinaryFile<std::string>& binFile, int& num)
{
    setlocale(LC_ALL, "Russian");

    int position{ 0 };
    std::string value{ "" };
    std::cout << "1 - Добавление в конец файла." << std::endl;
    std::cout << "2 - Добавление по индексу." << std::endl;
    std::cout << "3 - Удаление с конца файла." << std::endl;
    std::cout << "4 - Удаление по индексу." << std::endl;
    std::cout << "5 - Сортировка." << std::endl;
    std::cout << "6 - Обновление." << std::endl;
    std::cout << "7 - Просмотр." << std::endl;
    std::cout << "0 - для выхода" << std::endl;
    std::cout << "Введите число: " << std::endl;
    std::cin >> num;
    switch (num)
    {
    case 1:
        std::cout << "Введите данные: ";
        std::cin >> value;
        binFile.addObject(value);
        break;
    case 2:
        std::cout << "Введите позицию: ";
        std::cin >> position;
        std::cout << "Введите данные: ";
        std::cin >> value;
        binFile.addObjectToPos(value, position);
        break;
    case 3:
        binFile.deleteLastObject();
        break;
    case 4:
        std::cout << "Введите позицию: ";
        std::cin >> position;
        binFile.deleteObject(position);
        break;
    case 5:
        binFile.bubbleSort();
        break;
    case 6:
        binFile.update();
        break;
    case 7:
        std::cout << std::endl;
        binFile.viewObjects();
        break;
    case 0:
        break;
    default:
        std::cout << "Неизвестная команда!" << std::endl;
        break;
    }
}

int main()
{
    BinaryFile<std::string> binFile("data.bin");
    int num = -1;
    do
    {
        menu(binFile, num);
    } while (num != 0);
    return 0;
}
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
    // �����������
    ListNode() : nextPointer(-1), size(0), is_delete(false) {}

    // ����������� �����������
    ListNode(const ListNode& obj)
    {
        is_delete = obj.is_delete;
        size = obj.size;
        data = obj.data;
        nextPointer = obj.nextPointer;
    }

    // ����������
    ~ListNode() {}

    // ���������� ��������� << ��� ������ ������� � �����
    friend std::ostream& operator<<(std::ostream& os, const ListNode<T>& node)
    {
        os.write(reinterpret_cast<const char*>(&node.is_delete), sizeof(bool));
        os.write(reinterpret_cast<const char*>(&node.size), sizeof(long));
        os.write(node.data.c_str(), node.size);
        os.write(reinterpret_cast<const char*>(&node.nextPointer), sizeof(long));
        return os;
    }

    // ���������� ��������� >> ��� ������ ������� �� ������
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

    T get_data() { return data; } // ��������� ������
    void set_data(T obj) { data = obj; } // ��������� ������
    long get_size() { return size; } // ��������� ������� ������
    void set_size(T obj) { size = sizeInBytes(obj); } // ��������� ������� ������
    long get_nextPointer() { return nextPointer; } // ��������� ������ �� ��������� �������
    void set_nextPointer(long obj) { nextPointer = obj; } // ��������� ������ �� ��������� �������

    // ����� ���� �����
    void print_all()
    {
        std::cout << is_delete << std::endl;
        std::cout << size << std::endl;
        std::cout << data << std::endl;
        std::cout << nextPointer << std::endl << std::endl << std::endl;
    }

private:
    bool is_delete; // �������� �� ��������
    long size; // ������ �������� ������ (���� ����� ���� ���������)
    T data; // ������
    long nextPointer; // ������ �� ��������� �������
};

template<typename T>
class BinaryFile {
public:
    // �����������
    BinaryFile(const char* filename) : file(filename, std::ios::binary | std::ios::in | std::ios::out)
    {

        //nums = 0; // ��������� ���
        this->filename = filename; // �������� �������� ��������
        if (!file.is_open()) // �������� �����, ���� �� �� ����������
        {
            file.open(filename, std::ios::binary | std::ios::out);
            file.close();
            file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
            headPointer = -1; // �� ��������� ������� �������� ���
            file.seekg(0, std::ios::beg);// ������������� ��������� �� ������ �����
        }
        else
        {
            file.seekg(0, std::ios::beg);// ������������� ��������� �� ������ �����
            file.read(reinterpret_cast<char*>(&headPointer), sizeof(long));
        }
    }

    // ����������
    ~BinaryFile() { file.close(); }

    // ���������� �������� � ����� ������
    void addObjectToPos(const T& obj, int index)
    {
        file.clear(); // ������� �� ������
        ListNode<T> node;
        node.set_data(obj); // ����� ������
        node.set_size(obj); // ����� ������ ������

        if (index == 0) // ���� ��� ������ �������
        {
            file.seekp(0, std::ios::end);
            long currentPosition = file.tellp(); // ���������� ������� ����� �����
            node.set_nextPointer(headPointer);
            file << node; // ���������� �������� << ��� ������ �������� ������ � ����
            headPointer = currentPosition;
            file.seekg(0, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&headPointer), sizeof(long));
            //nums++;
        }
        else
        {
            file.clear(); // ������� ����� �� ������
            file.seekg(0, std::ios::beg); // ���� ������ �������
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������

            //long prevPosition = head; // ������� ����� ����������� ��������
            //long currentPosition = head; // ������� ����� ���������� ��������
            long prevNextPosition = head; // ������� ����� ���������� ��������

            ListNode<T> currentNode;
            for (int i = 0; i < index; i++) // ������� �������, �������������� ������� �������
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
                long currentPosition = file.tellp(); // ���������� ������� �������
                file << node; // ���������� �������� << ��� ������ �������� ������ � ����
                file.seekp(prevNextPosition, std::ios::beg);
                file.write(reinterpret_cast<const char*>(&currentPosition), sizeof(long));
                //nums++;
                return;
            }
            long NextPosition = file.tellp(); // ������ ������ ��������������� ��������
            file.seekp(0, std::ios::end);
            long currentPosition = file.tellp(); // ���������� ������� ����� �����
            node.set_nextPointer(NextPosition);
            file << node; // ���������� �������� << ��� ������ �������� ������ � ����
            file.seekp(prevNextPosition, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&currentPosition), sizeof(long));
            //nums++;
        }
    }

    // ���������� �� �������
    void addObject(const T& obj)
    {
        file.clear();
        ListNode<T> node;
        node.set_data(obj);
        node.set_size(obj);

        if (headPointer != -1) {
            file.seekg(0, std::ios::beg); // ���� ������ �������
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������
            ListNode<T> currentNode;
            long prevNextPosition = -1;
            // ������� ��������� ������� � ������
            while (!file.eof() && file.tellg() != -1)
            {
                file >> currentNode;
                prevNextPosition = file.tellp();
                prevNextPosition -= sizeof(long);
                file.seekg(currentNode.get_nextPointer(), std::ios::beg);
            }
            file.clear();
            file.seekp(0, std::ios::end);
            long currentPosition = file.tellp(); // ���������� ������� �������
            file << node; // ���������� �������� << ��� ������ �������� ������ � ����
            file.seekp(prevNextPosition, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&currentPosition), sizeof(long));
            //nums++;
        }
        if (headPointer == -1) {
            // ���� ��� ������ �������, ��������� ��������� �� ������ ������
            headPointer = 4;
            file.seekg(0, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&headPointer), sizeof(long));
            file << node; // ���������� �������� << ��� ������ �������� ������ � ����
            //nums++;
        }
    }

    // �������� �� �������
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
            file.seekg(0, std::ios::beg); // ���� ������ �������
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������
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
            file.clear(); // ������� ����� �� ������
            file.seekg(0, std::ios::beg); // ���� ������ �������
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������
            long prevPosition = head; // ������� ����� ����������� ��������
            long currentPosition = head; // ������� ����� ���������� ��������
            long prevNextPosition = head; // ������� ����� ���������� ��������
            ListNode<T> currentNode;
            for (int i = 0; i <= index; i++) // ������� �������, �������������� ������� �������
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
            file.seekp(currentPosition, std::ios::beg); // ������ �� ������ ����� ���������
            bool is_delete = true;
            file.write(reinterpret_cast<const char*>(&is_delete), sizeof(bool));
            long newlastPointer = currentNode.get_nextPointer();
            file.seekp(prevPosition - sizeof(long), std::ios::beg);
            file.write(reinterpret_cast<const char*>(&newlastPointer), sizeof(long));
            //nums--;
        }
    }

    // �������� ���������� ��������
    void deleteLastObject()
    {
        file.clear();
        if (headPointer == -1) {
            std::cerr << "No elements to delete" << std::endl;
            return;
        }
        file.clear(); // ������� ����� �� ������
        file.seekg(0, std::ios::beg); // ���� ������ �������
        long head = -1;
        file.read(reinterpret_cast<char*>(&head), sizeof(long));
        file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������
        long prevPosition = head; // ������� ����� ����������� ��������
        long currentPosition = head; // ������� ����� ���������� ��������
        long prevNextPosition = head; // ������� ����� ���������� ��������
        ListNode<T> currentNode; // ��������� �������
        int num = -1;
        // ������� ��������� ������� � ������
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

        if (num == 0) // ���� � ������ ������ ���� �������
        {
            // ������� ������
            file.seekp(currentPosition, std::ios::beg);
            bool is_delete = true;
            file.write(reinterpret_cast<const char*>(&is_delete), sizeof(bool));
            headPointer = -1;
            file.seekp(0, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&headPointer), sizeof(long));
            //nums--;
        }
        else // ��������� ������ �������������� ��������
        {
            file.seekp(currentPosition, std::ios::beg); // ������ �� ������ ����� ���������
            bool is_delete = true;
            file.write(reinterpret_cast<const char*>(&is_delete), sizeof(bool));
            file.seekp(prevPosition - sizeof(long), std::ios::beg);
            long newlastPointer = -1;
            file.write(reinterpret_cast<const char*>(&newlastPointer), sizeof(long));
            //nums--;
        }
    }

    // ����������
    void bubbleSort()
    {
        if (headPointer == -1) {
            std::cerr << "No elements" << std::endl;
            return;
        }
        file.clear(); // ������� ����� �� ������
        ListNode<T> currentNode;
        file.seekg(0, std::ios::beg); // ���� ������ �������
        long head = -1;
        file.read(reinterpret_cast<char*>(&head), sizeof(long));

        file >> currentNode;
        if (file.eof()) return;


        file.clear(); // ������� ����� �� ������
        file.seekg(0, std::ios::beg); // ���� ������ �������
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
        file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������

        for (int i = 0; i < nums; i++)
        {
            file.clear();
            for (int j = 1; j < nums; j++)
            {
                file.clear();
                file.seekg(0, std::ios::beg); // ���� ������ �������
                head = -1;
                file.read(reinterpret_cast<char*>(&head), sizeof(long));
                file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������
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

    // ����� ��� ���������� �����
    void update()
    {
        if (headPointer == -1) {
            std::cerr << "No elements" << std::endl;
            return;
        }
        file.clear();
        try {
            // ������� ����� �������� ���� ��� ������
            const char* newFileName = "data1.bin";
            std::fstream newFile(newFileName, std::ios::binary);
            if (!newFile.is_open()) {
                // �������� �����, ���� �� �� ����������
                newFile.open(newFileName, std::ios::binary | std::ios::out);
                newFile.close();
                newFile.open(newFileName, std::ios::binary | std::ios::in | std::ios::out);
            }

            // ������� ������ �� ������� ����� � �������� �� � ����� ����
            file.seekg(0, std::ios::beg); // ���� ������ �������
            long head = -1;
            file.read(reinterpret_cast<char*>(&head), sizeof(long));
            file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������
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

            // ������� ��� �����
            file.close();
            newFile.close();

            // ������� ������ ����
            if (std::remove(filename) != 0) {
                throw std::runtime_error("Failed to delete old binary file.");
            }

            // ��������������� ����� ����
            if (std::rename(newFileName, filename) != 0) {
                throw std::runtime_error("Failed to rename new binary.");
            }

            // ��������� ������� ���� � ������ ������ � ������
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

    // ����� ��� ��������� ��������
    void viewObjects()
    {
        if (headPointer == -1) {
            std::cerr << "No elements" << std::endl;
            return;
        }
        file.clear(); // ������� ����� �� ������
        file.seekg(0, std::ios::beg); // ���� ������ �������
        long head = -1;
        file.read(reinterpret_cast<char*>(&head), sizeof(long));
        file.seekg(head, std::ios::beg); // ���������� ��������� � ������ ������
        while (!file.eof() && file.tellg() != -1)
        {
            ListNode<T> node;
            file >> node; // ���������� �������� >> ��� ������ �������� ������ �� �����
            file.seekp(node.get_nextPointer(), std::ios::beg); // ���������� ��������� �� ��������� �������
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
    std::fstream file; // ���� ��� ������
    long headPointer; // ��������� �� ������ ������
    //int nums; // ����� ���������
    const char* filename; // ��� �����
};

void menu(BinaryFile<std::string>& binFile, int& num)
{
    setlocale(LC_ALL, "Russian");

    int position{ 0 };
    std::string value{ "" };
    std::cout << "1 - ���������� � ����� �����." << std::endl;
    std::cout << "2 - ���������� �� �������." << std::endl;
    std::cout << "3 - �������� � ����� �����." << std::endl;
    std::cout << "4 - �������� �� �������." << std::endl;
    std::cout << "5 - ����������." << std::endl;
    std::cout << "6 - ����������." << std::endl;
    std::cout << "7 - ��������." << std::endl;
    std::cout << "0 - ��� ������" << std::endl;
    std::cout << "������� �����: " << std::endl;
    std::cin >> num;
    switch (num)
    {
    case 1:
        std::cout << "������� ������: ";
        std::cin >> value;
        binFile.addObject(value);
        break;
    case 2:
        std::cout << "������� �������: ";
        std::cin >> position;
        std::cout << "������� ������: ";
        std::cin >> value;
        binFile.addObjectToPos(value, position);
        break;
    case 3:
        binFile.deleteLastObject();
        break;
    case 4:
        std::cout << "������� �������: ";
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
        std::cout << "����������� �������!" << std::endl;
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
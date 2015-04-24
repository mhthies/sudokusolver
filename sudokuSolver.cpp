#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>

typedef std::vector<std::vector<int>> sudokuField;

sudokuField parseFile(std::istream& input) {
    sudokuField result;

    std::string line;
    while (std::getline(input, line)) {
        std::vector<int> v;
        int num = 0;
        for(char c: line) {
            if (c == ' ') {
                if (num > 0) {
                    v.push_back(num);
                    num = 0;
                }
            } else if (c == '-') {
                num = 0;
                v.push_back(0);
            } else if (c >= '0' && c <= '9') {
                num = num * 10 + (c - '0');
            } else if (c >= 'a' && c <= 'z') {
                num = c - 'a' + 10;
            } else if (c >= 'A' && c <= 'Z') {
                num = c - 'A' + 10;
            }
        }
        if (num > 0)
            v.push_back(num);
        
        if (v.size() > 0)
            result.push_back(v);
    }
    
    return result;
}

bool checkField(const sudokuField& field) {
    int size = field.size();
    
    if (std::sqrt(size) != floor(std::sqrt(size)))
        return false;
    
    for (std::vector<int> v: field) {
        if (v.size() != size)
            return false;
        
        for (int i: v) {
            if (i < 0 || i > size)
                return false;
        }
    }
    
    return true;
}

void printField(std::ostream& os, const sudokuField& field) {
    for (std::vector<int> v: field) {
        for (int x: v) {
            if (x == 0) {
                for(int i = 0; i < (int)std::log10(field.size()); i++)
                    os << ' ';
                os << "- ";
            } else {
                for(int i = std::log10(x); i < (int)std::log10(field.size()); i++)
                    os << ' ';
                os << x << ' ';
            }
        }
        os << std::endl;
    }
}

std::ostream& operator<< (std::ostream& os, sudokuField& field) {
    printField(os, field);
    return os;
}

bool solveSudoku (sudokuField& field) {
    int size = field.size();
    int sizeBase = std::sqrt(size);
    
    std::vector<bool> allNumbers;
    allNumbers.push_back(false);
    for (int i=0;i<size;++i)
        allNumbers.push_back(true);
    
    std::vector<int> possibleNumbers;
    int minPossible = size+1;
    int x,y;
    
    /* search empty cells */
    for (int i=0;i<size;++i) {
        for (int j=0;j<size;++j) {
            if (field[i][j] == 0) {
                std::vector<bool> numbers = allNumbers;
            
                /* Scan row, col and square */
                for (int k=0;k<size;k++)
                    numbers[field[i][k]] = false;
                for (int k=0;k<size;k++)
                    numbers[field[k][j]] = false;
                for (int k=(i/sizeBase)*sizeBase;k<(i/sizeBase+1)*sizeBase;k++)
                    for (int l=(j/sizeBase)*sizeBase;l<(j/sizeBase+1)*sizeBase;l++)
                        numbers[field[k][l]] = false;
                            
                /* Count possibilities */
                int possible = 0;
                for (bool p: numbers)
                    if (p) possible++;
                    
                /* New minimum */
                if (possible < minPossible) {
                    minPossible = possible;
                    possibleNumbers.clear();
                    for(int k=1;k<=size;k++)
                        if (numbers[k])
                            possibleNumbers.push_back(k);
                    x = i;
                    y = j;
                    if (minPossible == 1) {
                        goto DOUBLEBREAK;
                    }
                }
            }
        }
    }
    DOUBLEBREAK :
    
    
    /* No empty cell anymore */
    if (minPossible > size)
        return true;
    
    /* Try possibilities */
    for (int i: possibleNumbers) {
        field[x][y] = i;
        if (solveSudoku(field))
            return true;
        field[x][y] = 0;
    }
         
    return false;
}

int main(int argv, char** argc) {
    std::vector<std::string> arguments(argc, argc + argv);

    sudokuField field;
    if (arguments.size() != 1) {
        std::ifstream stream(arguments[1]);
        field = parseFile(stream);
    } else {
        field = parseFile(std::cin);
    }
    
    std::cout << field << std::endl;
    
    if (!checkField(field)) {
        std::cout << "Not a valid field." << std::endl;
        return 1;
    }
    
    auto begin = std::chrono::high_resolution_clock::now();
    bool res = solveSudoku(field);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<float> dur = end - begin;
    if (res) {
        std::cout << "Solution:\n" << field;
        std::cout << std::endl << dur.count() << " Seconds elapsed." << std::endl;
    } else
        std::cout << "No solution found." << std::endl;
    
    return 0;
}
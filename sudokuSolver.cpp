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

int steps;
long reads;

std::vector<int> getPossibilities(sudokuField& field, int x, int y) {
    int size = field.size();
    int sizeBase = std::sqrt(size);
    
    std::vector<bool> numbers;
    numbers.resize(size+1,false);
    
    /* Scan row, col and square */
    for (int k=0;k<size;k++) {
        ++reads;
        numbers[field[x][k]] = true;
    }
    for (int k=0;k<size;k++) {
        ++reads;
        numbers[field[k][y]] = true;
    }
    for (int k=(x/sizeBase)*sizeBase;k<(x/sizeBase+1)*sizeBase;k++) {
        for (int l=(y/sizeBase)*sizeBase;l<(y/sizeBase+1)*sizeBase;l++) {
            ++reads;
            numbers[field[k][l]] = true;
        }
    }
    
    std::vector<int> result;
    for(int k=1;k<=size;k++)
        if (!numbers[k])
            result.push_back(k);
    return result;
}

bool solveSudoku (sudokuField& field, int lastX, int lastY, int lastPossible) {
    ++steps;
    
    int size = field.size();
    int sizeBase = std::sqrt(size);
    
    std::vector<int> possibleNumbers;
    int minPossible = lastPossible;
    int x = -1, y = -1;
    
    
    /* Heuristic: search line, col and box of last insertion */
    for (int i=0;i<size;++i) {
        ++reads;
        if (field[i][lastY] == 0) {
            /* Count possibilities */
            std::vector<int> possible = getPossibilities(field,i,lastY);
                
            /* New minimum */
            if (possible.size() < minPossible) {
                minPossible = possible.size();
                possibleNumbers = possible;
                x = i;
                y = lastY;
                if (minPossible <= 1)
                    break;
            }
        }
        
        ++reads;
        if (field[lastX][i] == 0) {
            /* Count possibilities */
            std::vector<int> possible = getPossibilities(field,lastX,i);
                
            /* New minimum */
            if (possible.size() < minPossible) {
                minPossible = possible.size();
                possibleNumbers = possible;
                x = lastX;
                y = i;
                if (minPossible <= 1)
                    break;
            }
        }
        
        int j = (lastX/sizeBase)*sizeBase + i % sizeBase;
        int k = (lastY/sizeBase)*sizeBase + i / sizeBase;
        ++reads;
        if (field[j][k] == 0) {
            /* Count possibilities */
            std::vector<int> possible = getPossibilities(field,j,k);
                
            /* New minimum */
            if (possible.size() < minPossible) {
                minPossible = possible.size();
                possibleNumbers = possible;
                x = j;
                y = k;
                if (minPossible <= 1)
                    break;
            }
        }
    }
    
    /* if heuristic was not successfull (minPossible < lastPossible or no empty cell):
       search all empty cells for least possibilities */
    if (x == -1) {
        minPossible = size+1;
        for (int h=0;h<size*size;++h) {
            int i = h % size;
            int j = h / size;
            
            ++reads;
            if (field[i][j] == 0) {
                
                /* Count possibilities */
                std::vector<int> possible = getPossibilities(field,i,j);
                    
                /* New minimum */
                if (possible.size() < minPossible) {
                    minPossible = possible.size();
                    possibleNumbers = possible;
                    x = i;
                    y = j;
                    if (minPossible <= lastPossible)
                        break;
                }
            }
        }
    }
    
    /* No empty cell anymore */
    if (x == -1)
        return true;
    
    //std::cout << x << "," << y << std::endl;
    
    /* Try possibilities */
    for (int i: possibleNumbers) {
        field[x][y] = i;
        if (solveSudoku(field,x,y,minPossible))
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
    bool res = solveSudoku(field,0,0,0);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<float> dur = end - begin;
    auto d = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
    if (res) {
        std::cout << "Solution:\n" << field;
        std::cout << std::endl << d.count() << " Milliseconds elapsed." << std::endl;
        std::cout << "Steps: " << steps << " Reads: " << reads << std::endl;
    } else
        std::cout << "No solution found." << std::endl;
    
    return 0;
}
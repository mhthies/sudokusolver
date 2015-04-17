#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>

typedef std::vector<std::vector<int>> sudokuField;

sudokuField parseFile(std::istream& input) {
   sudokuField result;

   std::string line;
   while (std::getline(input, line) && result.size() < 9) {
      std::vector<int> v;
      for(char num: line) {
         if (num == ' ')
            v.push_back(0);
         else
            v.push_back(num - '0');
      }
      result.push_back(v);
   }
   
   return result;
}

bool checkField(const sudokuField& field) {
   if (field.size() != 9)
      return false;
   
   for (std::vector<int> v: field) {
      if (v.size() != 9)
         return false;
      
      for (int i: v) {
         if (i < 0 || i > 10)
            return false;
      }
   }
   
   return true;
}

void printField(std::ostream& os, const sudokuField& field) {
   for (std::vector<int> v: field) {
      for (int i: v) {
         if (i == 0)
            os << ' ';
         else
            os << i;
      }
      os << std::endl;
   }
}

std::ostream& operator<< (std::ostream& os, sudokuField field) {
   printField(os, field);
   return os;
}

bool solveSudoku (sudokuField& field) {
   std::vector<bool> allNumbers;
   allNumbers.push_back(false);
   for (int i=0;i<9;++i)
      allNumbers.push_back(true);
   
   std::vector<int> possibleNumbers;
   int minPossible = 10;
   int x,y;
   
   /* search empty cells */
   for (int i=0;i<9;++i) {
      for (int j=0;j<9;++j) {
         if (field[i][j] == 0) {
            std::vector<bool> numbers = allNumbers;
            
            // std::cout << std::endl;
            // std::cout << "x: " << i << " y: " j << std::endl;
         
            /* Scan row, col and square */
            for (int k=0;k<9;k++)
               numbers[field[i][k]] = false;
            for (int k=0;k<9;k++)
               numbers[field[k][j]] = false;
            for (int k=(i/3)*3;k<(i/3+1)*3;k++)
               for (int l=(j/3)*3;l<(j/3+1)*3;l++)
                  numbers[field[k][l]] = false;
                     
            /* Count possibilities */
            int possible = 0;
            for (bool p: numbers)
               if (p) possible++;
               
            /* New minimum */
            if (possible < minPossible) {
               minPossible = possible;
               possibleNumbers.clear();
               for(int k=1;k<=9;k++)
                  if (numbers[k])
                     possibleNumbers.push_back(k);
               x = i;
               y = j;
            }
         }
      }
   }
   
   /* No empty cell anymore */
   if (minPossible == 10)
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
   
   if (!checkField(field))
      std::cout << "No valid field." << std::endl;
   
   
   std::cout << field << std::endl;
   
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
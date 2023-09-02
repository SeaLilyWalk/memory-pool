#include <iostream>
#include <ctime>
#include <vector>

#include "allocator_stack.h"

#define ELEMS 1000000
#define REPS 50

int main() {
  clock_t start;

  // use the default allocator
  std::cout << "Use the default allocator..." << std::endl;
  AllocatorStack<int, std::allocator<int> > default_stack;
  start = clock();
  for (int i = 0; i < REPS; ++i) {
    for (int j = 0; j < ELEMS; ++j)
      default_stack.push(j);
    for (int j = 0; j < ELEMS; ++j)
      default_stack.pop();
  }
  double time1 = ((double)(clock() - start)) / CLOCKS_PER_SEC;
  std::cout << "Default allocator time: " << time1 << std::endl << std::endl;

  // use the std::vector
  std::cout << "Use the std::vector..." << std::endl;
  std::vector<int> vector_stack;
  start = clock();
  for (int i = 0; i < REPS; ++i) {
    for (int j = 0; j < ELEMS; ++j)
      vector_stack.push_back(j);
    for (int j = 0; j < ELEMS; ++j)
      vector_stack.pop_back();
  }
  double time2 = ((double)(clock() - start)) / CLOCKS_PER_SEC;
  std::cout << "std::vector time: " << time2 << std::endl << std::endl;

  return 0;
}

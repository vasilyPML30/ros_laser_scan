#include "scanner.h"
#include <iostream>
#include <fstream>

int main(int argc, char **argv) {
  if (argc < 2)
    return 0;
  std::ifstream in(argv[1]);
  Scanner scanner(in);
  std::cout << "created" << std::endl;
  scanner.scan();
  std::cout << "scanned" << std::endl;
  std::ofstream out(argv[2]);
  std::cout << "written" << std::endl;
  scanner.write(out);
  return 0;
}
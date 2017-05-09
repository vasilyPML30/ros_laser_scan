#include "scanner.h"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char **argv) {
  std::ifstream in_file;
  std::ofstream out_file, visual_file;
  bool visual = false;
  for (int i = 1; i < argc; i++) {
    if (argv[i] == std::string("-i")) {
      in_file = std::ifstream(argv[++i]);
      if (!in_file) {
        std::cerr << "unable to open input file" << std::endl;
        return 1;
      }
    }
    else if (argv[i] == std::string("-o")) {
      out_file = std::ofstream(argv[++i]);
      if (!out_file) {
        std::cerr << "unable to open output file" << std::endl;
        return 1;
      }
    }
    else if (argv[i] == std::string("-v")) {
      visual = true;
      visual_file = std::ofstream(argv[++i]);
      if (!visual_file) {
        std::cerr << "unable to open visualization file " << std::endl;
        return 1;
      }
    }
    else if (argv[i] == std::string("-f")) {
      std::cout << "-i file_name  (oblig.) input file name (.yaml)" << std::endl;
      std::cout << "-o file_name  (oblig.) output file name (.msg)" << std::endl;
      std::cout << "-v file_name  (opt.)   visual file name (.pgm)" << std::endl;
      return 0;
    }
    else {
      std::cout << "wrong arguments (-f for help)" << std::endl;
      return 1;
    }
  }
  if (!in_file || !out_file) {
    std::cout << "wrong arguments (-f for help)" << std::endl;
    return 1;
  }
  Scanner scanner(in_file);
  if (!scanner.is_correct())
    return -1;
  scanner.scan();
  scanner.write(out_file);
  if (visual)
    scanner.visualize(visual_file);
  std::cout << "Process completed successfully" << std::endl;
  return 0;
}
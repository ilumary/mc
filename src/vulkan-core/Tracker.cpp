#include <iostream>

#include "Tracker.hpp"

void *operator new (size_t size, char *file, int line) {
    std::cout << file << "," << line << ": allocation " << size << " bytes" << std::endl;
    return malloc (size);
}

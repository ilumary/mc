#undef new

void *operator new (size_t size, char *file, int line);

#define NEW new (__FILE__, __LINE__)

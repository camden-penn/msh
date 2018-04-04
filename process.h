// process.h
// Definition of a struct to hold process information.
#include <string>
#include <unistd.h>
struct process{
  pid_t id;
  std::string name;
};
std::string to_string(process & p);

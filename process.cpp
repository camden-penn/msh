#include "process.h"
std::string to_string(process & p){
  return "Process " + std::to_string(p.id) + ": " + p.name;
}

//
// Created by Aymane on 7/26/2023.
//

#ifndef ACCEPTANCE_PROJECT_PROCESS_H
#define ACCEPTANCE_PROJECT_PROCESS_H

#include <string>

class Process {
 public:
  // Returns the username associated with the given process ID.
  static std::string UserName(int pid);
};

#endif  // ACCEPTANCE_PROJECT_PROCESS_H

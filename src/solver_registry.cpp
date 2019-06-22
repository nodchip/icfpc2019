#include "solver_registry.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <experimental/filesystem>

void displayAndWait(SolverParam param, std::shared_ptr<Game> game) {
  if (param.wait_ms > 0) {
    std::cout << "\033[2J\033[1;1H"; // clear screen and return to top-left.
    std::cout << *game << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(param.wait_ms));
  }
}

void SolverRegistry::displaySolvers() {
  for (auto it = getRegistry().begin(); it != getRegistry().end(); ++it) {
    std::cout << "solver: " << it->first << " @ " << it->second.file_name << std::endl;
  }
}
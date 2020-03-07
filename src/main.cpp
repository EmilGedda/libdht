#include <iostream>

#include "dht.hpp"

int main(int /* argc */, char** /* argv */) {  // NOLINT
  dht::device dht22{ 2 };
  for (const auto [humidity, temperature]: dht22) {
    std::cout << "RH: " << humidity << "%, " << temperature << "C\n";
  }
}

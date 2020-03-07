#include <iostream>
#include "dht.hpp"

auto main(int /* argc */, char** /* argv */) -> int {
  dht::device dht22{2};
  for(const auto [humidity, temperature]: dht22) {
    std::cout << "RH: " << humidity << "%, " << temperature << "C\n";
  }
}

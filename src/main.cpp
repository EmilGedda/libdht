#include <iostream>
#include <compare>
#include "dht.hpp"

int main(int argc, char* argv[]) {

  dht::device dht22{2};
  for(const auto [humidity, temperature]: dht22) {
    std::cout << "RH: " << humidity << "%, " << temperature << "C\n";
  }
}

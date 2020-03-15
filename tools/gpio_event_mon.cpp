#include <dht/gpio.hpp>

#include <chrono>
#include <iostream>
#include <string>

auto to_string(dht::event_type e) {
  switch (e) {
  case dht::event_type::rising_edge: return "rising  edge";
  case dht::event_type::falling_edge: return "falling edge";
  default: return "unknown     ";
  }
}

int main(int argc, char** argv) {  // NOLINT
  if (argc != 3) {
    std::cerr << "usage: " << argv[0] << " /dev/gpiochipX GPIOPIN\n";
    return -1;
  }

  std::string      chip = argv[1];
  auto             pin  = std::stoi(argv[2]);
  dht::gpio_handle handle(pin, chip);

  std::cout << "Listening on chip " << chip << " pin " << pin << '\n';
  while (true) {
    auto [timestamp, event] = handle.listen();
    std::cout << "GPIO event " << to_string(event) << " @ "
              << timestamp.time_since_epoch().count() << '\n';
  }
}

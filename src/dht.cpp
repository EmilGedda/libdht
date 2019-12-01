
#include <cstdint>
#include <compare>
#include <thread>
#include <iostream>

#include "dht.hpp"

namespace dht {

  using namespace std::chrono;

  namespace {
  }

device::device(uint16_t pin, std::string_view chip) : pin(pin) {

}

response device::poll() {
  std::this_thread::sleep_for(seconds{1});
  return {};
}

iterator device::begin() noexcept {
  return iterator{*this};
}

iterator device::end() noexcept {
  return iterator{*this};
}

iterator::iterator(device& unit) noexcept : unit(unit) { }

iterator::value_type iterator::operator++() {
  return this->r = unit.poll();
}

iterator::value_type iterator::operator++(int) {
  auto res = this->r;
  ++(*this);
  return res;
}

iterator::value_type iterator::operator*() noexcept {
  return this->r;
}

bool iterator::operator==(iterator&) const noexcept {
  return false;
}

bool iterator::operator!=(iterator& rhs) const noexcept {
  return !(*this == rhs);
}

}

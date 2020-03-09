#include <dht/device.hpp>

namespace dht {

iterator::iterator(device& unit) noexcept : unit(unit) {
}

auto iterator::operator++() -> iterator::value_type {
  return this->r = unit.poll();
}

auto iterator::operator++(int) -> iterator::value_type {
  auto res = this->r;
  ++(*this);
  return res;
}

auto iterator::operator*() const noexcept -> iterator::value_type {
  return this->r;
}

auto iterator::operator==(end_iterator /* unused */) const noexcept -> bool {
  return false;
}

auto iterator::operator!=(end_iterator /* unused */) const noexcept -> bool {
  return true;
}

auto iterator::operator==(iterator /* unused */) const noexcept -> bool {
  return true;
}

auto iterator::operator!=(iterator /* unused */) const noexcept -> bool {
  return false;
}

}  // namespace dht

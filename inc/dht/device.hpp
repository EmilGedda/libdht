#ifndef DHT_DEVICE_HPP
#define DHT_DEVICE_HPP

#include "gpio.hpp"

#include <bitset>
#include <iterator>
#include <string>

/**
 * @namespace dht
 * Documentation for dht here.
 */
namespace dht {

struct response {
  float humidity    = 0;
  float temperature = 0;
};

struct device;

struct end_iterator {};

struct iterator {
  using iterator_category = std::input_iterator_tag;
  using value_type        = response;
  using difference_type   = void;
  using pointer           = void;
  using reference         = void;

  explicit iterator(device& unit) noexcept;
  auto operator++() -> value_type;
  auto operator++(int) -> value_type;
  auto operator*() const noexcept -> value_type;

  auto operator==(iterator /* unused */) const noexcept -> bool;
  auto operator!=(iterator /* unused */) const noexcept -> bool;
  auto operator==(end_iterator /* unused */) const noexcept -> bool;
  auto operator!=(end_iterator /* unused */) const noexcept -> bool;

 private:
  device&  unit;
  response r;
};

/**
 * Represents a DHT device
 */
struct device {
  explicit device(gpio_handle&& handle);
  explicit device(int pin, const std::string& chip = default_chip);

  auto poll() -> response;

  auto begin() noexcept -> iterator;
  auto static end() noexcept -> end_iterator;

 private:
  constexpr static auto response_bitcount  = 40;
  constexpr static auto response_bytecount = response_bitcount / 8;

  auto read_data() -> std::bitset<response_bitcount>;

  gpio_handle handle;
};

}  // namespace dht

#endif  // DHT_DEVICE_HPP

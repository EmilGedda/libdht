#pragma once

#include <bitset>
#include <iterator>
#include <cstdint>
#include <chrono>
#include <optional>
#include <string_view>

namespace dht {
  using namespace std::chrono;

  struct response {
    float humidity = 0;
    float temperature = 0;
  };

  struct device;

  struct end_iterator { };

  struct iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = response;
    using difference_type = void;
    using pointer = void;
    using reference = void;

    explicit iterator(device& unit) noexcept;
    auto operator++() -> value_type;
    auto operator++(int) -> value_type;
    auto operator*() const noexcept -> value_type;

    auto operator==(iterator) const noexcept -> bool;
    auto operator!=(iterator) const noexcept -> bool;
    auto operator==(end_iterator) const noexcept -> bool;
    auto operator!=(end_iterator) const noexcept -> bool;

  private:
    device& unit;
    response r;
  };

  const std::string default_chip = "/dev/gpiochip0";

  struct device {

    constexpr static auto response_bitcount = 40;
    constexpr static auto response_bytecount = response_bitcount / 8;

    explicit device(int pin, const std::string& chip = default_chip);

    ~device();

    auto poll() -> response;

    auto begin() noexcept -> iterator;
    auto static end() noexcept -> end_iterator;

  private:

    auto read_data() -> std::bitset<response_bitcount>;

    int pin;
    int fd;
  };

};

#pragma once

#include <iterator>
#include <cstdint>
#include <compare>
#include <chrono>
#include <optional>
#include <string_view>

namespace dht {
  using namespace std::literals;
  using namespace std::chrono;

  struct response {
    uint16_t humidity = 0;
    uint16_t temperature = 0;
  };

  struct device;

  struct iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = response;
    using difference_type = void;
    using pointer = void;
    using reference = void;


    explicit iterator(device& unit) noexcept;
    value_type operator++();
    value_type operator++(int);
    value_type operator*() noexcept;

    bool operator==(iterator&) const noexcept;
    bool operator!=(iterator&) const noexcept;

  private:
    device& unit;
    response r;
  };

  struct device {

    device(uint16_t pin, std::string_view chip = "/dev/gpiochip0"sv);

    response poll();

    iterator begin() noexcept;
    iterator end() noexcept;

  private:
    uint16_t pin;
    system_clock::time_point last_read;

  };

};

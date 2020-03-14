#pragma once

#include <linux/gpio.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

/**
 * @namespace dht
 * dht namespace
 */
namespace dht {

// both libc++ and libstdc++ has yet to implement
// C++20's constexpr std::string
inline static const std::string default_chip  = "/dev/gpio_handlechip0";
inline static const std::string default_label = "libdht";

enum struct event_request {
  rising_edge  = GPIOEVENT_REQUEST_RISING_EDGE,
  falling_edge = GPIOEVENT_REQUEST_FALLING_EDGE,
  any          = GPIOEVENT_REQUEST_BOTH_EDGES,
};

enum struct event_type {
  rising_edge  = GPIOEVENT_EVENT_RISING_EDGE,
  falling_edge = GPIOEVENT_EVENT_FALLING_EDGE,
};

enum struct direction {
  input  = GPIOHANDLE_REQUEST_INPUT,
  output = GPIOHANDLE_REQUEST_OUTPUT,
};

struct event_data {
  std::chrono::steady_clock::time_point timestamp;
  event_type type;
};

/**
 * gpio_handle does stuff
 */
struct gpio_handle {
  explicit gpio_handle(uint32_t pin, const std::string& chip = default_chip);
  explicit gpio_handle(const std::string_view& label,
                       uint32_t                pin,
                       const std::string&      chip = default_chip);

  ~gpio_handle();
  gpio_handle(const gpio_handle&) = delete;
  gpio_handle(gpio_handle&& old) noexcept;
  auto operator=(const gpio_handle&) -> gpio_handle& = delete;

  auto listen(event_request event = event_request::any) -> event_data;
  void write(bool value = false);
  void write(int value);

 private:
  void set_input(event_request event);
  void set_output(bool value);

  uint32_t         pin;
  int              chip_fd = -1;
  int              gpio_fd = -1;
  std::string_view label;
  direction        port_direction;
};


}  // namespace dht

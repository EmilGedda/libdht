#pragma once

#include <linux/gpio.h>

#include <string>
#include <string_view>

/**
 * @namespace dht
 * dht namespace
 */
namespace dht {

// both libc++ and libstdc++ has yet to implement
// C++20's constexpr std::string
inline static const std::string default_chip  = "/dev/gpiochip0";
inline static const std::string default_label = "libdht";

enum struct event {
  rising_edge  = GPIOEVENT_EVENT_RISING_EDGE,
  falling_edge = GPIOEVENT_EVENT_FALLING_EDGE,
  any          = GPIOEVENT_REQUEST_BOTH_EDGES,
};

enum struct request {
  input  = GPIOHANDLE_REQUEST_INPUT,
  output = GPIOHANDLE_REQUEST_OUTPUT,
};

/**
 * gpio_handle does stuff
 */
struct gpio_handle {
  explicit gpio_handle(int pin, const std::string& chip = default_chip);
  explicit gpio_handle(const std::string_view& label,
                       int                     pin,
                       const std::string&      chip = default_chip);

  ~gpio_handle();

  auto read_event(event = event::any) -> gpioevent_data;
  auto read_value() -> gpioevent_data;


 private:
  gpioevent_request req;
};

}  // namespace dht

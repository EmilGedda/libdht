#ifndef DHT_GPIO_HPP
#define DHT_GPIO_HPP

#include <linux/gpio.h>

#include <chrono>
#include <cstdint>
#include <exception>
#include <string>
#include <string_view>

/**
 * @namespace dht
 * dht namespace
 */
namespace dht {

// both libc++ and libstdc++ has yet to implement
// C++20's constexpr std::string
static const std::string default_chip  = "/dev/gpiochip0";
static const std::string default_label = "libdht";

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
  unknown,
};

struct event_data {
  std::chrono::steady_clock::time_point timestamp;
  event_type                            type;
};

using namespace std::chrono_literals;
/**
 * gpio_handle does stuff
 */
struct gpio_handle {
  explicit gpio_handle(uint32_t pin, const std::string& chip = default_chip);
  explicit gpio_handle(const std::string_view& label,
                       uint32_t                pin,
                       const std::string&      chip = default_chip);

  ~gpio_handle() noexcept;
  gpio_handle(gpio_handle&& old) noexcept;
  auto operator=(gpio_handle&& rhs) noexcept -> gpio_handle&;

  gpio_handle(const gpio_handle&) = delete;
  auto operator=(const gpio_handle&) -> gpio_handle& = delete;

  auto listen(event_request             event   = event_request::any,
              std::chrono::milliseconds timeout = 100ms) -> event_data;
  void write(bool value = false);
  void write(int value);

  auto get_pin() noexcept -> int;

  friend void swap(gpio_handle& a, gpio_handle& b) noexcept;

 private:
  void set_input(event_request event);
  void set_output(bool value);
  void static try_close(int& fd);

  uint32_t         pin;
  int              chip_fd = -1;
  int              gpio_fd = -1;
  std::string_view label;
  direction        port_direction = direction::unknown;
};

struct timeout_exceeded : std::exception {
  explicit timeout_exceeded(gpio_handle&              handle,
                            event_request             requested_event,
                            std::chrono::milliseconds timeout) noexcept;

  gpio_handle&              handle;
  event_request             requested_event;
  std::chrono::milliseconds timeout;
};


}  // namespace dht
#endif  // DHT_GPIO_HPP

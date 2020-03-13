#pragma once

#include <linux/gpio.h>

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
inline static const std::string default_chip  = "/dev/gpiochip0";
inline static const std::string default_label = "libdht";

enum struct request {
  rising_edge  = GPIOEVENT_REQUEST_RISING_EDGE,
  falling_edge = GPIOEVENT_REQUEST_FALLING_EDGE,
  any          = GPIOEVENT_REQUEST_BOTH_EDGES,
};

enum struct direction {
  input  = GPIOHANDLE_REQUEST_INPUT,
  output = GPIOHANDLE_REQUEST_OUTPUT,
};

enum struct event {};


/**
 * gpio does stuff
 */
struct gpio {
  explicit gpio(uint32_t pin, const std::string& chip = default_chip);
  explicit gpio(const std::string_view& label,
                uint32_t                pin,
                const std::string&      chip = default_chip);

  ~gpio();

  gpio(const gpio&) = delete;
  auto operator=(const gpio&) -> gpio& = delete;
  gpio(gpio&& old) noexcept;

  struct reader {
    explicit reader(const gpioevent_request& req) noexcept;
    auto listen() -> event;

   private:
    gpioevent_request req;
  };

  struct writer {
    explicit writer(const gpiohandle_request& req) noexcept;
    void write(bool);

   private:
    gpiohandle_request req;
  };

  auto input(request req = request::any) -> reader;
  auto output(bool default_value = false) -> writer;

 private:
  int              fd;
  uint32_t         pin;
  std::string_view label;
};

struct event_data {};

}  // namespace dht

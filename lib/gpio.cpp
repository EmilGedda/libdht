#include <dht/gpio.hpp>

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cerrno>
#include <cstdint>
#include <cstdio>

// silence IWYU
using __u8  = uint8_t;
using __u32 = uint32_t;

namespace dht {


gpio_handle::gpio_handle(uint32_t pin, const std::string& chip)
    : gpio_handle(default_label, pin, chip) {
}

gpio_handle::gpio_handle(gpio_handle&& old) noexcept
    : pin(old.pin)
    , chip_fd(old.chip_fd)
    , gpio_fd(old.gpio_fd)
    , label(old.label) {
  old.chip_fd = -1;
  old.gpio_fd = -1;
}

gpio_handle::gpio_handle(const std::string_view& label,
                         uint32_t                pin,
                         const std::string&      chip)
    : pin(pin), label(label) {
  chip_fd = open(chip.c_str(), O_RDONLY);
  if (chip_fd == -1) {
    std::perror("failed to open gpio_handle chip device");
    throw 1;  // TODO: fix
  }
}

gpio_handle::~gpio_handle() {
  if (chip_fd == -1) return;

  auto err = close(chip_fd);
  if (err == -1) {
    std::perror("failed to close gpio_handle file descriptor");
  }
}

void gpio_handle::set_input(event_request event) {
  gpioevent_request event_request;

  event_request.lineoffset  = pin;
  event_request.eventflags  = static_cast<uint32_t>(event);
  event_request.handleflags = static_cast<uint32_t>(direction::input);

  auto n = std::min(label.size(), sizeof(event_request.consumer_label));
  std::copy_n(label.begin(), n, event_request.consumer_label);

  auto ret = ioctl(chip_fd, GPIO_GET_LINEEVENT_IOCTL, &event_request);
  if (ret == -1) {
    std::perror("unable to issue get line event");
    throw 1;  // TODO: fix
  }

  gpio_fd        = event_request.fd;
  port_direction = direction::input;
}

void gpio_handle::set_output(bool value) {
  gpiohandle_request handle_request;

  handle_request.lines             = 1;
  handle_request.lineoffsets[0]    = pin;
  handle_request.default_values[0] = static_cast<uint8_t>(value);
  handle_request.flags             = static_cast<uint32_t>(direction::output);

  auto n = std::min(label.size(), sizeof(handle_request.consumer_label));
  std::copy_n(label.begin(), n, handle_request.consumer_label);

  auto ret = ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &handle_request);
  if (ret == -1) {
    std::perror("unable to issue get line handle");
    throw 1;  // TODO: fix
  }

  gpio_fd        = handle_request.fd;
  port_direction = direction::output;
}

auto gpio_handle::listen(event_request event) -> event_data {
  if (port_direction != direction::input) {
    set_input(event);
  }

  gpioevent_data data;

  while (true) {
    auto ret = read(gpio_fd, &data, sizeof(gpioevent_data));

    if (ret == -1) {
      if (errno == -EAGAIN) continue;
      std::perror("failed to read event data");
      throw 1;
    }

    if (ret != sizeof(data)) {
      std::printf("reading event data failed. Read %zu bytes, expected %lu bytes\n", sizeof(data), ret);
      throw 1; // TODO fix
    }
  }

  return {
    std::chrono::steady_clock::time_point{
      std::chrono::nanoseconds{data.timestamp}
    },
    static_cast<event_type>(data.id)
  };
}

auto gpio_handle::write(int value) -> void {
  write(value != 0);
}

auto gpio_handle::write(bool value) -> void {
  if (port_direction != direction::output) {
    set_output(value);
  }
}

}  // namespace dht

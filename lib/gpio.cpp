#include <dht/gpio.hpp>

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
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
  chip_fd = open(chip.c_str(), O_RDWR);
  if (chip_fd == -1) {
    std::perror("failed to open gpio_handle chip device");
    throw 1;  // TODO: fix
  }
}

gpio_handle::~gpio_handle() noexcept {
  if (chip_fd == -1) return;

  auto err = close(chip_fd);
  if (err == -1) {
    std::perror("failed to close gpio_handle chip file descriptor");
  }

  try_close_gpio();
}

void gpio_handle::try_close_gpio() {
  if (gpio_fd < 1) return;
  auto err = close(gpio_fd);
  if (err == -1) {
    std::perror("failed to close gpio_handle gpio file descriptor");
  }
}

void gpio_handle::set_input(event_request event) {
  gpioevent_request req{};

  req.lineoffset  = pin;
  req.eventflags  = static_cast<uint32_t>(event);
  req.handleflags = static_cast<uint32_t>(direction::input);

  auto n = std::min(label.size(), sizeof(req.consumer_label));
  std::copy_n(label.begin(), n, req.consumer_label);

  auto ret = ioctl(chip_fd, GPIO_GET_LINEEVENT_IOCTL, &req);
  if (ret == -1) {
    std::perror("unable to issue get line event");
    throw 1;  // TODO: fix
  }

  if (req.fd < 1) {
    std::perror("get line event returned invalid gpio file descriptor");
    throw 1;  // TODO: fix
  }

  try_close_gpio();
  gpio_fd        = req.fd;
  port_direction = direction::input;
}

auto gpio_handle::listen(event_request event) -> event_data {
  if (port_direction != direction::input) {
    set_input(event);
  }

  gpioevent_data data;

  while (true) {
    auto ret = read(gpio_fd, &data, sizeof(data));
    if (ret == -1) {
      std::perror("failed to read event data");
      if (errno == -EAGAIN) continue;
      throw 1;
    }

    if (ret != sizeof(data)) {
      std::printf(
          "reading event data failed. Read %zu bytes, expected %lu bytes\n",
          sizeof(data),
          ret);
      throw 1;  // TODO fix
    }
    return { std::chrono::steady_clock::time_point{
                 std::chrono::nanoseconds{ data.timestamp } },
             static_cast<event_type>(data.id) };
  }
}

void gpio_handle::set_output(bool value) {
  gpiohandle_request req{};

  req.lines             = 1;
  req.lineoffsets[0]    = pin;
  req.default_values[0] = static_cast<uint8_t>(value);
  req.flags             = static_cast<uint32_t>(direction::output);

  auto n = std::min(label.size(), sizeof(req.consumer_label));
  std::copy_n(label.begin(), n, req.consumer_label);

  auto ret = ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
  if (ret == -1) {
    std::perror("unable to issue get line handle");
    throw 1;  // TODO: fix
  }

  try_close_gpio();
  gpio_fd        = req.fd;
  port_direction = direction::output;
}

auto gpio_handle::write(int value) -> void {
  write(value != 0);
}

auto gpio_handle::write(bool value) -> void {
  if (port_direction != direction::output) {
    set_output(value);
  }
}

auto gpio_handle::get_pin() noexcept -> int {
  return pin;
}
}  // namespace dht

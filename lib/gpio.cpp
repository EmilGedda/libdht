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
#include <utility>

// silence IWYU
using __u8  = uint8_t;
using __u32 = uint32_t;

namespace dht {


gpio_handle::gpio_handle(uint32_t pin, const std::string& chip)
    : gpio_handle(default_label, pin, chip) {
}

gpio_handle::gpio_handle(gpio_handle&& old) noexcept {
  swap(*this, old);
}

auto gpio_handle::operator=(gpio_handle&& rhs) noexcept -> gpio_handle& {
  swap(*this, rhs);
  return *this;
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
  try_close(gpio_fd);
  try_close(chip_fd);
}

void gpio_handle::try_close(int& fd) {
  if (fd < 1) return;
  auto err = close(fd);
  if (err == -1) {
    std::perror("failed to close file descriptor");
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

  try_close(gpio_fd);

  gpio_fd        = req.fd;
  port_direction = direction::input;
}

auto gpio_handle::listen(event_request event) -> event_data {
  if (port_direction != direction::input) {
    set_input(event);
  }

  ssize_t        ret;
  gpioevent_data data;

  do {
    ret = read(gpio_fd, &data, sizeof(data));
  } while (ret == -1 && errno == -EAGAIN);

  if (ret == -1) {
    std::perror("failed to read event data");
    throw 1;
  }

  if (ret != sizeof(data)) {
    std::printf(
        "reading event data failed. Read %zu bytes, expected %zd bytes\n",
        sizeof(data),
        ret);
    throw 1;  // TODO fix
  }

  return { std::chrono::steady_clock::time_point{
               std::chrono::nanoseconds{ data.timestamp } },
           static_cast<event_type>(data.id) };
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

  try_close(gpio_fd);

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

  gpiohandle_data data{};

  data.values[0] = static_cast<uint32_t>(value);

  auto ret = ioctl(gpio_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
  if (ret == -1) {
    std::perror("unable to set line values");
    throw 1;  // TODO: fix
  }
}

auto gpio_handle::get_pin() noexcept -> int {
  return pin;
}

void swap(gpio_handle& a, gpio_handle& b) noexcept {
  using std::swap;
  swap(a.pin, b.pin);
  swap(a.chip_fd, b.chip_fd);
  swap(a.gpio_fd, b.gpio_fd);
  swap(a.label, b.label);
  swap(a.port_direction, b.port_direction);
}

}  // namespace dht

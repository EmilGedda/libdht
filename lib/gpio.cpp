#include <dht/gpio.hpp>

#include <fcntl.h>
#include <linux/gpio.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <stdexcept>
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
  chip_fd = open(chip.c_str(), O_RDWR | O_CLOEXEC);
  if (chip_fd == -1) {
    throw std::runtime_error("unable to open gpio chip");  // TODO: fix
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
    std::perror("try_close(): failed to close file descriptor");
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
    std::string error = std::strerror(errno);
    throw std::runtime_error("set_input(): unable to issue get line event: "
                             + error);
  }

  if (req.fd < 1) {
    throw std::runtime_error(
        "get line event returned invalid gpio file descriptor");
  }

  try_close(gpio_fd);

  gpio_fd        = req.fd;
  port_direction = direction::input;
}

auto gpio_handle::listen(event_request event, std::chrono::milliseconds timeout)
    -> event_data {
  if (port_direction != direction::input) {
    set_input(event);
  }

  std::array fds = { pollfd{
      gpio_fd,
      POLLIN,
  } };

  auto ret = poll(fds.begin(), 1, timeout.count());

  if (ret == -1) {
    std::string err = std::strerror(errno);
    throw std::runtime_error("poll() returned -1: " + err);
  }

  if (ret == 0) {
    throw timeout_exceeded{ *this, event, timeout };
  }

  if (ret != 1) {
    throw std::runtime_error("unknown exception occured in listen");
  }

  if ((fds[0].revents & POLLERR) == POLLERR) {
    std::string err = std::strerror(errno);
    throw std::runtime_error("POLLERR returned from poll(): " + err);
  }

  if ((fds[0].revents & POLLOUT) != POLLOUT) {
    throw std::runtime_error("poll() returned without any readable data");
  }

  gpioevent_data data;
  ret = read(gpio_fd, &data, sizeof(data));

  if (ret == -1) {
    using namespace std::string_literals;
    throw std::runtime_error("listen() failed to read data: "s
                             + std::strerror(errno));
  }

  if (ret != sizeof(data)) {
    std::ostringstream ss;
    ss << "reading event data failed. Read " << ret << ", expected "
       << sizeof(data) << " bytes";
    throw std::runtime_error(ss.str());
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
    std::string err = std::strerror(errno);
    throw std::runtime_error("set_output(): unable to get line handle: " + err);
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
    std::string err = std::strerror(errno);
    throw std::runtime_error("write(): unable to set line values: " + err);
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


timeout_exceeded::timeout_exceeded(gpio_handle&              handle,
                                   event_request             requested_event,
                                   std::chrono::milliseconds timeout) noexcept
    : handle(handle), requested_event(requested_event), timeout(timeout) {
}

}  // namespace dht

#include <dht/gpio.hpp>

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>

// silence IWYU
using __u8  = uint8_t;
using __u32 = uint32_t;

namespace dht {


gpio::gpio(uint32_t pin, const std::string& chip)
    : gpio(default_label, pin, chip) {
}

gpio::gpio(gpio&& old) noexcept : fd(old.fd), pin(old.pin), label(old.label) {
  old.fd = 0;
}

gpio::gpio(const std::string_view& label, uint32_t pin, const std::string& chip)
    : pin(pin), label(label) {
  fd = open(chip.c_str(), O_RDONLY);
  if (fd == -1) {
    std::perror("failed to open gpio chip device");
    throw 1;  // TODO: fix
  }
}

gpio::~gpio() {
  if (fd == -1) { return; }

  auto err = close(fd);
  if (err == -1) { std::perror("failed to close gpio file descriptor"); }
}


auto gpio::input(request req) -> reader {
  gpioevent_request event_request;

  event_request.lineoffset  = pin;
  event_request.eventflags  = static_cast<uint32_t>(req);
  event_request.handleflags = static_cast<uint32_t>(direction::input);

  auto n = std::min(label.size(), sizeof(event_request.consumer_label));
  std::copy_n(label.begin(), n, event_request.consumer_label);

  auto ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &event_request);
  if (ret == -1) {
    std::perror("unable to issue get line event");
    throw 1;  // TODO: fix
  }

  return reader{ event_request };
}

auto gpio::output(bool default_value) -> writer {
  gpiohandle_request handle_request;

  handle_request.flags             = static_cast<uint32_t>(direction::output);
  handle_request.lines             = 1;
  handle_request.lineoffsets[0]    = pin;
  handle_request.default_values[0] = static_cast<uint8_t>(default_value);

  auto n = std::min(label.size(), sizeof(handle_request.consumer_label));
  std::copy_n(label.begin(), n, handle_request.consumer_label);

  auto ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &handle_request);
  if (ret == -1) {
    std::perror("unable to issue get line handle");
    throw 1;  // TODO: fix
  }

  return writer{ handle_request };
}

gpio::reader::reader(const gpioevent_request& req) noexcept : req(req) {
}

gpio::writer::writer(const gpiohandle_request& req) noexcept : req(req) {
}

}  // namespace dht

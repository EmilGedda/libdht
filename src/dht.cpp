#include "dht.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <thread>

namespace dht {

namespace {

template <size_t N>
constexpr auto bitset_to_bytes(const std::bitset<N>& set) {
  std::array<uint8_t, N / 8> bytes{};
  for (int i = 0; i < bytes.size(); i++) {
    uint8_t current{ 0 };
    for (int j = 0; j < 8; j++) {
      current = set[i * 8 + j] << j;
    }
    bytes[i] = current;
  }
  return bytes;
}

}  // namespace

device::device(int pin, const std::string& chip) : pin(pin) {
  fd = open(chip.c_str(), O_RDWR);
}

auto device::poll() -> response {
  std::array<uint8_t, response_bytecount> data;
  auto                                    valid_crc = false;

  while (!valid_crc) {
    data          = bitset_to_bytes(read_data());
    auto checksum = data[4];
    auto crc      = std::accumulate(data.begin(), data.begin() + 4, uint8_t(0));
    valid_crc     = crc == checksum;
    if (!valid_crc) {
      std::cerr << "Invalid CRC reading: got " << std::hex << crc
                << ", expected " << checksum << '\n';
    }
  }

  auto humidity    = static_cast<float>(data[0] << 8 | data[1]) / 10.0f;
  auto temperature = static_cast<float>(data[2] << 8 | data[3]) / 10.0f;

  return { humidity, temperature };
}

auto device::read_data() -> std::bitset<response_bitcount> {
  // https://github.com/torvalds/linux/blob/master/tools/gpio/gpio-event-mon.c

  std::bitset<response_bitcount> data;
  fd++;
  // wait until line is IDLE
  // pull LOW for 18ms
  // pull HIGH for around 30µs.
  // DHT22 responds by pull LOW for 80us.
  // DHT22 will pull HIGH for 80µs
  // Next it will send 40 bits of Data. Each bit starts with a 50µs LOW followed
  // by HIGH for 26-28µs for a “0” or for 70µs for a “1”.

  // i. wait for low (falling edge)
  for (int i = 0; i < response_bitcount; i++) {
    // ii.  wait for high (rising edge)
    // iii. verify time between ii and i (or iv) is 50µs
    // iv.  wait for low (falling edge)
    // v.   time between iv and ii determines bit.
  }

  // After communication ends, the Line is pulled HIGH by the pull-up resistor
  // and enters IDLE state.
  return data;
}

auto device::begin() noexcept -> iterator {
  return iterator{ *this };
}

auto device::end() noexcept -> end_iterator {
  return end_iterator{};
}

device::~device() {
  close(fd);
}

iterator::iterator(device& unit) noexcept : unit(unit) {
}

auto iterator::operator++() -> iterator::value_type {
  return this->r = unit.poll();
}

auto iterator::operator++(int) -> iterator::value_type {
  auto res = this->r;
  ++(*this);
  return res;
}

auto iterator::operator*() const noexcept -> iterator::value_type {
  return this->r;
}

auto iterator::operator==(end_iterator /* unused */) const noexcept -> bool {
  return false;
}

auto iterator::operator!=(end_iterator rhs) const noexcept -> bool {
  return !(*this == rhs);
}

auto iterator::operator==(iterator /* unused */) const noexcept -> bool {
  return false;
}

auto iterator::operator!=(iterator rhs) const noexcept -> bool {
  return !(*this == rhs);
}

}  // namespace dht

#include <dht/device.hpp>
#include <dht/gpio.hpp>

#include <array>
#include <bitset>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>

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

device::device(gpio_handle&& handle) : handle(std::move(handle)) {
}

device::device(int pin, const std::string& chip) : handle(pin, chip) {
}

auto device::poll() -> response {
  auto data      = std::array<uint8_t, response_bytecount>{};
  auto valid_crc = false;

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
  using namespace std::chrono_literals;
  //  auto low = event_request::falling_edge;
  //  auto high = event_request::rising_edge;

  std::bitset<response_bitcount> data;
  std::cout << "Initiating communication...\n"
            << static_cast<int>(handle.listen().type);
  //
  //  // Communication starts at HIGH
  //  handle.write(1);
  //  std::this_thread::sleep_for(5ms);
  //
  //  // Host pulls LOW for 1ms minimum
  //  handle.write(0);
  //  std::this_thread::sleep_for(5ms);
  //  // Host pulls HIGH for 20-40µs
  //  handle.write(1);
  //  // Sensor pulls LOW for 80µs
  //  auto a = handle.listen(low);
  //  // if listen() blocks for >40µs, no sensor was found;
  //
  //  // Sensor pulls HIGH for 80µs
  //  auto b = handle.listen(high);
  //
  //  // Next it will send 40 bits of Data. Each bit starts with a 50µs LOW
  //  followed
  //  // by HIGH for 26-28µs for a “0” or for 70µs for a “1”.
  //  // i. wait for low (falling edge)
  //  a = handle.listen(event_request::falling_edge);
  //  for (int i = 0; i < response_bitcount; i++) {
  //    // ii.  wait for high (rising edge)
  //    b = handle.listen(event_request::rising_edge);
  //    // iii. verify time between ii and i (or iv) is 50µs
  //    // iv.  wait for low (falling edge)
  //    a = handle.listen(event_request::falling_edge);
  //    // v.   time between iv and ii determines bit.
  //    auto duration = a.timestamp - b.timestamp;
  //    if (duration < 75us && duration > 65us) {
  //      data.set(i);
  //    }
  //  }

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

}  // namespace dht

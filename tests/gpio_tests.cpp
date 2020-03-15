#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <dht/gpio.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <fstream>  // IWYU pragma: keep
#include <iostream>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using namespace dht;

template <size_t Pins>
struct virtual_gpio {
  std::string chip;

  virtual_gpio() {
    std::system("rmmod gpio-mockup -f -s");
    auto existing_chips = get_chips();

    std::string cmd =
        "modprobe gpio-mockup gpio_mockup_ranges=-1," + std::to_string(Pins);
    std::system(cmd.c_str());
    int tries = 0;

    while (tries < 3) {
      tries++;
      auto new_chips = get_chips();

      std::vector<std::string> mocked_chips;
      std::set_difference(new_chips.begin(),
                          new_chips.end(),
                          existing_chips.begin(),
                          existing_chips.end(),
                          std::back_inserter(mocked_chips));

      if (mocked_chips.size() != 1) continue;
      chip = mocked_chips.front();
      return;
    }

    std::cerr << "Unable to find the virtual gpiochip\n";
    std::abort();
  }

  ~virtual_gpio() {
    std::system("rmmod gpio-mockup -f -s");
  }

  auto get_chips() {
    std::vector<std::string> existing_chips;
    for (const auto& chip: fs::directory_iterator("/dev")) {
      std::string filename = chip.path().filename().native();
      // filter files starting with gpiochip
      if (filename.rfind("gpiochip", 0) == 0) {
        existing_chips.push_back(std::move(filename));
      }
    }
    std::sort(existing_chips.begin(), existing_chips.end());
    return existing_chips;
  }

  auto next_pin() {
    if (current_pin == Pins) {
      std::cerr << "Out of GPIO pins! Increase Pin number to virtual_gpio\n";
      std::abort();
    }
    return current_pin++;
  }

  auto new_handle() {
    return gpio_handle(next_pin(), "/dev/" + chip);
  }

  auto read_pin(int pin) {
    auto path =
        "/sys/kernel/debug/gpio-mockup/" + chip + "/" + std::to_string(pin);
    int value = -1;
    std::ifstream(path) >> value;
    return value;
  }

  auto set_pin(int pin, bool value) {
    auto path =
        "/sys/kernel/debug/gpio-mockup/" + chip + "/" + std::to_string(pin);
    std::ofstream(path) << value;
  }

 private:
  int current_pin = 0;
};

TEST_CASE("test gpio_handle against virtual gpio") {
  virtual_gpio<4> gpio_mockup;

  SUBCASE("test virtual gpio reading and writing") {
    auto pin = gpio_mockup.next_pin();
    CHECK(gpio_mockup.read_pin(pin) == 0);
    gpio_mockup.set_pin(pin, true);
    CHECK(gpio_mockup.read_pin(pin) == 1);
    gpio_mockup.set_pin(pin, false);
    CHECK(gpio_mockup.read_pin(pin) == 0);
    gpio_mockup.set_pin(pin, true);
    gpio_mockup.set_pin(pin, true);
    CHECK(gpio_mockup.read_pin(pin) == 1);
    gpio_mockup.set_pin(pin, false);
    gpio_mockup.set_pin(pin, false);
    CHECK(gpio_mockup.read_pin(pin) == 0);
  }

  SUBCASE("test construction and destruction of gpio_handle") {
    gpio_handle(gpio_mockup.next_pin(), "/dev/" + gpio_mockup.chip);
  }


  SUBCASE("test writing to gpio pin") {
    auto handle = gpio_mockup.new_handle();
    auto pin    = handle.get_pin();
    CHECK(gpio_mockup.read_pin(pin) == 0);
    handle.write(1);
    CHECK(gpio_mockup.read_pin(pin) == 1);
    handle.write(0);
    CHECK(gpio_mockup.read_pin(pin) == 0);
    handle.write(1);
    handle.write(1);
    CHECK(gpio_mockup.read_pin(pin) == 1);
    handle.write(0);
    handle.write(0);
    CHECK(gpio_mockup.read_pin(pin) == 0);
  }


  SUBCASE("test listening to gpio events") {
    bool                    ready = false;
    std::mutex              m;
    std::condition_variable cv;

    auto handle = gpio_mockup.new_handle();
    auto pin    = handle.get_pin();

    auto mark_ready = [&]() {
      std::scoped_lock lock(m);
      ready = true;
      cv.notify_one();
    };

    auto set_pin = [&](int value) {
      using namespace std::chrono_literals;
      std::unique_lock lock(m);
      cv.wait(lock, [&] { return ready; });
      ready = false;
      std::this_thread::sleep_for(10ms);
      gpio_mockup.set_pin(pin, value != 0);
    };

    // TODO: C++20 std::jthread
    std::thread pin_hammer([&] {
      set_pin(1);
      set_pin(0);
      set_pin(1);
      set_pin(0);
    });

    mark_ready();
    CHECK(handle.listen().type == event_type::rising_edge);
    mark_ready();
    CHECK(handle.listen().type == event_type::falling_edge);
    mark_ready();
    CHECK(handle.listen().type == event_type::rising_edge);
    mark_ready();
    CHECK(handle.listen().type == event_type::falling_edge);

    pin_hammer.join();
  }
}

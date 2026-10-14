#include <rex/graphics/vulkan/fetch_register_update.h>
#include <array>
#include <cassert>
#include <random>

int main() {
  std::mt19937 random(99680);
  std::array<uint32_t, 192> registers{}, expected{}, guest{};
  for (unsigned trial = 0; trial < 10000; ++trial) {
    const unsigned first = random() % 193;
    const unsigned count = random() % (193 - first);
    std::array<bool, 96> expected_vertices{}, actual_vertices{};
    uint32_t expected_textures = 0;
    for (unsigned i = 0; i < count; ++i) {
      const uint32_t value = (random() & 1) ? random() : registers[first + i];
      guest[i] = std::endian::native == std::endian::little ? std::byteswap(value) : value;
      if (expected[first + i] != value) {
        expected_textures |= UINT32_C(1) << ((first + i) / 6);
        expected_vertices[(first + i) / 2] = true;
        expected[first + i] = value;
      }
    }
    const auto words = std::span<const uint32_t>(guest.data(), count);
    const auto mask = rex::graphics::vulkan::UpdateFetchRegisters(registers, first, words,
        [&](uint32_t vertex) { assert(vertex < 96); actual_vertices[vertex] = true; });
    assert(registers == expected);
    assert(mask == expected_textures);
    assert(actual_vertices == expected_vertices);
    // Replaying a packet must not upload or invalidate anything again.
    assert(!rex::graphics::vulkan::UpdateFetchRegisters(registers, first, words,
        [](uint32_t) { assert(false); }));
  }
}

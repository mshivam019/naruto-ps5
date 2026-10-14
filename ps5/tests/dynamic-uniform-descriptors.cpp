#include <rex/graphics/vulkan/descriptor_snapshot.h>
#include <array>
#include <cassert>

int main() {
  rex::graphics::vulkan::DynamicUniformDescriptorSnapshot<5> snapshot;
  std::array<VkDescriptorBufferInfo, 5> buffers{};
  for (size_t i = 0; i < buffers.size(); ++i) {
    buffers[i] = {reinterpret_cast<VkBuffer>(uintptr_t(i + 1)), 256 * i, 128};
  }
  assert(!snapshot.MatchAndRemember(buffers));
  for (unsigned draw = 1; draw < 10000; ++draw) {
    for (auto& buffer : buffers) buffer.offset += 256;
    assert(snapshot.MatchAndRemember(buffers));
    for (size_t i = 0; i < buffers.size(); ++i) {
      // The descriptor plus bind-time offset must address the exact same data
      // as the previous non-dynamic descriptor, without widening its range.
      const auto& descriptor = snapshot.data()[i];
      assert(descriptor.buffer == buffers[i].buffer);
      assert(descriptor.offset + buffers[i].offset == buffers[i].offset);
      assert(descriptor.range == buffers[i].range);
    }
  }
  buffers[2].buffer = reinterpret_cast<VkBuffer>(uintptr_t(99));
  assert(!snapshot.MatchAndRemember(buffers));
  assert(snapshot.MatchAndRemember(buffers));
  buffers[3].range += 16;
  assert(!snapshot.MatchAndRemember(buffers));
  snapshot.Invalidate();
  assert(!snapshot.MatchAndRemember(buffers));
  assert(snapshot.MatchAndRemember(buffers));
}

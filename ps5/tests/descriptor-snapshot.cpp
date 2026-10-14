#include <rex/graphics/vulkan/descriptor_snapshot.h>
#include <array>
#include <cassert>
using rex::graphics::vulkan::TextureDescriptorSnapshot;
int main() {
  TextureDescriptorSnapshot snapshot;
  auto layout = reinterpret_cast<VkDescriptorSetLayout>(uintptr_t(1));
  std::array<VkDescriptorImageInfo, 2> bindings{};
  bindings[0].imageView = reinterpret_cast<VkImageView>(uintptr_t(2));
  bindings[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  bindings[1].sampler = reinterpret_cast<VkSampler>(uintptr_t(3));
  assert(!snapshot.MatchAndRemember(layout, 1, bindings));
  assert(snapshot.MatchAndRemember(layout, 1, bindings));
  assert(snapshot.MatchesShape(layout, 1, 1));
  assert(!snapshot.MatchesShape(layout, 2, 0));
  assert(!snapshot.MatchesShape(layout, 1, 2));
  assert(snapshot.MatchesSampler(0, bindings[1].sampler));
  assert(!snapshot.MatchesSampler(1, bindings[1].sampler));
  assert(!snapshot.MatchesSampler(0, reinterpret_cast<VkSampler>(uintptr_t(9))));
  bindings[0].imageView = reinterpret_cast<VkImageView>(uintptr_t(4));
  assert(!snapshot.MatchAndRemember(layout, 1, bindings));
  bindings[1].sampler = reinterpret_cast<VkSampler>(uintptr_t(5));
  assert(!snapshot.MatchAndRemember(layout, 1, bindings));
  bindings[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  assert(!snapshot.MatchAndRemember(layout, 1, bindings));
  assert(!snapshot.MatchAndRemember(layout, 0, bindings));
  layout = reinterpret_cast<VkDescriptorSetLayout>(uintptr_t(6));
  assert(!snapshot.MatchAndRemember(layout, 0, bindings));
  assert(snapshot.MatchAndRemember(layout, 0, bindings));
  snapshot.Invalidate();
  assert(!snapshot.MatchesShape(layout, 0, 2));
  assert(!snapshot.MatchesSampler(0, bindings[0].sampler));
  assert(!snapshot.MatchAndRemember(layout, 0, bindings));
  assert(!snapshot.MatchAndRemember(layout, 0, {}));
}

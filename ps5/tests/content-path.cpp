#include <rex/filesystem/case_path.h>
#include <cassert>
#include <fstream>
#include <unistd.h>
int main() {
  char temp[] = "/tmp/naruto-content-XXXXXX";
  auto* name = mkdtemp(temp); assert(name);
  const std::filesystem::path root(name);
  std::filesystem::create_directory(root / "PlayerSave1.ninjasave");
  std::ofstream(root / "PlayerSave1.ninjasave" / "Save.dat") << "preserved progress";
  std::ofstream(root / "PlayerSave1.ninjasave.header") << "header";
  using rex::filesystem::ResolveCaseInsensitiveLeaf;
  auto resolved = ResolveCaseInsensitiveLeaf(root / "playersave1.ninjasave");
  assert(resolved == root / "PlayerSave1.ninjasave");
  std::string data; std::getline(std::ifstream(resolved / "Save.dat"), data);
  assert(data == "preserved progress");
  assert(ResolveCaseInsensitiveLeaf(root / "PLAYERSAVE1.NINJASAVE.HEADER") == root / "PlayerSave1.ninjasave.header");
  assert(ResolveCaseInsensitiveLeaf(resolved) == resolved);
  assert(ResolveCaseInsensitiveLeaf(root / "NewSave") == root / "NewSave");
  assert(ResolveCaseInsensitiveLeaf(root / "missing" / "NewSave") == root / "missing" / "NewSave");
  std::filesystem::remove_all(root);
}

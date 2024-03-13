#include "world.h"
#include "../../plugin.h"
World::World(const std::vector<std::string> &statics) {
  for (const auto &s : statics) std::cout << s << std::endl;
}

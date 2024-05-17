#include <cstddef>
#include <vector>

// A literal denotes a literal in a precondition
// Which is the combination of a predicate and parameter indexes
struct Literal {
  // Denotes the predicate index
  std::size_t predicate;
  // Denotes the parameter indexes
  std::vector<std::size_t> params;
  Literal(
    std::size_t predicate,
    const std::vector<std::size_t> &&params
  );
};

// A combination is a set of precondition literals
// Allows the addition of new params
struct Combination {
  // The number of states wherein the combination is applicable
  std::size_t applicable;
  // What parameters are added, potentially none
  std::vector<std::size_t> params;
  // What literals build the combination
  std::vector<Literal> literals;
  Combination(
    std::size_t applicable, 
    std::vector<std::size_t> params, 
    std::vector<Literal> literals
  );
};

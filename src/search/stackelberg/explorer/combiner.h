#include <cstddef>
#include <functional>
#include <unordered_map>
#include "combination.h"
#include "world.h"
namespace symbolic {
class SymVariables;
}
class Combiner {
public:
  Combiner(
    const std::size_t &&min_parameters,
    const std::size_t &&max_parameters,
    const std::size_t &&min_precondition_size,
    const std::size_t &&max_precondition_size
  );

  void Init(
    std::shared_ptr<symbolic::SymVariables> vars,
    const World &world
  );

  virtual void Combine(
    const World &world,
    const BDD &valid,
    const BDD &invalid,
    std::function<void (Combination)> f
  );

protected:
  std::shared_ptr<symbolic::SymVariables> vars;
  const std::size_t min_parameters;
  const std::size_t max_parameters;
  const std::size_t min_precondition_size;
  const std::size_t max_precondition_size;
  std::size_t default_parameters;
  std::vector<std::vector<std::size_t>> default_instantiations;
  std::unordered_map<std::size_t, std::vector<Literal>> expansion_literals;

  std::vector<Literal> GenerateRoot(const World &world, std::size_t param_count);
  void ExploreDAG(
    const World &world, 
    const std::vector<Literal> &literals,
    const BDD &valid,
    const BDD &invalid,
    std::function<void (Combination)> f
  );
  void ExploreDAG(
    const World &world, 
    const std::vector<Literal> &literals,
    const BDD &valid,
    const BDD &invalid,
    std::vector<std::size_t> &comb,
    std::function<void (Combination)> f
  );
  BDD Reduce(
    const World &world, 
    const std::vector<std::vector<std::size_t>> &instantiations,
    const std::vector<Literal> &literals,
    const std::vector<std::size_t> &comb,
    const BDD &bdd
  );
  void Expand(
    const World &world, 
    const std::vector<Literal> &p_literals,
    const std::vector<std::size_t> &p_comb,
    const BDD &valid,
    const BDD &invalid,
    std::function<void (Combination)> f
  );
};

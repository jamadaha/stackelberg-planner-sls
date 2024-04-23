#include "combination.h"

Literal::Literal(
  std::size_t predicate,
  const std::vector<std::size_t> &&params
) :
    predicate(std::move(predicate)),
    params(std::move(params)) 
{}


Combination::Combination(
  std::size_t applicable, 
  std::size_t invalid, 
  std::vector<std::size_t> params, 
  std::vector<Literal> literals
) : 
    applicable(std::move(applicable)),
    invalid(std::move(invalid)),
    params(std::move(params)),
    literals(std::move(literals))
{}

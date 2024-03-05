#ifndef SEARCH_BDD_TREE_H
#define SEARCH_BDD_TREE_H

#include "cuddObj.hh"

std::vector<std::vector<size_t>> comb(int N, int K)
{
  std::string bitmask(K, 1); // K leading 1's
  bitmask.resize(N, 0); // N-K trailing 0's

  std::vector<std::vector<size_t>> combinations;
  // print integers and permute bitmask
  do {
    std::vector<size_t> combination;
    for (int i = 0; i < N; ++i) // [0..N-1] integers
      if (bitmask[i])
        combination.push_back(i);
    combinations.push_back(std::move(combination));
  } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

  return combinations;
}

template <class T>
class BDDTree {
public:
    explicit BDDTree(std::shared_ptr<symbolic::SymVariables> vars) : vars(std::move(vars)) {}
    size_t RootSize() { return root.size(); }
    void AddRoot(T key, BDD bdd) {
      root.push_back({key, bdd});
    }
    std::vector<std::pair<std::vector<T>, size_t>> Generate() {
      std::vector<std::pair<std::vector<T>, size_t>> result;

      for (size_t i = 1; i < 5; i++) {
        const auto combinations = comb(root.size(), i);
        for (const auto &c : combinations) {
          std::vector<T> key{root[c[0]].first};
          BDD bdd = root[c[0]].second;
          for (size_t t = 1; t < c.size(); t++) {
            key.push_back(root[c[t]].first);
            bdd &= root[c[t]].second;
          }
          result.push_back({std::move(key), vars->numStates(bdd)});
        }
      }

      return result;
    }
private:
    std::shared_ptr<symbolic::SymVariables> vars;
    std::vector<std::pair<T, BDD>> root;
};


#endif //SEARCH_BDD_TREE_H

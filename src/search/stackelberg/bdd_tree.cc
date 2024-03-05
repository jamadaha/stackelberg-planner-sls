#include <algorithm>
#include <iostream>
#include "bdd_tree.h"

using namespace std;

template <class T>
void BDDTree<T>::AddRoot(T key, BDD bdd) {
    root.push_back({key, bdd});
}

template <class T>
std::vector<std::pair<std::vector<T>, BDD>> BDDTree<T>::Generate() {
    std::vector<std::pair<std::vector<T>, BDD>> result;

  std::string bitmask(root.size(), 1);
  bitmask.resize(root.size(), 0);

  do {

    for (int i = 0; i < root.size(); ++i)
    {
      if (bitmask[i]) std::cout << " " << i;
    }
    std::cout << std::endl;
  } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

    return result;
}
#ifndef SEARCH_BDD_TREE_H
#define SEARCH_BDD_TREE_H

#include "cuddObj.hh"

template <class T>
class BDDTree {
public:
    void AddRoot(T key, BDD bdd);
    std::vector<std::pair<std::vector<T>, BDD>> Generate();
private:
    std::vector<std::pair<T, BDD>> root;
};


#endif //SEARCH_BDD_TREE_H

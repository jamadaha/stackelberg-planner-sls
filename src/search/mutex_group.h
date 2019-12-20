#ifndef MUTEX_GROUP_H
#define MUTEX_GROUP_H

#include <iostream>
#include <vector>

struct FactPair {
    int var;
    int value;

    FactPair(int var, int value)
        : var(var), value(value) {
    }

    bool operator<(const FactPair &other) const {
        return var < other.var || (var == other.var && value < other.value);
    }

    bool operator==(const FactPair &other) const {
        return var == other.var && value == other.value;
    }

    bool operator!=(const FactPair &other) const {
        return var != other.var || value != other.value;
    }

    std::pair<int, int> get_pair() const {
        return std::make_pair (var, value);
    }
};

std::ostream &operator<<(std::ostream &os, const FactPair &fact_pair);

class MutexGroup {
  bool detected_fw;
  bool exactly_one; 

  std::vector<FactPair> facts;
 public:
  MutexGroup(std::istream &in);

  void dump() const;

  //  void set_exactly_invariant(); (Already done in preprocess)

  bool hasPair(int var, int val) const;

  inline const std::vector<FactPair> & getFacts() const{
    return facts;
  }

  inline bool detectedFW() const{
    return detected_fw;
  }

  //If the mutex was detected bw is used to prune fw search and vice versa
  inline bool pruneFW() const{
    return !detected_fw;
  }


  inline bool isExactlyOne() const{
    return exactly_one;
  }
  friend std::ostream & operator<<(std::ostream &os, const MutexGroup & mg);
};

#endif

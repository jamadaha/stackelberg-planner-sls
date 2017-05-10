#ifndef PER_STATE_STORAGE_H
#define PER_STATE_STORAGE_H

#include "../global_state.h"
#include "../segmented_vector.h"

#include <cstdlib>

namespace second_order_search
{

template<typename T>
class PerStateStorage
{
private:
    SegmentedVector<T> m_data;
    T _default;
public:
    PerStateStorage(const T &d = T());
    T &operator[](const GlobalState &state);
    T &operator[](const StateID &state);
    T &operator[](const size_t &state);
    size_t size() const;
};

}

#include "per_state_storage.cc"

#endif

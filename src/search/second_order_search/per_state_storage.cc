
#ifdef PER_STATE_STORAGE_H

namespace second_order_search
{

template<typename T>
PerStateStorage<T>::PerStateStorage(const T &d)
    : _default(d)
{}

template<typename T>
T &PerStateStorage<T>::operator[](const GlobalState &state)
{
    return this->operator[](state.get_id().hash());
}

template<typename T>
T &PerStateStorage<T>::operator[](const StateID &state)
{
    return this->operator[](state.hash());
}

template<typename T>
T &PerStateStorage<T>::operator[](const size_t &state)
{
    if (state >= m_data.size()) {
        m_data.resize(state + 1, _default);
    }
    return m_data[state];
}

template<typename T>
size_t PerStateStorage<T>::size() const
{
    return m_data.size();
}

}

#endif

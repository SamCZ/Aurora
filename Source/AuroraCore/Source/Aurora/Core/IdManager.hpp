#pragma once

#include <boost/numeric/interval.hpp>
#include <limits>
#include <set>
#include <iostream>

#include "SmartPointer.hpp"

namespace Aurora
{
    template<typename T>
    class id_interval
    {
    public:
        id_interval(T ll, T uu) : value_(ll,uu)  {}
        bool operator < (const id_interval& s) const
        {
            return
                    (value_.lower() < s.value_.lower()) &&
                    (value_.upper() < s.value_.lower());
        }
        T left() const { return value_.lower(); }
        T right() const {  return value_.upper(); }
    private:
        boost::numeric::interval<T> value_;
    };

    template<typename T>
    class IdManager {
    public:
        IdManager() : free_()
        {
            free_.insert(id_interval<T>(1, std::numeric_limits<T>::max()));
        }

        T AllocateId()
        {
            id_interval<T> first = *(free_.begin());
            T free_id = first.left();
            free_.erase(free_.begin());
            if (first.left() + 1 <= first.right()) {
                free_.insert(id_interval<T>(first.left() + 1 , first.right()));
            }
            return free_id;
        }

        void FreeId(T id)
        {
            auto it = free_.find(id_interval<T>(id,id));
            if (it != free_.end()  && it->left() <= id && it->right() > id) {
                return ;
            }
            it = free_.upper_bound(id_interval<T>(id,id));
            if (it == free_.end()) {
                return ;
            } else {
                id_interval<T> free_interval = *(it);

                if (id + 1 != free_interval.left()) {
                    free_.insert(id_interval<T>(id, id));
                } else {
                    if (it != free_.begin()) {
                        auto it_2 = it;
                        --it_2;
                        if (it_2->right() + 1 == id ) {
                            id_interval<T> free_interval_2 = *(it_2);
                            free_.erase(it);
                            free_.erase(it_2);
                            free_.insert(
                                    id_interval<T>(free_interval_2.left(),
                                                free_interval.right()));
                        } else {
                            free_.erase(it);
                            free_.insert(id_interval<T>(id, free_interval.right()));
                        }
                    } else {
                        free_.erase(it);
                        free_.insert(id_interval<T>(id, free_interval.right()));
                    }
                }
            }
        }

        bool MarkAsUsed(T id)
        {
            auto it = free_.find(id_interval(id,id));
            if (it == free_.end()) {
                return false;
            } else {
                id_interval<T> free_interval = *(it);
                free_.erase (it);
                if (free_interval.left() < id) {
                    free_.insert(id_interval<T>(free_interval.left(), id-1));
                }
                if (id +1 <= free_interval.right() ) {
                    free_.insert(id_interval<T>(id+1, free_interval.right()));
                }
                return true;
            }
        }
    private:
        typedef std::set<id_interval<T>> id_intervals_t;
        id_intervals_t free_;
    };
}
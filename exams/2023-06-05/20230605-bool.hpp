#ifndef _20230605_HPP_
#define _20230605_HPP_

#include <cstring>
#include <sys/types.h>


template <int N>
u_int32_t h(int k, u_int32_t x)
{
    auto f = x;
    auto g = (x << 16) | (x >> 16);
    return ( f + k*g ) % N;
}




template <int N, int K>
class BloomFilter
{
private:
    bool _bf[N];

public:
    BloomFilter()
    {
        for(auto &i : _bf) 
        {
            i = false;
        } 
    }

    BloomFilter(const BloomFilter&) = default;
    BloomFilter& operator=(const BloomFilter&) = default;
    BloomFilter(BloomFilter&&) = default;
    BloomFilter& operator=(BloomFilter&&) = default;
    ~BloomFilter() = default;


      
    void insert(u_int32_t x)
    {
        for (auto k=0; k < K; ++k)
        {
            auto idx = h<N>(k,x);
            _bf[ idx ] = true;
        }
    }

    bool query(u_int32_t x) const
    {
        for (auto k=0; k < K; ++k)
        {
            auto idx = h<N>(k,x);
            if ( _bf[ idx ] == 0 ) return false;
        }
        return true;
    }
};


#endif
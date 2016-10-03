// g++ -O3  -march=native -std=c++11 -o pointersort pointersort.cpp -I .
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <numeric>
#include <vector>
#include <algorithm>
#include <timsort.hpp>

#define RDTSC_START(cycles)                                                   \
    do {                                                                      \
        unsigned cyc_high, cyc_low;                                  \
        __asm volatile(                                                       \
            "cpuid\n\t"                                                       \
            "rdtsc\n\t"                                                       \
            "mov %%edx, %0\n\t"                                               \
            "mov %%eax, %1\n\t"                                               \
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx"); \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                      \
    } while (0)

#define RDTSC_FINAL(cycles)                                                   \
    do {                                                                      \
        unsigned cyc_high, cyc_low;                                  \
        __asm volatile(                                                       \
            "rdtscp\n\t"                                                      \
            "mov %%edx, %0\n\t"                                               \
            "mov %%eax, %1\n\t"                                               \
            "cpuid\n\t"                                                       \
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx"); \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                      \
    } while (0)




/*
 * Prints the best number of operations per cycle where
 * test is the function call, answer is the expected answer generated by
 * test, repeat is the number of times we should repeat and size is the
 * number of operations represented by test.
 */
#define BEST_TIME(test,  pre, repeat, size)                         \
        do {                                                              \
            printf("%40s [%40s]: ", #test, #pre);                                        \
            fflush(NULL);                                                 \
            uint64_t cycles_start, cycles_final, cycles_diff;             \
            uint64_t min_diff = (uint64_t)-1;                             \
            for (int i = 0; i < repeat; i++) {                            \
                pre;                                                       \
                __asm volatile("" ::: /* pretend to clobber */ "memory"); \
                RDTSC_START(cycles_start);                                \
                test;                                                     \
                RDTSC_FINAL(cycles_final);                                \
                cycles_diff = (cycles_final - cycles_start);              \
                if (cycles_diff < min_diff) min_diff = cycles_diff;       \
            }                                                             \
            uint64_t S = size;                                            \
            float cycle_per_op = (min_diff) / (double)S;                  \
            printf(" %.2f cycles per element", cycle_per_op);           \
            printf("\n");                                                 \
            fflush(NULL);                                                 \
 } while (0)

#define BEST_TIME_COND(test, expected,  pre, repeat, size)                         \
        do {                                                              \
            printf("%40s [%40s]: ", #test, #pre);                                        \
            fflush(NULL);                                                 \
            uint64_t cycles_start, cycles_final, cycles_diff;             \
            uint64_t min_diff = (uint64_t)-1;                             \
            for (int i = 0; i < repeat; i++) {                            \
                pre;                                                       \
                __asm volatile("" ::: /* pretend to clobber */ "memory"); \
                RDTSC_START(cycles_start);                                \
                if(test!= expected) printf("bug");                         \
                RDTSC_FINAL(cycles_final);                                \
                cycles_diff = (cycles_final - cycles_start);              \
                if (cycles_diff < min_diff) min_diff = cycles_diff;       \
            }                                                             \
            uint64_t S = size;                                            \
            float cycle_per_op = (min_diff) / (double)S;                  \
            printf(" %.2f cycles per element", cycle_per_op);           \
            printf("\n");                                                 \
            fflush(NULL);                                                 \
 } while (0)

template <class T>
struct pointer_cmp {
    bool operator()(T *a, T *b)
    {
        return *a < *b;
    }
};


void demo(int size) {
    printf("size = %d values \n",size);
    int repeat = 500;
    std::vector<uint32_t> v(size);
    for(uint32_t i = 0; i < size; ++i) v[i] = i;
    std::vector<uint32_t> t(size);
    for(uint32_t i = 0; i < size; ++i) t[i] = i;
    std::vector<std::string> s(size);
    for(uint32_t i = 0; i < size; ++i) s[i] = i;
    std::vector<std::string> sp(size);
    for(uint32_t i = 0; i < size; ++i) {
      sp[i] = i;
      sp[i].resize(8,' ');
    }
    std::vector<uint32_t*> pv(size);
    for(uint32_t i = 0; i < size; ++i) {
      pv[i] = & t[i];
    }
    pointer_cmp<uint32_t> cmp;

    std::vector<uint32_t> buffer(size);
    std::vector<uint32_t> pbuffer(size);
    std::vector<std::string> sbuffer(size);


    BEST_TIME(buffer.assign(v.begin(), v.end()),, repeat, size);


    BEST_TIME_COND(std::is_sorted(v.begin(), v.end()),true,std::sort(v.begin(), v.end()), repeat, size);
    BEST_TIME_COND(std::is_sorted(pv.begin(), pv.end(),cmp),true,std::sort(pv.begin(), pv.end(),cmp), repeat, size);
    BEST_TIME_COND(std::is_sorted(s.begin(), s.end()),true,std::sort(s.begin(), s.end()), repeat, size);
    BEST_TIME_COND(std::is_sorted(sp.begin(), sp.end()),true,std::sort(sp.begin(), sp.end()), repeat, size);
   
    BEST_TIME(std::sort(v.begin(), v.end()),std::sort(v.begin(), v.end()), repeat, size);
    BEST_TIME(std::sort(pv.begin(), pv.end(),cmp),std::sort(pv.begin(), pv.end(),cmp), repeat, size);
    BEST_TIME(std::sort(s.begin(), s.end()),std::sort(s.begin(), s.end()), repeat, size);
    BEST_TIME(std::sort(sp.begin(), sp.end()),std::sort(sp.begin(), sp.end()), repeat, size);
   
    BEST_TIME(std::stable_sort(v.begin(), v.end()),std::sort(v.begin(), v.end()), repeat, size);
    BEST_TIME(std::stable_sort(pv.begin(), pv.end(),cmp),std::sort(pv.begin(), pv.end(),cmp), repeat, size);

    BEST_TIME(gfx::timsort(v.begin(), v.end()),std::sort(v.begin(), v.end()), repeat, size);
    BEST_TIME(gfx::timsort(pv.begin(), pv.end(),cmp),std::sort(pv.begin(), pv.end(),cmp), repeat, size);
    BEST_TIME(gfx::timsort(s.begin(), s.end()),std::sort(s.begin(), s.end()), repeat, size);


    BEST_TIME(std::sort(v.begin(), v.end()),std::random_shuffle(v.begin(), v.end()), repeat, size);
    BEST_TIME(std::sort(pv.begin(), pv.end(),cmp),std::random_shuffle(pv.begin(), pv.end()), repeat, size);
    BEST_TIME(std::sort(s.begin(), s.end()),std::random_shuffle(s.begin(), s.end()), repeat, size);
    BEST_TIME(std::sort(sp.begin(), sp.end()),std::random_shuffle(sp.begin(), sp.end()), repeat, size);



    BEST_TIME_COND(std::is_sorted(v.begin(), v.end()),false,std::random_shuffle(v.begin(), v.end()), repeat, size);
    BEST_TIME_COND(std::is_sorted(pv.begin(), pv.end(),cmp),false,std::random_shuffle(pv.begin(), pv.end()), repeat, size);

    BEST_TIME(std::stable_sort(v.begin(), v.end()),std::random_shuffle(v.begin(), v.end()), repeat, size);
    BEST_TIME(std::stable_sort(pv.begin(), pv.end(),cmp),std::random_shuffle(pv.begin(), pv.end()), repeat, size);

    BEST_TIME(gfx::timsort(v.begin(), v.end()),std::random_shuffle(v.begin(), v.end()), repeat, size);
    BEST_TIME(gfx::timsort(pv.begin(), pv.end(),cmp),std::random_shuffle(pv.begin(), pv.end()), repeat, size);
    BEST_TIME(gfx::timsort(s.begin(), s.end()),std::random_shuffle(s.begin(), s.end()), repeat, size);


    BEST_TIME(std::sort(v.begin(), v.end()),std::sort(v.rbegin(), v.rend()), repeat, size);
    BEST_TIME(std::sort(pv.begin(), pv.end()),std::sort(pv.rbegin(), pv.rend(),cmp), repeat, size);

    BEST_TIME(std::stable_sort(v.begin(), v.end()),std::sort(v.rbegin(), v.rend()), repeat, size);
    BEST_TIME(std::stable_sort(pv.begin(), pv.end(),cmp),std::sort(pv.rbegin(), pv.rend(),cmp), repeat, size);

    BEST_TIME(gfx::timsort(v.begin(), v.end()),std::sort(v.rbegin(), v.rend()), repeat, size);
    BEST_TIME(gfx::timsort(pv.begin(), pv.end(),cmp),std::sort(pv.rbegin(), pv.rend(),cmp), repeat, size);

    printf("\n");
}

int main() {
    demo(1024);
    demo(1<<16);
    demo(1000000);
    return 0;
}

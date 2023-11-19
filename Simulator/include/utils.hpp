#pragma once
#include<bits/stdc++.h>

#define ALWAYS_INLINE inline __attribute__((always_inline)) 
#define UNROLL __attribute__((optimize("unroll-loops")))
#define NOINLINE __attribute__((noinline))

#define all(x) begin(x), end(x)

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;
using i64 = std::int64_t;
using i32 = std::int32_t;
using i16 = std::int16_t;
using i8 = std::int8_t;

#define REP(i,a,b) for(int i=(a),i##_end_=(b);i<i##_end_;++i)
#define REP_EQ(i,a,b) for(int i=(a),i##_end_=(b);i<=i##_end_;++i)
#define DREP(i,a,b) for(int i=(a),i##_end_=(b);i>i##_end_;--i)
#define DREP_EQ(i,a,b) for(int i=(a),i##_end_=(b);i>=i##_end_;--i)

#define __lg(x) (31 - __builtin_clz(x))
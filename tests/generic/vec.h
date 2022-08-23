// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#ifndef __TESTS_VEC_H__
#define __TESTS_VEC_H__

#define GENERIC_TYPE int
#define GENERIC_NAME IntVec
#define GENERIC_PREFIX int_vec
#include <ds/generic/vec.h>

#define GENERIC_TYPE IntVec
#define GENERIC_NAME IntVec2D
#define GENERIC_PREFIX int_vec_2d
#define GENERIC_TERM_ITEM(_itm) int_vec_term(_itm)
#include <ds/generic/vec.h>

#endif

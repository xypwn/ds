// Copyright 2022 Darwin Schuppan <darwin@nobrain.org>
// SPDX license identifier: MIT

#ifndef __DS_NUMBER_H__
#define __DS_NUMBER_H__

/* Returns the smaller/larger value of x and y. */
#define min(x, y) ((x) < (y) ? (x) : y)
#define max(x, y) ((x) > (y) ? (x) : y)

/* Tries to calculate x - y. If the result would be negative, returns 0 instead.
 * Can safely be used on unsigned types like size_t. */
#define sub_clamped(x, y) ((x) > (y) ? (x) - (y) : 0)

#endif

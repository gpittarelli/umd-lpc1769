/*
 * assert.h
 * Author: George Pittarelli
 *
 * Implements compile time assertions with ct_assert.
 * Will be displayed by the compiler as a divide-by-0 error
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define CT_ASSERT(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }

#endif /* ASSERT_H_ */

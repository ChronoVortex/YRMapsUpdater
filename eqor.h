/**
 * @file eqor.h
 * @brief Function to check if a value is equal to any of a set of values.
 * @author Chrono Vortex#9916@Discord
 */

#pragma once

/**
 * Check if a value is equal to any of a set of values.
 *
 * @param a value to check.
 * @param values list of values to check 'a' against.
 * @return true 'a' is equal to any entry in 'values', false if not.
 */
template <class Comparable>
bool _eqor(Comparable a, std::initializer_list<Comparable> values) {
	for (Comparable e : values)
		if (a == e)
			return true;
	return false;
}

// eliminate need for braces when calling
#define eqor(A, ...) _eqor(A, std::initializer_list<decltype(A)>{__VA_ARGS__})

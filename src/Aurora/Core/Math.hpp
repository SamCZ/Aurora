#pragma once

namespace Aurora::Math
{
	/**
	 * Returns the largest (closest to positive infinity)
	 * @param x the dividend
	 * @param y the divisor
	 * @return the largest (closest to positive infinity)
	 */
	inline int FloorDiv(int x, int y) {
		int r = x / y;
		// if the signs are different and modulo not zero, round down
		if ((x ^ y) < 0 && (r * y != x)) {
			r--;
		}
		return r;
	}

	/**
	 * Returns the largest (closest to positive infinity)
	 * @param x the dividend
	 * @param y the divisor
	 * @return the largest (closest to positive infinity)
	 */
	inline long FloorDiv(long x, long y) {
		long r = x / y;
		// if the signs are different and modulo not zero, round down
		if ((x ^ y) < 0 && (r * y != x)) {
			r--;
		}
		return r;
	}

	/**
	 * Returns the floor modulus of the {@code int} arguments.
	 * @param x the dividend
	 * @param y the divisor
	 * @return the floor modulus {@code x - (FloorDiv(x, y) * y)}
	 */
	inline int FloorMod(int x, int y) {
		int r = x - floorDiv(x, y) * y;
		return r;
	}

	/**
	 * Returns the floor modulus of the {@code int} arguments.
	 * @param x the dividend
	 * @param y the divisor
	 * @return the floor modulus {@code x - (FloorDiv(x, y) * y)}
	 */
	inline long FloorMod(long x, long y) {
		return x - floorDiv(x, y) * y;
	}
}
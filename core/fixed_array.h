#ifndef FIXED_ARRAY_H
#define FIXED_ARRAY_H

#include "core/local_vector.h"

// High performance fixed size array, single threaded.
// Especially useful if you need to create an array on the stack, to
// prevent dynamic allocations (especially in bottleneck code).

template <class T, uint32_t CAPACITY = 8, bool force_trivial = false, uint32_t ALIGN = 1>
class FixedArray {
	static_assert(ALIGN > 0, "ALIGN must be at least 1.");
	const static uint32_t UNIT_SIZE = ((sizeof(T) + ALIGN - 1) / ALIGN * ALIGN);
	const static bool CONSTRUCT = !std::is_trivially_constructible<T>::value && !force_trivial;
	const static bool DESTRUCT = !std::is_trivially_destructible<T>::value && !force_trivial;

	uint32_t _size = 0;
	uint8_t _data[CAPACITY * UNIT_SIZE];

	const T &get(uint32_t p_index) const {
		return *(const T *)&_data[p_index * UNIT_SIZE];
	}
	T &get(uint32_t p_index) {
		return *(T *)&_data[p_index * UNIT_SIZE];
	}

public:
	uint32_t size() const { return _size; }
	bool is_empty() const { return !_size; }
	bool is_full() const { return _size == CAPACITY; }
	uint32_t capacity() const { return CAPACITY; }

	T *request(bool p_construct = true) {
		if (size() < CAPACITY) {
			T *ele = &get(_size++);
			if (CONSTRUCT && p_construct) {
				memnew_placement(ele, T);
			}
			return ele;
		}
		return nullptr;
	}
	void push_back(const T &p_val) {
		T *mem = request(false);
		ERR_FAIL_NULL(mem);
		*mem = p_val;
	}
	void clear() {
		resize(0);
	}
	void remove_unordered(uint32_t p_index) {
		ERR_FAIL_UNSIGNED_INDEX(p_index, _size);

		_size--;
		if (_size > p_index) {
			get(p_index) = get(_size);
		}

		if (DESTRUCT) {
			get(_size).~T();
		}
	}
	void resize(uint32_t p_size) {
		ERR_FAIL_COND(p_size > CAPACITY);

		if (DESTRUCT && (p_size < _size)) {
			for (uint32_t i = p_size; i < _size; i++) {
				get(i).~T();
			}
		}

		if (CONSTRUCT && (p_size > _size)) {
			for (uint32_t i = _size; i < p_size; i++) {
				memnew_placement(&get(i), T);
			}
		}

		_size = p_size;
	}
	const T &operator[](uint32_t p_index) const {
		DEV_ASSERT(p_index < size());
		return get(p_index);
	}
	T &operator[](uint32_t p_index) {
		DEV_ASSERT(p_index < size());
		return get(p_index);
	}

	operator Vector<T>() const {
		Vector<T> ret;
		if (size()) {
			ret.resize(size());
			T *dest = ret.ptrw();
			if (ALIGN <= 1) {
				memcpy(dest, _data, sizeof(T) * _size);
			} else {
				for (uint32_t n = 0; n < _size; n++) {
					dest[n] = get(n);
				}
			}
		}
		return ret;
	}
};

#endif // FIXED_ARRAY_H

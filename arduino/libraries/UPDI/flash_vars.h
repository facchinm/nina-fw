#ifndef FLASH_VARS_H_
#define FLASH_VARS_H_

#define FLASH(T) const flash<T>

template <typename T>
class flash {
	private:
	const T data;

	public:
	// normal constructor
	constexpr flash (T _data) : data(_data) {}
	// default constructor
	constexpr flash () : data(0) {}

	operator T() const {
		switch (sizeof(T)) {
			case 1: return data;
			case 2: return data;
			case 4: return data;
		}
	}
};

#endif /* FLASH_VARS_H_ */

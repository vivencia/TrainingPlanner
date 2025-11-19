#pragma once

class TPBool
{

public:
	// Default constructor initializes the value to false
	inline TPBool() : m_value(false) {}

	// Constructor to initialize with a specific boolean value
	inline explicit TPBool(bool val) : m_value{val} {}

	// Operator overloading to allow implicit conversion to bool
	inline operator bool() const {
		return m_value;
	}

	inline bool operator=(bool val) { m_value = val; return m_value; }

private:
	bool m_value;
};

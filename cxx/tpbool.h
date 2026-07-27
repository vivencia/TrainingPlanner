#pragma once

class	TPBool
{
public:
	inline TPBool() {}
	inline explicit TPBool(bool val) : m_value{val} {}
	inline operator bool() const { return m_value; } // Operator overloading to allow implicit conversion to bool
	inline bool operator()() const { return m_value; }
	inline bool operator=(const bool val) { m_value = val; return m_value; }

private:
	bool m_value{false};
};

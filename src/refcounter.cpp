#include <new>
#include "refcounter.h"

size_t RefCounter::count() {
	if (m_count) return *m_count;
	else return -1;
}

RefCounter::RefCounter() : m_count{ new size_t{ 0 } } {}

RefCounter::RefCounter(const RefCounter& src) : m_count{ src.m_count } {
	(*m_count)++;
}

RefCounter::RefCounter(RefCounter&& src) noexcept : m_count{ src.m_count } {
	src.m_count = nullptr;
}

RefCounter& RefCounter::operator=(const RefCounter& src) {
	this->~RefCounter();
	new (this) RefCounter{ src };
	return *this;
}

RefCounter& RefCounter::operator=(RefCounter&& src) noexcept {
	this->~RefCounter();
	new (this) RefCounter{ std::move(src) };
	return *this;
}

RefCounter::~RefCounter() {
	if (m_count) {
		if (*m_count == 0) delete m_count;
		else (*m_count)--;
	}
}
#ifndef REFCOUNTER_H
#define REFCOUNTER_H

class RefCounter {
	public:
		size_t count();

		RefCounter();
		RefCounter(const RefCounter& src);
		RefCounter(RefCounter&& src) noexcept;
		RefCounter& operator=(const RefCounter& src);
		~RefCounter();

	private:
		size_t* m_count;
};

#endif
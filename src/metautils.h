#ifndef METAUTILS_H
#define METAUTILS_H

template<class...Fs>
struct overload : Fs... {
	using Fs::operator()...;
};

#endif
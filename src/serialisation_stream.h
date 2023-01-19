#pragma once

#include <fstream>
#include <filesystem>
#include <zlib.h>
#include <concepts>

namespace fs = std::filesystem;

template<typename sizeT>
class SerialisationStream: public std::ostream, private std::streambuf {
private:
	std::ofstream inner;

public:
	SerialisationStream(const char* filename)
		: std::ostream(this), inner(filename, std::ios::binary | std::ios::out | std::ios::trunc) {
	}

	bool is_open() const {
		return inner.is_open();
	}

	virtual std::streamsize xsputn(const char* s, std::streamsize n);

	/// <summary>
	/// Closes the stream if it is open.
	/// </summary>
	void close() {
		if (inner.is_open())
			inner.close();
	}

	~SerialisationStream() {
		if (inner.is_open())
			inner.close();
	}

	void write_size(sizeT size);
	void write_string(const std::string& string);
	void write_path(const fs::path& path);

	/// <summary>
	/// Writes a scalar type to the stream. Should be a simple POD type.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="scalar"></param>
	template<typename T>
	void write_scalar(const T& scalar);
	
	typedef sizeT size_type;
};

typedef SerialisationStream<uint16_t> SeStream;




template<typename sizeT, typename streamT>
requires std::derived_from<streamT, std::istream>
class DeSerialisationStream : public std::istream, private std::streambuf {
private:
	streamT inner;

public:
	DeSerialisationStream(streamT&& stream)
		: std::istream(this), inner(std::move(stream)) {
	}

	bool is_open() const {
		return inner.is_open();
	}

	virtual std::streamsize xsgetn(char* s, std::streamsize n);

	/// <summary>
	/// Closes the stream if it is open.
	/// </summary>
	void close() {
		if (inner.is_open())
			inner.close();
	}

	~DeSerialisationStream() {
		if (inner.is_open())
			inner.close();
	}

	/// <summary>
	/// Calculates the file size (unfortunately not `const`)
	/// </summary>
    size_t filesize() {
        inner.seekg(0, inner.end);
        size_t length = inner.tellg();
        inner.seekg(0, inner.beg);
        return length;
    }

	size_t read_size();
	fs::path read_path();
	std::string read_string();

	template<typename T>
	T read_scalar();

	typedef sizeT size_type;
};

typedef DeSerialisationStream<uint16_t> DeSeStream;

template<typename sizeT>
inline std::streamsize SerialisationStream<sizeT>::xsputn(const char* s, std::streamsize n)
{
	inner.write(s, n);
	return n;
}

template<typename sizeT>
inline void SerialisationStream<sizeT>::write_size(sizeT size)
{
	sizeT s = static_cast<sizeT>(size);
	inner.write(reinterpret_cast<char*>(&size), sizeof(sizeT));
}

template<typename sizeT>
inline void SerialisationStream<sizeT>::write_path(const fs::path& path)
{
	write_string(path.string());
}

template<typename sizeT>
inline void SerialisationStream<sizeT>::write_string(const std::string& string)
{
	sizeT size = static_cast<sizeT>(string.size());
	inner.write(reinterpret_cast<char*>(&size), sizeof(sizeT));
	inner.write(&string[0], size);
}

template<typename sizeT>
inline std::streamsize DeSerialisationStream<sizeT>::xsgetn(char* s, std::streamsize n)
{
	inner.read(s, n);
	return n;
}


template<typename sizeT>
inline size_t DeSerialisationStream<sizeT>::read_size() {
	sizeT size;
	inner.read((char*)&size, sizeof(sizeT));
	return static_cast<size_t>(size);
}

template<typename sizeT>
inline fs::path DeSerialisationStream<sizeT>::read_path()
{
	return fs::path(read_string());
}

template<typename sizeT>
inline std::string DeSerialisationStream<sizeT>::read_string()
{
	size_t size = read_size();
	std::string str;
	str.resize(size);
	inner.read(&str[0], size);
	return str;
}

template<typename sizeT>
template<typename T>
inline T DeSerialisationStream<sizeT>::read_scalar()
{
	T scalar;
	inner.read(reinterpret_cast<char*>(&scalar), sizeof(T));
	return scalar;
}

template<typename sizeT>
template<typename T>
inline void SerialisationStream<sizeT>::write_scalar(const T& scalar)
{
	inner.write(reinterpret_cast<const char*>(&scalar), sizeof(T));
}

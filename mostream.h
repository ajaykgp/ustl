// This file is part of the ustl library, an STL implementation.
// Copyright (C) 2003 by Mike Sharov <msharov@talentg.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the 
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
// Boston, MA  02111-1307  USA.
//
// mostream.h

#ifndef MOSTREAM_H
#define MOSTREAM_H

#include "memlink.h"

namespace ustl {

class istream;

///
/// \brief Helper class to write packed binary streams.
///
/// This class contains a set of functions to write integral types into an
/// unstructured memory block. Packing binary file data can be done this
/// way, for instance. aligning the data is your responsibility, and can
/// be accomplished by proper ordering of writes and by calling the align()
/// function. Unaligned access is usually slower by orders of magnitude and,
/// on some architectures, such as PowerPC, can cause your program to crash.
/// Therefore, all write functions have asserts to check alignment.
/// Overwriting the end of the stream will also cause a crash (an assert in
/// debug builds). Oh, and don't be intimidated by the size of the inlines
/// here. In the assembly code the compiler will usually chop everything down
/// to five instructions each.
///
/// Example code:
/// \code
///     CMemoryBlock b;
///     ostream os (b);
///     os << boolVar;
///     os.align (sizeof(int));
///     os << intVar << floatVar;
///     os.write (binaryData, binaryDataSize);
///     os.align (sizeof(u_long));
///     b.SetSize (os.pos());
///     write (fd, b, b.size());
/// \endcode
///
class ostream : public memlink {
public:
			ostream (void);
			ostream (void* p, size_t n);
    explicit		ostream (const memlink& source);
    virtual void	unlink (void);
    inline void		seek (uoff_t newPos);
    inline void		skip (size_t nBytes);
    inline uoff_t	pos (void) const;
    inline size_t	remaining (void) const;
    inline bool		aligned (size_t grain = c_DefaultAlignment) const;
    inline void		align (size_t grain = c_DefaultAlignment);
    void		write (const void* buffer, size_t size);
    inline void		write (const cmemlink& buf);
    void		read (istream& is);
    void		write (ostream& os) const;
    void		insert (iterator start, size_t size);
    void		erase (iterator start, size_t size);
    void		swap (ostream& os);
    inline size_t	stream_size (void) const;
    template <typename T>
    inline void		iwrite (const T& v);
    template <typename T>
    inline ostream&	operator<< (T* v);
    template <typename T>
    inline ostream&	operator<< (const T* v);
    inline ostream&	operator<< (char v);
    inline ostream&	operator<< (short v);
    inline ostream&	operator<< (int v);
    inline ostream&	operator<< (long v);
    inline ostream&	operator<< (u_char v);
    inline ostream&	operator<< (u_short v);
    inline ostream&	operator<< (u_int v);
    inline ostream&	operator<< (u_long v);
    inline ostream&	operator<< (float v);
    inline ostream&	operator<< (double v);
    inline ostream&	operator<< (bool v);
    inline ostream&	operator<< (wchar_t v);
private:
    uoff_t		m_Pos;	///< Current write position.
};

//----------------------------------------------------------------------

/// An iterator over an ostream to use with uSTL algorithms.
template <class T>
class ostream_iterator {
public:
    typedef T			value_type;
    typedef ptrdiff_t		difference_type;
    typedef value_type*		pointer;
    typedef value_type&		reference;
public:
    explicit			ostream_iterator (ostream& os)
				    : m_Os (os) {}
 				ostream_iterator (const ostream_iterator& i)
				    : m_Os (i.m_Os) {} 
    /// Writes \p v into the stream.
    inline ostream_iterator&	operator= (const T& v)
				    { m_Os << v; return (*this); }
    inline ostream_iterator&	operator* (void) { return (*this); }
    inline ostream_iterator&	operator++ (void) { return (*this); }
    inline ostream_iterator	operator++ (int) { return (*this); }
    inline bool			operator== (const ostream_iterator& i) const
				    { return (m_Os.pos() == i.m_Os.pos()); }
    inline bool			operator< (const ostream_iterator& i) const
				    { return (m_Os.pos() < i.m_Os.pos()); }
private:
    ostream&	m_Os;
};

//----------------------------------------------------------------------

/// Returns the current write position. Usually this is also the number of bytes written.
inline uoff_t ostream::pos (void) const
{
    return (m_Pos);
}

/// Move the write pointer to \p newPos
inline void ostream::seek (uoff_t newPos)
{
    assert (newPos <= size());
    m_Pos = newPos;
}

/// Skips \p nBytes without writing anything.
inline void ostream::skip (size_t nBytes)
{
    seek (pos() + nBytes);
}

/// Returns number of bytes remaining in the write buffer.
inline size_t ostream::remaining (void) const
{
    return (size() - pos());
}

/// Returns \c true if the write pointer is aligned on \p grain
inline bool ostream::aligned (size_t grain) const
{
    return (pos() % grain == 0);
}

/// Aligns the write pointer on \p grain. Nothing is written to the skipped bytes.
inline void ostream::align (size_t grain)
{
    seek (Align (pos(), grain));
}

/// Writes the contents of \p buf into the stream as a raw dump.
inline void ostream::write (const cmemlink& buf)
{
    write (buf.begin(), buf.size());
}

/// Returns number of bytes written.
inline size_t ostream::stream_size (void) const
{
    return (pos());
}

/// Writes type T into the stream via a direct pointer cast.
template <typename T>
inline void ostream::iwrite (const T& v)
{
    assert (aligned (sizeof(T) < c_DefaultAlignment ? sizeof(T) : c_DefaultAlignment));
    assert (remaining() >= sizeof(T));
    void* pv = begin() + pos();
    *reinterpret_cast<T*>(pv) = v;
    skip (sizeof(T));
}

template <typename T>
inline ostream&	ostream::operator<< (T* v)	{ iwrite(v); return (*this); }
template <typename T>
inline ostream&	ostream::operator<< (const T* v){ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (char v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (short v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (int v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (long v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (u_char v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (u_short v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (u_int v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (u_long v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (float v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (double v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (bool v)	{ iwrite(v); return (*this); }
inline ostream&	ostream::operator<< (wchar_t v)	{ iwrite(v); return (*this); }

}; // namespace ustl

#endif


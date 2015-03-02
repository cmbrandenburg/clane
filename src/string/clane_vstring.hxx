// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// vim: set noet:

/** @file */

#ifndef CLANE_VSTRING_HXX
#define CLANE_VSTRING_HXX

#include <cstring>
#include <string>

namespace clane {
	
	/** Variable-ownership, immutable string
	 *
	 * @remark The @ref vstring class encapsulates an immutable string that it may
	 * or may not own the memory for. In the case where a @ref vstring instance
	 * owns the memory, the class acts takes care of allocation and deallocation
	 * and behaves similarly to a `const std::string`. In the case where the @ref
	 * vstring instance doesn't own the memory, it's the responsibility of the
	 * programmer to ensure the memory remains valid for the lifetime of the @ref
	 * vstring instance.
	 *
	 * @remark The underlying string must be null-terminated and not contain any
	 * null characters. The @ref vstring class doesn't have a &ldquo;length&rdquo;
	 * member variable, so operations such as size() are of complexity O(n), where
	 * &lsquo;n&rsquo; is the length of the string. The intention is for @ref
	 * vstring to be used to hold short sequences of ASCII-encoded protocol data,
	 * such as URI components and HTTP headers, that typically are no more than a
	 * few hundred bytes. */
	struct vstring {
		friend void swap(vstring&, vstring&) noexcept;
		friend vstring vstring_as_ref(char const*) noexcept;

		typedef std::size_t size_type;
		typedef char const* const_iterator;
		typedef std::reverse_iterator<const_iterator> reverse_const_iterator;

	private:
		char const *m_str{""};
		bool        m_own{};

		static size_type constexpr npos = static_cast<size_type>(-1);

	public:

		~vstring() {
			if (m_own)
				delete[] m_str;
		}

		/** Constructs an empty string */
		vstring() = default;

		vstring(vstring const &) = default;
		vstring(vstring &&) = default;

		/** Constructs an owning string */
		vstring(char const *s) noexcept: m_str{s} {}

		/** Constructs an owning string */
		template<typename String> vstring(String const &s):
			m_own{true}
		{
			auto n = std::strlen(s.c_str())+1;
			m_str = new char[n];
			std::memcpy(m_str, s, n);
		}

		vstring &operator=(vstring const &) = default;
		vstring &operator=(vstring &&) = default;

		/** Assigns a copy of a string */
		vstring &operator=(char const *s) noexcept {
			if (m_own) {
				delete[] m_str;
				m_own = false;
			}
			m_str = s;
			return *this;
		}

		std::string string() const {
			return std::string{m_str};
		}

		size_type size() const noexcept {
			return std::strlen(m_str);
		}

		size_type length() const noexcept {
			return size();
		}

		bool empty() const noexcept {
			return !*m_str;
		}

		const_iterator begin() const noexcept {
			return cbegin();
		}

		const_iterator cbegin() const noexcept {
			return m_str;
		}

		const_iterator end() const noexcept {
			return cend();
		}

		const_iterator cend() const noexcept {
			return m_str + std::strlen(m_str);
		}

		reverse_const_iterator rbegin() const noexcept {
			return crbegin();
		}

		reverse_const_iterator crbegin() const noexcept {
			return std::reverse_iterator<const_iterator>{cend()};
		}

		reverse_const_iterator rend() const noexcept {
			return crend();
		}

		reverse_const_iterator crend() const noexcept {
			return std::reverse_iterator<const_iterator>{cbegin()};
		}

		char const &operator[](size_type pos) const noexcept {
			return m_str[pos];
		}

		char const *data() const noexcept {
			return m_str;
		}

		char const *c_str() const noexcept {
			return m_str;
		}

		void clear() noexcept {
			if (m_own) {
				delete[] m_str;
				m_own = false;
			}
			m_str = "";
		}

		size_type find(char c) const noexcept {
			auto p = std::strchr(m_str, c);
			return p ? p-m_str : npos;
		}

		size_type find(char const *s) const noexcept {
			auto p = std::strstr(m_str, s);
			return p ? p-m_str : npos;
		}

		template<typename String> size_type find(String const &s) const noexcept {
			auto p = std::strstr(m_str, s.c_str());
			return p ? p-m_str : npos;
		}

		size_type rfind(char c) const noexcept {
			auto p = std::strrchr(m_str, c);
			return p ? p-m_str : npos;
		}

		// FIXME: implement
		size_type rfind(char const *s) const noexcept;

		// FIXME: implement
		template<typename String> size_type rfind(String const &s) const noexcept;

		size_type find_first_of(char const *s) const noexcept {
			auto n = std::strcspn(m_str, s);
			return m_str[n] ? n : npos;
		}

		template<typename String> size_type find_first_of(String const &s) const noexcept {
			auto n = std::strcspn(m_str, s);
			return m_str[n] ? n : npos;
		}

		// FIXME: implement
		size_type find_last_of(char const *s) const noexcept;

		// FIXME: implement
		template<typename String> size_type find_last_of(String const &s) const noexcept;

		size_type find_first_not_of(char const *s) const noexcept {
			auto n = std::strspn(m_str, s);
			return m_str[n] ? n : npos;
		}

		template<typename String> size_type find_first_not_of(String const &s) const noexcept {
			auto n = std::strspn(m_str, s.c_str());
			return m_str[n] ? n : npos;
		}

		// FIXME: implement
		size_type find_last_not_of(char const *s) const noexcept;

		// FIXME: implement
		template<typename String> size_type find_last_not_of(String const &s) const noexcept;
	};

	void swap(vstring &a, vstring &b) noexcept {
		using std::swap;
		swap(a.m_own, b.m_own);
		swap(a.m_str, b.m_str);
	}

	/** Returns a non-owning string */
	vstring vstring_as_ref(char const *s) noexcept {
		vstring vs;
		vs.m_str = s;
		return vs;
	}

}

#endif // #ifndef CLANE_VSTRING_HXX

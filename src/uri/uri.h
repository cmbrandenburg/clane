// vim: set noet:

#ifndef CLANE_URI_H
#define CLANE_URI_H

/** @file
 *
 * @brief URI type */

#include <ostream>
#include <stdexcept>

namespace clane {

	/** @brief URI parsing and composition */
	namespace uri {

		/** @brief Exception type for reporting an attempt to operate on an invalid
		 * URI. */
		class invalid_uri: public std::invalid_argument {
		public:
			invalid_uri(char const *what_arg): std::invalid_argument(what_arg) {}
		};

		/** @brief Uniform Resource Identifier type
		 *
		 * The clane::uri::uri type encapsulates a URI as its distinct components.
		 * An instance may be constructed from individual components or by parsing a
		 * string via the parse_uri_reference() function. Once constructed, an
		 * instance may be cast to a string or inserted into an output stream. */
		class uri {
		public:

			/** @brief Scheme, e.g. "http" */
			std::string scheme;

			/** @brief User info, e.g., "johndoe" */
			std::string user_info;

			/** @brief Host, e.g., "example.com" */
			std::string host;

			/** @brief Port, e.g., "1234" */
			std::string port;

			/** @brief Path, e.g., "/my/file" */
			std::string path;

			/** @brief Query, e.g., "alpha=bravo&charlie=delta" */
			std::string query;

			/** @brief Fragment, e.g., "chapter_33" */
			std::string fragment;

		public:
			~uri() = default;
			uri() = default;
			uri(uri const &) = default;
			uri(uri &&) = default;
			uri &operator=(uri const &) = default;
			uri &operator=(uri &&) = default;
			operator std::string() const;
			bool operator==(uri const &that) const;
			bool operator!=(uri const &that) const { return !(*this == that); }
			void clear();
		};

		/** @brief Swaps the contents of two clane::uri::uri instances */
		void swap(uri &a, uri &b);

		/** @brief Parses a string as a URI reference
		 *
		 * @return If the string is a syntactically valid URI reference then
		 * parse_uri_reference() returns true and assigns to the `uri` argument the
		 * parsed URI. Else parse_uri_reference() returns false. */
		bool parse_uri_reference(uri &uri, std::string const &s);

		/** @brief Inserts a URI into an output stream */
		std::ostream &operator<<(std::ostream &ostrm, uri const &uri);
	}
}

#endif // #ifndef CLANE_URI_H

// vim: set noet:

#ifndef CLANE_HTTP_CONSUMER_H
#define CLANE_HTTP_CONSUMER_H

/** @file
 *
 * @brief HTTP parsing */

#include "http_common.h"
#include "http_header.h"
#include "http_message.h"
#include "http_status.h"
#include "../uri/uri.h"

namespace clane {
	namespace http {

		// Base class for all consumers.
		class consumer {
		protected:
			enum class status {
				ready, // ready to consume (more) data
				ok,    // parsing completed with success
				error  // parsing completed with error
			} stat;
		private:
			size_t len_limit; // length limit, if any
			size_t cur_len;
			status_code error_code_; // HTTP response status, in case an error occurred
			char const *what_;       // error description, in case an error occurred
		public:
			~consumer() = default;
			consumer();
			consumer(consumer const &) = delete;
			consumer(consumer &&) = default;
			consumer &operator=(consumer const &) = delete;
			consumer &operator=(consumer &&) = default;

			// Returns true if and only if the consumer is in a non-error state.
			operator bool() const { return stat != status::error; }

			// Returns the HTTP response status, if an error occurred.
			status_code error_code() const { return error_code_; }

			// Returns the error description, if an error occurred.
			char const *what() const { return what_; }

			// Resets the consumer state so that it may begin parsing anew.
			void reset();

			// Returns the number of bytes consumed from parsing.
			size_t length() const { return cur_len; }

			// Sets the consumer's length limit.
			void set_length_limit(size_t n) { len_limit = n; }

		protected:

			// Increases the current length and returns true if and only if the
			// current length limit is not set or not exceeded.
			bool increase_length(size_t n);

			// Sets the consumer into the error state, with a description of the error.
			void set_error(status_code error_stat, char const *what);
		};

		class headers_consumer: virtual public consumer {
			enum class phase {
				start_line,   // after consuming newline, expecting header name or linear whitespace
				end_newline,  // expecting newline character to end headers
				name,         // consuming a header name
				pre_value,    // whitespace before header value, or linear whitespace in line continuation
				value,        // consuming a header value
				value_newline // expecting newline character to end value line
			} cur_phase;
			header_map *hdrs;
			std::string hdr_name;
			std::string hdr_val;
		public:
			~headers_consumer() = default;
			headers_consumer(header_map &hdrs);
			headers_consumer(headers_consumer const &) = delete;
			headers_consumer(headers_consumer &&) = default;
			headers_consumer &operator=(headers_consumer const &) = delete;
			headers_consumer &operator=(headers_consumer &&) = default;
			bool consume(char const *buf, size_t size);
			void reset(header_map &hdrs);
		};

		class request_line_consumer: virtual public consumer {
			enum class phase {
				method,
				uri,
				version,
				newline
			} cur_phase;
			std::string *method;
			uri::uri *uri;
			int *major_ver;
			int *minor_ver;
			std::string uri_str;
			std::string version_str;
		public:
			~request_line_consumer() = default;
			request_line_consumer(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver);
			request_line_consumer(request_line_consumer const &) = delete;
			request_line_consumer(request_line_consumer &&) = default;
			request_line_consumer &operator=(request_line_consumer const &) = delete;
			request_line_consumer &operator=(request_line_consumer &&) = default;
			bool consume(char const *buf, size_t size);
			void reset(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver);
		private:
			bool parse_version();
		};

		class request_1x_consumer: virtual public consumer, private request_line_consumer, private headers_consumer {
			enum class phase {
				request_line,
				headers
			} cur_phase;
		public:
			~request_1x_consumer() = default;
			request_1x_consumer(request &req);
			request_1x_consumer(request_1x_consumer const &) = delete;
			request_1x_consumer(request_1x_consumer &&) = default;
			request_1x_consumer &operator=(request_1x_consumer const &) = delete;
			request_1x_consumer &operator=(request_1x_consumer &&) = default;
			bool consume(char const *buf, size_t size);
			void reset(request &req);
		};

		class chunk_line_consumer: virtual public consumer {
			static constexpr int max_nibs = 2 * sizeof(size_t);
			enum class phase {
				digit,
				newline
			} cur_phase;
			int nibs;
			size_t val;
		public:
			~chunk_line_consumer() = default;
			chunk_line_consumer();
			chunk_line_consumer(chunk_line_consumer const &) = delete;
			chunk_line_consumer(chunk_line_consumer &&) = default;
			chunk_line_consumer &operator=(chunk_line_consumer const &) = delete;
			chunk_line_consumer &operator=(chunk_line_consumer &&) = default;
			// Note: Ignores length limit, uses fixed length limit based of
			// sizeof(size_t).
			bool consume(char const *buf, size_t size);
			void reset();
			size_t chunk_size() const { return val; }
		};

		inline headers_consumer::headers_consumer(header_map &hdrs): cur_phase(phase::start_line), hdrs(&hdrs) {
		}

		inline void headers_consumer::reset(header_map &hdrs) {
			consumer::reset();
			cur_phase = phase::start_line;
			this->hdrs = &hdrs;
			hdr_name.clear();
			hdr_val.clear();
		}

		inline request_line_consumer::request_line_consumer(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver):
		  	 	cur_phase(phase::method), method(&method), uri(&uri), major_ver(&major_ver), minor_ver(&minor_ver) {
		}

		inline void request_line_consumer::reset(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver) {
			consumer::reset();
			cur_phase = phase::method;
			this->method = &method;
			this->uri = &uri;
			this->major_ver = &major_ver;
			this->minor_ver = &minor_ver;
			uri_str.clear();
			version_str.clear();
		}

		inline request_1x_consumer::request_1x_consumer(request &req):
		  	 	request_line_consumer(req.method, req.uri, req.major_version, req.minor_version),
				headers_consumer(req.headers), cur_phase(phase::request_line) {
		}

		inline void request_1x_consumer::reset(request &req) {
			consumer::reset();
			cur_phase = phase::request_line;
			request_line_consumer::reset(req.method, req.uri, req.major_version, req.minor_version);
			headers_consumer::reset(req.headers);
		}

		inline chunk_line_consumer::chunk_line_consumer(): cur_phase(phase::digit), nibs{}, val{} {}

		inline void chunk_line_consumer::reset() {
			consumer::reset();
			cur_phase = phase::digit;
			nibs = 0;
			val = 0;
		}
	}
}

#endif // #ifndef CLANE_HTTP_CONSUMER_H

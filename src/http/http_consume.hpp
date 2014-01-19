// vim: set noet:

#ifndef CLANE__HTTP_CONSUMER_H
#define CLANE__HTTP_CONSUMER_H

#include "http_common.hpp"
#include "http_header.hpp"
#include "http_request.hpp"
#include "http_status.hpp"
#include "../uri/uri.hpp"

namespace clane {
	namespace http {

		// Searches a given memory block for the first newline. If a carriage return
		// and newline pair is found ("\r\n") then this returns a pointer to the
		// carriage return character. Else, if a newline is found then this returns
		// a pointer to the newline character. Else, if a carriage return is found
		// at the last byte of the block then this returns a pointer to that
		// carriage return. Else, this returns a pointer to the first byte after the
		// memory block.
		//
		// In other words, the result of this function is to return a pointer to the
		// first "unreadable" byte in or out of the block, where readable characters
		// are characters in the current line, excluding "\r\n" and "\n". Note that
		// carriage returns followed by a character other than a newline are
		// considered readable.
		char const *find_newline(const char *p, size_t n);

		void rtrim(std::string &s);

		bool is_header_name_valid(std::string const &s);
		bool is_header_value_valid(std::string const &s);
		bool is_method_valid(std::string const &s);

		bool parse_version(int *major_ver, int *minor_ver, std::string &s);
		bool parse_status_code(status_code *stat, std::string &s);

		// Base class for all consumers. A consumer is a stream-oriented parser that
		// processes input one memory block at a time.
		//
		// Derived classes implement a 'consume' member function that accepts a
		// memory block as input and returns the number of bytes consumed, or the
		// special value 'error' if the input memory block is invalid. After
		// consumption, the consumer may be checked for whether consumption
		// completed. For example, an HTTP header consumer will be incomplete if the
		// input memory block contains only a partial header (e.g., "Content-Length:
		// 0\r\nContent-"), whereas the consumer will be complete if the input
		// memory block contains a full header (e.g., "Content-Length:
		// 0\r\nContent-Type: text/plain\r\n\r\n").
		//
		// Consumption may result in only some of the input memory block being
		// consumed. This will happen if the input memory block contains additional
		// data (e.g., an HTTP header consumer will ignore entity body data
		// following the end of the header) or if (by design) the consumer returns
		// control to the caller to allow the caller to process the consumed data
		// before resuming further consumption.
		//
		// A consumer instance may be reused after calling the 'reset' method.
		// Derived classes overload this method to do additional reset steps.
		//
		// The consumer class uses no virtual methods. The base class is provided as
		// a means of providing state common to all consumer classes; otherwise the
		// consumers are unrelated to each other except by idiom.
		//
		class consumer {
		public:
			static size_t constexpr error = static_cast<size_t>(-1);
		private:
			size_t len_limit;  // length limit, if any
			size_t total_len;  // total number of bytes consumed in all memory blocks
			char const *what_; // error description, in case an error occurred
			bool done_;
		public:
			~consumer() = default;
			consumer();
			consumer(consumer const &) = delete;
			consumer(consumer &&) = default;
			consumer &operator=(consumer const &) = delete;
			consumer &operator=(consumer &&) = default;

			// Returns true if and only if the consumer is done, either due to success
			// or error.
			bool done() const { return done_; }

			// Returns the error description, if an error occurred.
			char const *what() const { return what_; }

			// Resets the consumer state so that it may begin parsing anew.
			void reset();

			// Sets the consumer's length limit. Zero means no limit.
			void set_length_limit(size_t n);

		protected:

			// Increases the current length and returns true if and only if the
			// current length limit is not set or not exceeded.
			bool increase_length(size_t n);

			// Sets the consumer into the error state, with a description of the error.
			void set_error(char const *what);

			// Sets the consumer into the done state.
			void set_done();
		};

		inline consumer::consumer(): len_limit{}, total_len{}, done_{} {}

		inline bool consumer::increase_length(size_t n) {
			size_t new_len = total_len + n;
			if (new_len < total_len)
				return false; // overflow
			if (len_limit && new_len > len_limit)
				return false; // length limit set and exceeded
			total_len = new_len;
			return true;
		}

		inline void consumer::reset() {
			done_ = false;
			total_len = 0;
			// The length limit is preserved.
		}

		inline void consumer::set_error(char const *what) {
			done_ = true;
			what_ = what;
		}

		inline void consumer::set_done() {
			done_ = true;
		}

		inline void consumer::set_length_limit(size_t n) {
		 	len_limit = n;
	 	}

		// Base class for server-side consumers. This provides an additional HTTP
		// status code error code that may be sent in an HTTP response.
		class server_consumer: virtual public consumer {
			status_code error_code_; // HTTP response status, in case an error occurred, if server-side
		public:
			~server_consumer() = default;
			server_consumer() = default;
			server_consumer(server_consumer const &) = delete;
			server_consumer(server_consumer &&) = default;
			server_consumer &operator=(server_consumer const &) = delete;
			server_consumer &operator=(server_consumer &&) = default;

			// Returns the HTTP response status, if an error occurred, if server-side.
			status_code error_code() const { return error_code_; }

		protected:

			// Sets the consumer into the error state, with a description of the error.
			void set_error(status_code error_stat, char const *what);
		};

		inline void server_consumer::set_error(status_code n, char const *what) {
			consumer::set_error(what);
			error_code_ = n;
		}

		class v1x_request_line_consumer: virtual public server_consumer {
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
			~v1x_request_line_consumer() = default;
			v1x_request_line_consumer(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver);
			v1x_request_line_consumer(v1x_request_line_consumer const &) = delete;
			v1x_request_line_consumer(v1x_request_line_consumer &&) = default;
			v1x_request_line_consumer &operator=(v1x_request_line_consumer const &) = delete;
			v1x_request_line_consumer &operator=(v1x_request_line_consumer &&) = default;
			size_t consume(char const *buf, size_t size);
			void reset(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver);
		};

		inline v1x_request_line_consumer::v1x_request_line_consumer(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver):
		  	 	cur_phase(phase::method), method(&method), uri(&uri), major_ver(&major_ver), minor_ver(&minor_ver) {
		}

		inline void v1x_request_line_consumer::reset(std::string &method, uri::uri &uri, int &major_ver, int &minor_ver) {
			consumer::reset();
			cur_phase = phase::method;
			this->method = &method;
			this->uri = &uri;
			this->major_ver = &major_ver;
			this->minor_ver = &minor_ver;
			uri_str.clear();
			version_str.clear();
		}

		class v1x_status_line_consumer: virtual public consumer {
			enum class phase {
				version,
				status,
				reason,
				newline
			} cur_phase;
			int *major_ver;
			int *minor_ver;
			status_code *stat;
			std::string *reason;
			std::string version_str;
			std::string status_str;
		public:
			~v1x_status_line_consumer() = default;
			v1x_status_line_consumer(int &major_ver, int &minor_ver, status_code &stat, std::string &reason);
			v1x_status_line_consumer(v1x_status_line_consumer const &) = delete;
			v1x_status_line_consumer(v1x_status_line_consumer &&) = default;
			v1x_status_line_consumer &operator=(v1x_status_line_consumer const &) = delete;
			v1x_status_line_consumer &operator=(v1x_status_line_consumer &&) = default;
			size_t consume(char const *buf, size_t size);
			void reset(int &major_ver, int &minor_ver, status_code &stat, std::string &reason);
		};

		inline v1x_status_line_consumer::v1x_status_line_consumer(int &major_ver, int &minor_ver, status_code &stat, std::string &reason):
			cur_phase(phase::version), major_ver(&major_ver), minor_ver(&minor_ver), stat(&stat), reason(&reason) {}

		inline void v1x_status_line_consumer::reset(int &major_ver, int &minor_ver, status_code &stat, std::string &reason) {
			consumer::reset();
			cur_phase = phase::version;
			this->major_ver = &major_ver;
			this->minor_ver = &minor_ver;
			this->stat = &stat;
			this->reason = &reason;
			version_str.clear();
			status_str.clear();
		}

		class v1x_headers_consumer: virtual public server_consumer {
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
			~v1x_headers_consumer() = default;
			v1x_headers_consumer(header_map &hdrs);
			v1x_headers_consumer(v1x_headers_consumer const &) = delete;
			v1x_headers_consumer(v1x_headers_consumer &&) = default;
			v1x_headers_consumer &operator=(v1x_headers_consumer const &) = delete;
			v1x_headers_consumer &operator=(v1x_headers_consumer &&) = default;
			size_t consume(char const *buf, size_t size);
			void reset(header_map &hdrs);
		};

		inline v1x_headers_consumer::v1x_headers_consumer(header_map &hdrs): cur_phase(phase::start_line), hdrs(&hdrs) {}

		inline void v1x_headers_consumer::reset(header_map &hdrs) {
			consumer::reset();
			cur_phase = phase::start_line;
			this->hdrs = &hdrs;
			hdr_name.clear();
			hdr_val.clear();
		}

#if 0
		class request_1x_consumer: virtual public consumer, private v1x_request_line_consumer, private v1x_headers_consumer {
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

		inline request_1x_consumer::request_1x_consumer(request &req):
		  	 	v1x_request_line_consumer(req.method, req.uri, req.major_version, req.minor_version),
				v1x_headers_consumer(req.headers), cur_phase(phase::request_line) {
		}

		inline void request_1x_consumer::reset(request &req) {
			consumer::reset();
			cur_phase = phase::request_line;
			v1x_request_line_consumer::reset(req.method, req.uri, req.major_version, req.minor_version);
			v1x_headers_consumer::reset(req.headers);
		}

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
			// Note: Ignores length limit, uses fixed length limit based on
			// sizeof(size_t).
			bool consume(char const *buf, size_t size);
			void reset();
			size_t chunk_size() const { return val; }
		};

		inline chunk_line_consumer::chunk_line_consumer(): cur_phase(phase::digit), nibs{}, val{} {}

		inline void chunk_line_consumer::reset() {
			consumer::reset();
			cur_phase = phase::digit;
			nibs = 0;
			val = 0;
		}
#endif
	}
}

#endif // #ifndef CLANE__HTTP_CONSUMER_H

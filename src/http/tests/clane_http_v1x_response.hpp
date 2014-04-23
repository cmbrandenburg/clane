// vim: set noet:

#ifndef CLANE_HTTP_V1X_RESPONSE_HPP
#define CLANE_HTTP_V1X_RESPONSE_HPP

#include "../clane_http_parse.hpp"
#include "../clane_http_message.hpp"
#include "../../clane_check.hpp"
#include <string>

inline void check_v1x_response(std::string const &s, int exp_major, int exp_minor, clane::http::status_code exp_stat,
	 	std::string const &exp_reason, clane::http::header_map const &exp_hdrs, std::string const &exp_body,
	 	clane::http::header_map const &exp_trlrs) {

	using namespace clane;

	// parse:
	http::v1x_response_incparser pars;
	pars.reset();
	std::string body;
	size_t i = 0;
	while (i < s.size()) {
		size_t stat = pars.parse_some(s.data()+i, s.data()+s.size());
		check(pars.error != stat); // invalid response?
		body.append(s.data()+i+pars.offset(), pars.size());
		i += stat;
	}
	check(pars.got_headers()); // incomplete response?

	// check fields:
	check(pars.major_version() == exp_major);
	check(pars.minor_version() == exp_minor);
	check(pars.status() == exp_stat);
	check(pars.reason() == exp_reason);
	check(pars.headers() == exp_hdrs);
	check(body == exp_body);
	check(pars.trailers() == exp_trlrs);
}

#endif // #ifndef CLANE_HTTP_V1X_RESPONSE_HPP

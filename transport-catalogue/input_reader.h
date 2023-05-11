#pragma once
#include "transport_catalogue.h"

namespace input_reader {
	namespace add_query {
		void Add(std::vector<std::string>& query, transport_catalogue::TransportCatalogue& catalogue);
	}//close add_query
	
	namespace detail {
		transport_catalogue::detail::Query ParseQueryStopFirstIteration(std::string_view str);

		transport_catalogue::detail::Query ParseQueryStopSecondIteration(std::string_view str);

		transport_catalogue::detail::Query ParseQueryBus(
			std::string_view str, transport_catalogue::TransportCatalogue& catalogue);
	}//close detail
	
}//close input_reader

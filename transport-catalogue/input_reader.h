#pragma once
#include "transport_catalogue.h"

namespace input_reader {
	void ÑatalogFilling(std::istream& in, transport_catalogue::TransportCatalogue& catalogue);

	namespace add_query {
		void Add(std::vector<std::string>& query, transport_catalogue::TransportCatalogue& catalogue);
	}//close add_query
	
	namespace detail {
		transport_catalogue::detail::Stop ParseQueryStopFirstIteration(std::string_view str);

		transport_catalogue::detail::Distance ParseQueryStopSecondIteration(std::string_view str);

		transport_catalogue::detail::Bus ParseQueryBus(
			std::string_view str, transport_catalogue::TransportCatalogue& catalogue);
	}//close detail
	
}//close input_reader

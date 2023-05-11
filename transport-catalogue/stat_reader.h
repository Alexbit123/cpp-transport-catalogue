#pragma once
#include "transport_catalogue.h"

namespace stat_reader {
	namespace statistic {
		namespace detail {
			void GetBus(std::string_view str, transport_catalogue::TransportCatalogue& catalogue);

			void GetStop(std::string_view str, transport_catalogue::TransportCatalogue& catalogue);
		}//close detail

		void GetResult(std::vector<std::string>& query, transport_catalogue::TransportCatalogue& catalogue);
	}//close get_statistic
}//close stat_reader

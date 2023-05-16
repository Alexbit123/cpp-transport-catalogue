#pragma once
#include "transport_catalogue.h"

namespace stat_reader {
	void RequestProcessing(std::istream& in, std::ostream& out, transport_catalogue::TransportCatalogue& catalogue);

	namespace statistic {
		namespace detail {
			void PrintInfoBus(std::string_view str, transport_catalogue::TransportCatalogue& catalogue, std::ostream& out);

			void PrintInfoStop(std::string_view str, transport_catalogue::TransportCatalogue& catalogue, std::ostream& out);
		}//close detail

		void PrintResult(std::vector<std::string>& query, transport_catalogue::TransportCatalogue& catalogue, std::ostream& out);
	}//close get_statistic
}//close stat_reader

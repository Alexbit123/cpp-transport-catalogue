#include "stat_reader.h"

namespace stat_reader {
	void RequestProcessing(std::istream& in, std::ostream& out, transport_catalogue::TransportCatalogue& catalogue) {
		int query_count_out;
		std::vector<std::string> query;
		std::string str;

		in >> query_count_out;

		for (int i = 0; i < query_count_out; ++i) {
			std::getline(in >> std::ws, str);
			query.push_back(str);
		}

		stat_reader::statistic::PrintResult(query, catalogue, out);
	}

	namespace statistic {
		namespace detail {
			void PrintInfoBus(std::string_view str, transport_catalogue::TransportCatalogue& catalogue, std::ostream& out) {
				transport_catalogue::detail::QueryResultBus result;
				auto space = str.find(' ');
				std::string_view query_name = str.substr(space + 1, str.npos);
				result = catalogue.GetInfoBus(query_name);
				if (result.count_stops != 0) {
					out << "Bus " << result.query_name << ": " << result.count_stops << " stops on route, "
						<< result.unique_count_stops << " unique stops, " << std::setprecision(6)
						<< result.route_length << " route length, " << result.curvature << " curvature" << std::endl;
				}
				else {
					out << "Bus " << result.query_name << ": not found" << std::endl;
				}
				
			}

			void PrintInfoStop(std::string_view str, transport_catalogue::TransportCatalogue& catalogue, std::ostream& out) {
				transport_catalogue::detail::QueryResultStop result;
				auto space = str.find(' ');
				std::string_view query_name = str.substr(space + 1, str.npos);
				result = catalogue.GetInfoStop(query_name);
				if (!result.buses.empty()) {
					out << "Stop " << result.query_name << ": buses ";
					for (std::string_view stop : result.buses) {
						out << stop << " ";
					}
					out << std::endl;
				}
				else {
					if (catalogue.FindStop(result.query_name) == nullptr) {
						out << "Stop " << result.query_name << ": not found" << std::endl;
					}
					else {
						out << "Stop " << result.query_name << ": no buses" << std::endl;
					}
				}
			}
		}//close detail

		void PrintResult(std::vector<std::string>& query, transport_catalogue::TransportCatalogue& catalogue, std::ostream& out) {
			std::string_view name;
			for (std::string_view str : query) {
				auto space = str.find(' ');
				name = str.substr(0, space);
				if (name == "Bus") {
					detail::PrintInfoBus(str, catalogue, out);
				}
				if (name == "Stop") {
					detail::PrintInfoStop(str, catalogue, out);
				}
			}
		}
	}//close statistic
}//close stat_reader

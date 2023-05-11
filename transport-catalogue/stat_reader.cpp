#include "stat_reader.h"

namespace stat_reader {
	namespace statistic {
		namespace detail {
			void GetBus(std::string_view str, transport_catalogue::TransportCatalogue& catalogue) {
				transport_catalogue::detail::QueryResultBus result;
				std::unordered_set<transport_catalogue::detail::Stop*> count;
				double route_length_geo = 0;
				auto space = str.find(' ');
				result.query_name = str.substr(space + 1, str.npos);
				if (catalogue.FindBus(result.query_name) != nullptr) {
					auto bus = catalogue.FindBus(result.query_name);
					result.count_stops = static_cast<int>(bus->route.size());

					count.insert(bus->route.begin(), bus->route.end());
					result.unique_count_stops = static_cast<int>(count.size());

					for (size_t i = 0; i < bus->route.size() - 1; ++i) {
						result.route_length += catalogue.GetDistance(bus->route[i]->stop_name, bus->route[i + 1]->stop_name);
						route_length_geo += geo_coordinates::distance::ComputeDistance(bus->route[i]->coordinates_, 
							bus->route[i + 1]->coordinates_);
					}
					result.curvature = result.route_length / route_length_geo;
					catalogue.GetInfoBus(result);
				}
				else {
					std::cout << "Bus " << result.query_name << ": not found" << std::endl;
				}
			}

			void GetStop(std::string_view str, transport_catalogue::TransportCatalogue& catalogue) {
				transport_catalogue::detail::QueryResultStop result;
				auto space = str.find(' ');
				result.query_name = str.substr(space + 1, str.npos);
				if (!catalogue.GetBuses(result.query_name).empty()) {
					result.buses.insert(catalogue.GetBuses(result.query_name).begin(), catalogue.GetBuses(result.query_name).end());
					catalogue.GetInfoStop(result);
				}
				else {
					if (catalogue.FindStop(result.query_name) == nullptr) {
						std::cout << "Stop " << result.query_name << ": not found" << std::endl;
					}
					else {
						std::cout << "Stop " << result.query_name << ": no buses" << std::endl;
					}
				}
			}
		}//close detail

		void GetResult(std::vector<std::string>& query, transport_catalogue::TransportCatalogue& catalogue) {
			std::string_view name;
			for (std::string_view str : query) {
				auto space = str.find(' ');
				name = str.substr(0, space);
				if (name == "Bus") {
					detail::GetBus(str, catalogue);
				}
				if (name == "Stop") {
					detail::GetStop(str, catalogue);
				}
			}
		}
	}//close statistic
}//close stat_reader

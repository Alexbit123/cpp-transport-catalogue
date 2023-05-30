#pragma once
#include "geo.h"
#include <string>
#include <vector>
#include <set>

namespace domain {
		struct Stop {
			std::string stop_name = "";
			geo::Coordinates coordinates_;
		};

		struct Bus {
			bool is_roundtrip;
			std::string bus_name = "";
			std::vector<Stop*> route;
		};

		struct Query {
			std::string_view query_name;
			Stop stop_struct;
			Bus bus_struct;
			std::vector<std::tuple<uint64_t, std::string, std::string>> stop_to_stop_distance;
		};

		struct Distance {
			std::set<std::tuple<std::string, std::string, uint64_t>> stop_to_stop_distance;
		};
		struct QueryResultBus {
			std::string_view query_name;
			int count_stops = 0;
			int unique_count_stops = 0;
			double route_length = 0;
			double curvature = 0;
		};

		struct QueryResultStop {
			std::string_view query_name;
			std::set<std::string_view> buses;
		};

		struct StopHasher {
			size_t operator() (const std::pair<Stop*, Stop*>& stops) const {
				return hasher_(stops.first) + hasher_(stops.second) * 37;
			}
		private:
			std::hash<const Stop*> hasher_;
		};
}//close domain

#pragma once

#include "geo.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>

namespace transport_catalogue {
	using namespace std::literals;

	namespace detail {
		struct Stop {
			std::string stop_name = "";
			geo_coordinates::Coordinates coordinates_;
		};

		struct Bus {
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
	}//close detail

	class TransportCatalogue {
	public:
		void AddBus(const detail::Bus& query);

		void AddStop(const detail::Stop& query);

		void AddDistance(const std::string_view name_stop_one, 
			const std::string_view name_stop_two, const uint64_t distance);

		detail::Bus* FindBus(std::string_view name_bus);

		detail::Stop* FindStop(std::string_view name_stop);

		std::vector<std::string_view>& GetBuses(std::string_view name_stop);

		uint64_t GetDistance(std::string_view stop_one, std::string_view stop_two);

		detail::QueryResultBus GetInfoBus(std::string_view name_bus);

		detail::QueryResultStop GetInfoStop(std::string_view name_stop);

	private:
		std::unordered_map<std::string_view, detail::Bus*> busname_to_bus;
		std::unordered_map<std::string_view, detail::Stop*> stopname_to_stop;
		std::unordered_map<std::string_view, std::vector<std::string_view>> stopname_to_bus;
		std::unordered_map<std::pair<detail::Stop*, detail::Stop*>, uint64_t, detail::StopHasher> stops_distance;
		std::deque<detail::Bus> buses;
		std::deque<detail::Stop> stops;
	};
}//close transport_catalogue

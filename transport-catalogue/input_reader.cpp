#include "input_reader.h"

namespace input_reader {
	using namespace std::literals;

	namespace detail {
		constexpr int COMMA = 2;
		transport_catalogue::detail::Query ParseQueryBus(
			std::string_view str, transport_catalogue::TransportCatalogue& catalogue) {
			transport_catalogue::detail::Query result;
			bool circular_route = false, query_name = true;

			while (true) {
				auto space = str.find(' ');
				if (str[0] == ' ') {
					str.remove_prefix(space + 1);
					continue;
				}
				if (str.substr(0, space) == "Bus"sv && query_name) {
					query_name = false;
					result.query_name = str.substr(0, space);
				}
				else {
					if (result.bus_struct.bus_name == ""sv) {
						const auto colon = str.find(':');
						std::string_view buf = str.substr(0, colon);
						result.bus_struct.bus_name = str.substr(0, buf.find_last_not_of(" ", buf.npos) + 1);
						str.remove_prefix(colon + 1);
						continue;
					}
					if (str.find('>') != str.npos) {
						circular_route = true;
						const auto greater_sign = str.find('>');
						std::string_view buf = str.substr(0, greater_sign);
						result.bus_struct.route.push_back(catalogue.FindStop(
							str.substr(0, buf.find_last_not_of(" ", buf.npos) + 1)));
						str.remove_prefix(greater_sign + 1);
						continue;
					}
					else if (circular_route) {
						std::string_view buf = str.substr(0, str.npos);
						result.bus_struct.route.push_back(catalogue.FindStop(
							str.substr(0, buf.find_last_not_of(" ", buf.npos) + 1)));
						space = str.npos;
					}
					else if (str.find('-') != str.npos) {
						const auto dash = str.find('-');
						std::string_view buf = str.substr(0, dash);
						result.bus_struct.route.push_back(catalogue.FindStop(
							str.substr(0, buf.find_last_not_of(" ", buf.npos) + 1)));
						str.remove_prefix(dash + 1);
						continue;
					}
					else {
						std::string_view buf = str.substr(0, str.npos);
						result.bus_struct.route.push_back(catalogue.FindStop(
							str.substr(0, buf.find_last_not_of(" ", buf.npos) + 1)));

						for (int i = static_cast<int>(result.bus_struct.route.size() - 2); i >= 0; --i) {
							result.bus_struct.route.push_back(result.bus_struct.route[i]);
						}
						space = str.npos;
					}
				}

				if (space == str.npos) {
					break;
				}
				else {
					str.remove_prefix(space + 1);
				}
			}

			return result;
		}

		transport_catalogue::detail::Query ParseQueryStopSecondIteration(std::string_view str) {
			transport_catalogue::detail::Query result;
			std::string stop_name_one = "";
			bool query_name = true;
			int count_comma = 0;

			while (true) {

				auto space = str.find(' ');

				if (str[0] == ' ') {
					str.remove_prefix(space + 1);
					continue;
				}
				if (str.substr(0, space) == "Stop"sv && query_name) {
					query_name = false;
				}
				else {
					if (stop_name_one == ""sv) {
						const auto colon = str.find(':');
						std::string_view buf = str.substr(0, colon);
						stop_name_one = str.substr(0, buf.find_last_not_of(" ", buf.npos) + 1);
						str.remove_prefix(colon + 1);
						continue;
					}
					if (count_comma >= COMMA) {
						uint64_t distance = std::stoi(static_cast<std::string>(str.substr(0, space - 1)));
						str.remove_prefix(space + 1);
						space = str.find(' ');
						if (str.substr(0, space) == "to") {
							str.remove_prefix(space + 1);
							space = str.find(' ');
						}
						if (str.find(',') != str.npos) {
							result.stop_to_stop_distance.push_back({distance, stop_name_one,
								static_cast<std::string>(str.substr(0, str.find(','))) });
							space = str.find(',');
						}
						else {
							result.stop_to_stop_distance.push_back({ distance, stop_name_one,
								static_cast<std::string>(str.substr(0, str.npos)) });
							space = str.npos;
						}
					}
				}

				if (count_comma < COMMA && str[space - 1] == ',') {
					++count_comma;
				}

				if (space == str.npos) {
					break;
				}
				else {
					str.remove_prefix(space + 1);
				}
			}

			return result;
		}

		transport_catalogue::detail::Query ParseQueryStopFirstIteration(std::string_view str) {
			transport_catalogue::detail::Query result;
			bool query_name = true;

			while (true) {
				auto space = str.find(' ');
				if (str[0] == ' ') {
					str.remove_prefix(space + 1);
					continue;
				}
				if (str.substr(0, space) == "Stop"sv && query_name) {
					query_name = false;
					result.query_name = str.substr(0, space);
				}
				else {
					if (result.stop_struct.stop_name == ""sv) {
						const auto colon = str.find(':');
						std::string_view word_to_colon = str.substr(0, colon);
						result.stop_struct.stop_name = str.substr(
							0, word_to_colon.find_last_not_of(" ", word_to_colon.npos) + 1);
						str.remove_prefix(colon + 1);
						continue;
					}
					result.stop_struct.coordinates_.lat = std::stod(static_cast<std::string>(str.substr(0, space - 1)));
					str.remove_prefix(space + 1);
					space = str.find(' ');
					if (space == str.npos) {
						result.stop_struct.coordinates_.lng = std::stod(static_cast<std::string>(str.substr(0, space)));
					}
					else {
						result.stop_struct.coordinates_.lng = std::stod(static_cast<std::string>(str.substr(0, space - 1)));
					}
					break;
				}

				if (space == str.npos) {
					break;
				}
				else {
					str.remove_prefix(space + 1);
				}
			}

			return result;
		}
	}//close detail

	namespace add_query {
		void Add(std::vector<std::string>& query, transport_catalogue::TransportCatalogue& catalogue) {
			std::string str, str_buf;
			for (std::string_view str : query) {
				const auto space = str.find(' ');
				if (str.substr(0, space) == "Stop") {
					transport_catalogue::detail::Query result = detail::ParseQueryStopFirstIteration(str);
					catalogue.AddStop(result);
				}
			}

			for (std::string_view str : query) {
				const auto space = str.find(' ');
				if (str.substr(0, space) == "Stop") {
					if (str.find(" to ", 0) != str.npos) {
						transport_catalogue::detail::Query result = detail::ParseQueryStopSecondIteration(str);
						catalogue.AddDistance(result);
					}
				}
			}

			for (std::string_view str : query) {
				const auto space = str.find(' ');
				if (str.substr(0, space) == "Bus") {
					transport_catalogue::detail::Query result = detail::ParseQueryBus(str, catalogue);
					catalogue.AddBus(result);
				}
			}
		}
	}//close add_query

}//close input_reader


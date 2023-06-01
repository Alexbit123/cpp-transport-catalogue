#include "json_reader.h"

namespace json_reader {
	void Read(transport_catalogue::TransportCatalogue& db, 
		renderer::MapRenderer& renderer, std::istream& in, std::ostream& out, bool need_out_stat_request) {
		std::string str, query = "";
		int count_bracket = 0;
		while (true) {
			std::getline(in >> std::ws, str);
			query += str;
			if (str.find("{") != str.npos) {
				++count_bracket;
			}
			if (str.find("}") != str.npos) {
				--count_bracket;
			}
			if (count_bracket == 0) {
				break;
			}
		}
		add_json_query::AddBaseRequests(db, renderer, query);
		if (need_out_stat_request) {
			out_json_query::Output(db, query, out, renderer);
		}
	}
	namespace out_json_query {
		void Output(transport_catalogue::TransportCatalogue& db, 
			std::string& query, std::ostream& out, renderer::MapRenderer& renderer) {
			std::istringstream strm(query);
			json::Document doc = json::Load(strm);

			detail::PrintResult(db, doc.GetRoot(), out, renderer);
		}
	}
	namespace add_json_query {
		void AddBaseRequests(transport_catalogue::TransportCatalogue& db,
			renderer::MapRenderer& renderer, std::string& query) {
			std::istringstream strm(query);
			json::Document doc = json::Load(strm);

			std::vector<domain::Stop> result_stops = detail::ParseQueryStop(doc.GetRoot());
			for (auto& stop : result_stops) {
				db.AddStop(stop);
			}

			std::vector<domain::Distance> result_distance = detail::ParseQueryDistance(doc.GetRoot());
			for (auto& dist : result_distance) {
				for (auto& [name_stop_one, name_stop_two, distance] : dist.stop_to_stop_distance) {
					db.AddDistance(name_stop_one, name_stop_two, distance);
				}
			}

			std::vector<domain::Bus> result_bus = detail::ParseQueryBus(doc.GetRoot(), db);
			for (auto& bus : result_bus) {
				db.AddBus(bus);
			}

			renderer = detail::ParseQueryMap(doc.GetRoot());
		}
	}

	namespace detail {
		json::Dict GetInfoBus(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem) {
			domain::QueryResultBus result;
			json::Dict json_dict;
			std::string str = "not found";
			result = db.GetInfoBus(elem.at("name").AsString());
			if (result.count_stops != 0) {
				json_dict["curvature"] = result.curvature;
				json_dict["request_id"] = elem.at("id").AsInt();
				json_dict["route_length"] = result.route_length;
				json_dict["stop_count"] = result.count_stops;
				json_dict["unique_stop_count"] = result.unique_count_stops;
			}
			else {
				json_dict["request_id"] = elem.at("id").AsInt();
				json_dict["error_message"] = str;
			}
			return json_dict;
		}

		json::Dict GetInfoStop(transport_catalogue::TransportCatalogue& db,  
			const json::Dict& elem) {
			domain::QueryResultStop result;
			result = db.GetInfoStop(elem.at("name").AsString());
			json::Array json_array;
			json::Dict json_dict;
			std::string str = "not found";
			if (!result.buses.empty()) {
				for (std::string_view bus : result.buses) {
					json_array.push_back(json::Node(static_cast<std::string>(bus)));
				}
				json_dict["buses"] = json_array;
				json_dict["request_id"] = elem.at("id").AsInt();
			}
			else {
				if (db.FindStop(result.query_name) != nullptr) {
					json_dict["buses"] = json_array;
					json_dict["request_id"] = elem.at("id").AsInt();
				}
				else {
					json_dict["request_id"] = elem.at("id").AsInt();
					json_dict["error_message"] = str;
				}
			}
			return json_dict;
		}

		json::Dict GetInfoMap(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem, renderer::MapRenderer& renderer) {
			std::stringstream out;
			json::Dict json_dict;

			RequestHandler result(db, renderer);
			svg::Document svg_doc = result.RenderMap();
			svg_doc.Render(out);
			std::string str = out.str();

			json::Node json_node(str);
			json_dict["map"] = json_node;
			json_dict["request_id"] = elem.at("id").AsInt();
			
			return json_dict;
		}

		void PrintResult(transport_catalogue::TransportCatalogue& db,
			const json::Node& elem, std::ostream& out, renderer::MapRenderer& renderer) {
			json::Array json_result;
			auto map_base_requests = elem.AsMap().find("stat_requests");
			auto& it_node = map_base_requests->second.AsArray();
			for (auto& elem_node : it_node) {
				auto& map_result = elem_node.AsMap();
				if (map_result.count("type")) {
					if (map_result.at("type").AsString() == "Stop") {
						json_result.push_back(GetInfoStop(db, map_result));
					}
					else if (map_result.at("type").AsString() == "Bus") {
						json_result.push_back(GetInfoBus(db, map_result));
					}
					else {
						json_result.push_back(GetInfoMap(db, map_result, renderer));
					}
				}
			}
			json::Node node_result(json_result);
			json::PrintNode(node_result, out);
		}

		std::vector<domain::Stop> ParseQueryStop(const json::Node& elem) {
			std::vector<domain::Stop> vec_stops;
			auto map_base_requests = elem.AsMap().find("base_requests");
			auto& it_node = map_base_requests->second.AsArray();
			for (auto& elem_node : it_node) {
				domain::Stop stop;
				auto& map_result = elem_node.AsMap();
				if (map_result.count("type")) {
					if (map_result.at("type").AsString() == "Stop") {
						stop.stop_name = map_result.at("name").AsString();
						stop.coordinates_.lat = map_result.at("latitude").AsDouble();
						stop.coordinates_.lng = map_result.at("longitude").AsDouble();
						vec_stops.push_back(stop);
					}
				}
			}
			return vec_stops;
		}

		std::vector<domain::Distance> ParseQueryDistance(const json::Node& elem) {
			std::vector<domain::Distance> vec_distance;
			auto map_base_requests = elem.AsMap().find("base_requests");
			auto& it_node = map_base_requests->second.AsArray();
			for (auto& elem_node : it_node) {
				domain::Distance distance;
				std::string name_one, name_two;
				int dist;
				auto& map_result = elem_node.AsMap();
				if (map_result.count("type")) {
					if (map_result.at("type").AsString() == "Stop") {
						name_one = map_result.at("name").AsString();
						auto& map_dist = map_result.at("road_distances").AsMap();
						for (auto& [name_stop, node] : map_dist) {
							name_two = name_stop;
							dist = node.AsInt();
							distance.stop_to_stop_distance.insert({ name_one, name_two, dist });
							vec_distance.push_back(distance);
						}
					}
				}
			}
			return vec_distance;
		}

		std::vector<domain::Bus> ParseQueryBus(const json::Node& elem,
			transport_catalogue::TransportCatalogue& db) {
			std::vector<domain::Bus> vec_buses;
			auto map_base_requests = elem.AsMap().find("base_requests");
			auto& it_node = map_base_requests->second.AsArray();
			for (auto& elem_node : it_node) {
				domain::Bus bus;
				auto& map_result = elem_node.AsMap();
				if (map_result.count("type")) {
					if (map_result.at("type").AsString() == "Bus") {
						bus.bus_name = map_result.at("name").AsString();
						if (map_result.at("is_roundtrip").AsBool()) {
							bus.is_roundtrip = true;
							auto& node_stops = map_result.at("stops");
							for (auto& node_stop : node_stops.AsArray()) {
								bus.route.push_back(db.FindStop(node_stop.AsString()));
							}
						}
						else {
							bus.is_roundtrip = false;
							auto& node_stops = map_result.at("stops");
							for (auto& node_stop : node_stops.AsArray()) {
								bus.route.push_back(db.FindStop(node_stop.AsString()));
							}
							for (int i = static_cast<int>(bus.route.size() - 2); i >= 0; --i) {
								bus.route.push_back(bus.route[i]);
							}
						}
						vec_buses.push_back(bus);
					}
				}
			}
			return vec_buses;
		}

		renderer::MapRenderer ParseQueryMap(const json::Node& elem) {
			renderer::MapRenderer result_map_renderer;
			auto map_base_requests = elem.AsMap().find("render_settings");
			auto& map_result = map_base_requests->second.AsMap();
			if (map_result.count("width")) {
				result_map_renderer.width_ = map_result.at("width").AsDouble();
				result_map_renderer.height_ = map_result.at("height").AsDouble();
				result_map_renderer.padding_ = map_result.at("padding").AsDouble();
				result_map_renderer.stop_radius_ = map_result.at("stop_radius").AsDouble();
				result_map_renderer.line_width_ = map_result.at("line_width").AsDouble();
				result_map_renderer.bus_label_font_size_ = map_result.at("bus_label_font_size").AsInt();
				for (auto& elem_array : map_result.at("bus_label_offset").AsArray()) {
					result_map_renderer.bus_label_offset.push_back(elem_array.AsDouble());
				}
				result_map_renderer.stop_label_font_size_ = map_result.at("stop_label_font_size").AsInt();
				for (auto& elem_array : map_result.at("stop_label_offset").AsArray()) {
					result_map_renderer.stop_label_offset.push_back(elem_array.AsDouble());
				}
				result_map_renderer.underlayer_color_ = ParseColor(map_result.at("underlayer_color"));
				result_map_renderer.underlayer_width_ = map_result.at("underlayer_width").AsDouble();
				for (auto& elem_array : map_result.at("color_palette").AsArray()) {
					result_map_renderer.color_palette_.push_back(ParseColor(elem_array));
				}
			}
			return result_map_renderer;
		}

		svg::Color ParseColor(const json::Node& node_result) {
			svg::Color color_;
			if (node_result.IsString()) {
				color_ = node_result.AsString();
			}
			else {
				if (node_result.AsArray().size() == 3) {
					color_ = svg::Rgb(node_result.AsArray()[0].AsInt(),
						node_result.AsArray()[1].AsInt(),
						node_result.AsArray()[2].AsInt());
				}
				else {
					color_ = svg::Rgba(node_result.AsArray()[0].AsInt(),
						node_result.AsArray()[1].AsInt(),
						node_result.AsArray()[2].AsInt(),
						node_result.AsArray()[3].AsDouble());
				}
			}
			return color_;
		}

	}//close detail

}//close json_reader


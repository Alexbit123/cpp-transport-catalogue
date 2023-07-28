#include "json_reader.h"

namespace json_reader {
	using namespace std::string_literals;

	void Read(transport_catalogue::TransportCatalogue& db,
		renderer::MapRenderer& renderer, router::TransportRouter& route, std::istream& in, std::string& query) {
		std::string str = "";
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
		add_json_query::AddBaseRequests(db, renderer, route, query);
	}

	void Out(transport_catalogue::TransportCatalogue& db,
		renderer::MapRenderer& renderer, router::TransportRouter& route, 
		std::istream& in, std::ostream& out, std::string& query) {
		out_json_query::Output(db, query, in, out, renderer, route);
	}
	namespace out_json_query {
		void Output(transport_catalogue::TransportCatalogue& db, std::string& query, 
			std::istream& in, std::ostream& out, renderer::MapRenderer& renderer,
			router::TransportRouter& route) {
			std::string str = "";
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
			std::istringstream strm(query);
			json::Document doc = json::Load(strm);

			proto_info::ProtoInfo proto;
			const std::filesystem::path path = detail::ParseQuerySerialization(doc.GetRoot());
			proto.Deserialization(path);

			std::vector<domain::Stop> result_stops = proto.ParseProtoStops();
			for (auto& stop : result_stops) {
				db.AddStop(stop);
			}

			std::vector<domain::Distance> result_distance = proto.ParseProtoDistance();
			for (auto& dist : result_distance) {
				for (auto& [name_stop_one, name_stop_two, distance] : dist.stop_to_stop_distance) {
					db.AddDistance(name_stop_one, name_stop_two, distance);
				}
			}

			std::vector<domain::Bus> result_bus = proto.ParseProtoBuses(db);
			for (auto& bus : result_bus) {
				db.AddBus(bus);
			}

			proto.ParseProtoMap(renderer);
			proto.ParseProtoTransportRouter(route, db);

			detail::PrintResult(db, doc.GetRoot(), out, renderer, route);
		}
	}
	namespace add_json_query {
		void AddBaseRequests(transport_catalogue::TransportCatalogue& db,
			renderer::MapRenderer& renderer, router::TransportRouter& route, std::string& query) {
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

			route = detail::ParseQueryRoute(doc.GetRoot());
			route.CreateGraph(db.GetBusNameToBus(), db);

			proto_info::ProtoInfo proto(db, renderer, route);
			const std::filesystem::path path = detail::ParseQuerySerialization(doc.GetRoot());
			proto.Serialization(path);
		}
	}

	namespace detail {
		json::Dict GetInfoBus(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem) {
			domain::QueryResultBus result;
			json::Builder builder;
			std::string str = "not found";
			result = db.GetInfoBus(elem.at("name").AsString());
			if (result.count_stops != 0) {
				builder.StartDict()
					.Key("curvature").Value(result.curvature)
					.Key("request_id").Value(elem.at("id").AsInt())
					.Key("route_length").Value(result.route_length)
					.Key("stop_count").Value(result.count_stops)
					.Key("unique_stop_count").Value(result.unique_count_stops)
					.EndDict();
			}
			else {
				builder.StartDict()
					.Key("request_id").Value(elem.at("id").AsInt())
					.Key("error_message").Value(str)
					.EndDict();
			}
			return builder.Build().AsDict();
		}

		json::Dict GetInfoStop(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem) {
			domain::QueryResultStop result;
			result = db.GetInfoStop(elem.at("name").AsString());
			json::Builder builder;
			std::string str = "not found";
			if (!result.buses.empty()) {
				builder.StartDict().Key("buses").StartArray();
				for (std::string_view bus : result.buses) {
					builder.Value((static_cast<std::string>(bus)));
				}
				builder.EndArray().Key("request_id").Value(elem.at("id").AsInt())
					.EndDict();
			}
			else {
				if (db.FindStop(result.query_name) != nullptr) {
					builder.StartDict()
						.Key("buses").StartArray().EndArray()
						.Key("request_id").Value(elem.at("id").AsInt())
						.EndDict();
				}
				else {
					builder.StartDict()
						.Key("request_id").Value(elem.at("id").AsInt())
						.Key("error_message").Value(str)
						.EndDict();
				}
			}
			return builder.Build().AsDict();
		}

		json::Dict GetInfoMap(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem, renderer::MapRenderer& renderer, router::TransportRouter& route) {
			std::stringstream out;
			json::Builder builder;
			RequestHandler result(db, renderer, route);
			svg::Document svg_doc = result.RenderMap();
			svg_doc.Render(out);
			std::string str = out.str();

			json::Node json_node(str);
			builder.StartDict()
				.Key("map").Value(json_node.AsString())
				.Key("request_id").Value(elem.at("id").AsInt())
				.EndDict();

			return builder.Build().AsDict();
		}

		json::Dict GetInfoRoute(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem, renderer::MapRenderer& renderer, router::TransportRouter& route) {
			json::Builder builder;
			RequestHandler result(db, renderer, route);
			auto doc = result.CreateRoute(db.FindStop(elem.at("from"s).AsString())->vertex_id,
				db.FindStop(elem.at("to"s).AsString())->vertex_id);
			if (!doc) {
				builder.StartDict().Key("error_message"s).Value("not found"s)
					.Key("request_id").Value(elem.at("id"s).AsInt()).EndDict();
			}
			else {
				builder.StartDict().Key("items"s).StartArray();
				for (auto& elem : doc->route) {
					builder.StartDict().Key("stop_name"s).Value(elem.stop->stop_name);
					builder.Key("time"s).Value(elem.wait_time);
					builder.Key("type"s).Value("Wait"s).EndDict();
					builder.StartDict().Key("bus"s).Value(elem.bus->bus_name);
					builder.Key("span_count"s).Value(elem.count_stops);
					builder.Key("time"s).Value(elem.run_time);
					builder.Key("type"s).Value("Bus"s).EndDict();
				}
				builder.EndArray().Key("request_id"s).Value(elem.at("id"s).AsInt())
					.Key("total_time"s).Value(doc->total_time).EndDict();
			}
			return builder.Build().AsDict();
		}

		void PrintResult(transport_catalogue::TransportCatalogue& db,
			const json::Node& elem, std::ostream& out, renderer::MapRenderer& renderer, router::TransportRouter& route) {
			json::Builder builder;
			builder.StartArray();
			auto map_base_requests = elem.AsDict().find("stat_requests");
			auto& it_node = map_base_requests->second.AsArray();
			for (auto& elem_node : it_node) {
				auto& map_result = elem_node.AsDict();
				if (map_result.count("type")) {
					if (map_result.at("type").AsString() == "Stop") {
						builder.Value(GetInfoStop(db, map_result));
					}
					else if (map_result.at("type").AsString() == "Bus") {
						builder.Value(GetInfoBus(db, map_result));
					}
					else if (map_result.at("type").AsString() == "Route") {
						builder.Value(GetInfoRoute(db, map_result, renderer, route));
					}
					else {
						builder.Value(GetInfoMap(db, map_result, renderer, route));
					}
				}
			}
			builder.EndArray();
			json::Node node_result(builder.Build().AsArray());
			json::Print(json::Document{ node_result }, out);
		}

		std::vector<domain::Stop> ParseQueryStop(const json::Node& elem) {
			std::vector<domain::Stop> vec_stops;
			auto map_base_requests = elem.AsDict().find("base_requests");
			auto& it_node = map_base_requests->second.AsArray();
			for (auto& elem_node : it_node) {
				domain::Stop stop;
				auto& map_result = elem_node.AsDict();
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
			auto map_base_requests = elem.AsDict().find("base_requests");
			auto& it_node = map_base_requests->second.AsArray();
			for (auto& elem_node : it_node) {
				domain::Distance distance;
				std::string name_one, name_two;
				int dist;
				auto& map_result = elem_node.AsDict();
				if (map_result.count("type")) {
					if (map_result.at("type").AsString() == "Stop") {
						name_one = map_result.at("name").AsString();
						auto& map_dist = map_result.at("road_distances").AsDict();
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
			auto map_base_requests = elem.AsDict().find("base_requests");
			auto& it_node = map_base_requests->second.AsArray();
			for (auto& elem_node : it_node) {
				domain::Bus bus;
				auto& map_result = elem_node.AsDict();
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
			auto map_base_requests = elem.AsDict().find("render_settings");
			auto& map_result = map_base_requests->second.AsDict();
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

		router::TransportRouter ParseQueryRoute(const json::Node& elem) {
			router::TransportRouter route;
			auto map_base_requests = elem.AsDict().find("routing_settings");
			auto& map_result = map_base_requests->second.AsDict();
			if (map_result.count("bus_wait_time")) {
				route.wait = map_result.at("bus_wait_time").AsInt();
				route.speed = map_result.at("bus_velocity").AsDouble();
			}
			return route;
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

		std::filesystem::path ParseQuerySerialization(const json::Node& elem) {
			std::filesystem::path path;
			auto map_base_requests = elem.AsDict().find("serialization_settings");
			auto& map_result = map_base_requests->second.AsDict();
			if (map_result.count("file")) {
				path = map_result.at("file").AsString();
			}
			return path;
		}

	}//close detail

}//close json_reader
#include "serialization.h"

proto_info::ProtoInfo::ProtoInfo(transport_catalogue::TransportCatalogue& db,
	renderer::MapRenderer& renderer, router::TransportRouter& route) {
	AddStops(db);
	AddDistance(db);
	AddBuses(db);
	AddMap(renderer);
	AddRoute(route);
}

void proto_info::ProtoInfo::Serialization(const std::filesystem::path& path) {
	std::ofstream out_file(path, std::ios::binary);
	t_catalogue_.SerializeToOstream(&out_file);
}

void proto_info::ProtoInfo::Deserialization(const std::filesystem::path& path) {
	std::ifstream in_file(path, std::ios::binary);
	t_catalogue_.ParseFromIstream(&in_file);
}

std::vector<domain::Bus> proto_info::ProtoInfo::ParseProtoBuses(transport_catalogue::TransportCatalogue& db) {
	std::vector<domain::Bus> result_bus;
	result_bus.reserve(t_catalogue_.buses_size());

	for (size_t i = 0; i < t_catalogue_.buses_size(); ++i) {
		domain::Bus bus;
		bus.is_roundtrip = t_catalogue_.mutable_buses(i)->is_roundtrip();
		bus.bus_name = t_catalogue_.mutable_buses(i)->bus_name();
		for (size_t j = 0; j < t_catalogue_.mutable_buses(i)->route_size(); ++j) {
			bus.route.push_back(db.FindStop(t_catalogue_.mutable_buses(i)->mutable_route(j)->stop_name()));
		}
		result_bus.push_back(bus);
	}

	return result_bus;
}

void proto_info::ProtoInfo::ParseProtoMap(renderer::MapRenderer& renderer) {
	renderer.width_ = t_catalogue_.mutable_map()->width();
	renderer.height_ = t_catalogue_.mutable_map()->height();
	renderer.padding_ = t_catalogue_.mutable_map()->padding();
	renderer.stop_radius_ = t_catalogue_.mutable_map()->stop_radius();
	renderer.line_width_ = t_catalogue_.mutable_map()->line_width();
	renderer.bus_label_font_size_ = t_catalogue_.mutable_map()->bus_label_font_size();
	for (size_t i = 0; i < t_catalogue_.mutable_map()->bus_label_offset_size(); ++i) {
		renderer.bus_label_offset.push_back(t_catalogue_.mutable_map()->bus_label_offset(i));
	}
	renderer.stop_label_font_size_ = t_catalogue_.mutable_map()->stop_label_font_size();
	for (size_t i = 0; i < t_catalogue_.mutable_map()->stop_label_offset_size(); ++i) {
		renderer.stop_label_offset.push_back(t_catalogue_.mutable_map()->stop_label_offset(i));
	}
	renderer.underlayer_width_ = t_catalogue_.mutable_map()->underlayer_width();
	AddColorOutProto(renderer);
	AddColorPaletteOutProto(renderer);
}

void proto_info::ProtoInfo::ParseProtoTransportRouter(router::TransportRouter& route,
	transport_catalogue::TransportCatalogue& db) {
	ParseProtoGraph(route);
	ParseProtoEdgeInfo(route, db);
	ParseProtoRouter(route);
}

void proto_info::ProtoInfo::AddBuses(transport_catalogue::TransportCatalogue& db) {
	for (auto& bus : db.GetBusDeque()) {
		t_catalogue_proto::Bus proto_bus;
		proto_bus.set_is_roundtrip(bus.is_roundtrip);
		proto_bus.set_bus_name(bus.bus_name);
		for (auto& stop : bus.route) {
			t_catalogue_proto::Stop proto_stop;
			proto_stop.set_vertex_id(stop->vertex_id);
			proto_stop.set_stop_name(stop->stop_name);
			proto_stop.mutable_coordinates_()->set_lat(stop->coordinates_.lat);
			proto_stop.mutable_coordinates_()->set_lng(stop->coordinates_.lng);
			*proto_bus.mutable_route()->Add() = proto_stop;
		}
		*t_catalogue_.mutable_buses()->Add() = proto_bus;
	}
}

void proto_info::ProtoInfo::AddDistance(transport_catalogue::TransportCatalogue& db) {
	for (auto& stop : db.GetStopDeque()) {
		t_catalogue_proto::Stop proto_stop;
		proto_stop.set_stop_name(stop.stop_name);
		proto_stop.mutable_coordinates_()->set_lat(stop.coordinates_.lat);
		proto_stop.mutable_coordinates_()->set_lng(stop.coordinates_.lng);
		proto_stop.set_vertex_id(stop.vertex_id);
		*t_catalogue_.mutable_stops()->Add() = proto_stop;
	}
}

void proto_info::ProtoInfo::AddStops(transport_catalogue::TransportCatalogue& db) {
	for (auto& [stops, distance] : db.GetStopsDistance()) {
		t_catalogue_proto::Dist proto_dist;
		proto_dist.set_stop_one(stops.first->stop_name);
		proto_dist.set_stop_two(stops.second->stop_name);
		proto_dist.set_distance(distance);
		*t_catalogue_.mutable_stops_distance()->Add() = proto_dist;
	}
}

void proto_info::ProtoInfo::AddMap(renderer::MapRenderer& renderer) {
	t_catalogue_.mutable_map()->set_width(renderer.width_);
	t_catalogue_.mutable_map()->set_height(renderer.height_);
	t_catalogue_.mutable_map()->set_padding(renderer.padding_);
	t_catalogue_.mutable_map()->set_stop_radius(renderer.stop_radius_);
	t_catalogue_.mutable_map()->set_line_width(renderer.line_width_);
	t_catalogue_.mutable_map()->set_bus_label_font_size(renderer.bus_label_font_size_);
	for (auto& elem : renderer.bus_label_offset) {
		t_catalogue_.mutable_map()->add_bus_label_offset(elem);
	}
	t_catalogue_.mutable_map()->set_stop_label_font_size(renderer.stop_label_font_size_);
	for (auto& elem : renderer.stop_label_offset) {
		t_catalogue_.mutable_map()->add_stop_label_offset(elem);
	}
	t_catalogue_.mutable_map()->set_underlayer_width(renderer.underlayer_width_);
	AddColorInProto(renderer);
	AddColorPaletteInProto(renderer);
}

void proto_info::ProtoInfo::AddRoute(router::TransportRouter& route) {
	HelperAddGraphInProto(route);
	HelperAddEdgesInProto(route);
	HelperAddRouterInProto(route);
	t_catalogue_.mutable_transport_router()->set_wait(route.wait);
	t_catalogue_.mutable_transport_router()->set_speed(route.speed);
}

void proto_info::ProtoInfo::AddColorInProto(renderer::MapRenderer& renderer) {
	switch (renderer.underlayer_color_.index()) {
	case 1: {
		t_catalogue_.mutable_map()->mutable_underlayer_color()->
			set_str_color(std::get<std::string>(renderer.underlayer_color_));
		break;
	}
	case 2: {
		svg::Rgb rgb;
		rgb = std::get<svg::Rgb>(renderer.underlayer_color_);
		t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgb_color()->set_r(rgb.red);
		t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgb_color()->set_g(rgb.green);
		t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgb_color()->set_b(rgb.blue);
		break;
	}
	case 3: {
		svg::Rgba rgba;
		rgba = std::get<svg::Rgba>(renderer.underlayer_color_);
		t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgba_color()->set_r(rgba.red);
		t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgba_color()->set_g(rgba.green);
		t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgba_color()->set_b(rgba.blue);
		t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgba_color()->set_opacity(rgba.opacity);
		break;
	}
	default: { break; }
	}
}

void proto_info::ProtoInfo::AddColorPaletteInProto(renderer::MapRenderer& renderer) {
	for (auto& elem : renderer.color_palette_) {
		switch (elem.index()) {
		case 1: {
			t_catalogue_proto::Color color;
			color.set_str_color(std::get<std::string>(elem));
			*t_catalogue_.mutable_map()->mutable_color_palette_()->Add() = color;
			break;
		}
		case 2: {
			t_catalogue_proto::Color color;
			svg::Rgb rgb;
			rgb = std::get<svg::Rgb>(elem);
			color.mutable_rgb_color()->set_r(rgb.red);
			color.mutable_rgb_color()->set_g(rgb.green);
			color.mutable_rgb_color()->set_b(rgb.blue);
			*t_catalogue_.mutable_map()->mutable_color_palette_()->Add() = color;
			break;
		}
		case 3: {
			t_catalogue_proto::Color color;
			svg::Rgba rgba;
			rgba = std::get<svg::Rgba>(elem);
			color.mutable_rgba_color()->set_r(rgba.red);
			color.mutable_rgba_color()->set_g(rgba.green);
			color.mutable_rgba_color()->set_b(rgba.blue);
			color.mutable_rgba_color()->set_opacity(rgba.opacity);
			*t_catalogue_.mutable_map()->mutable_color_palette_()->Add() = color;
			break;
		}
		default: { break; }
		}
	}
}

void proto_info::ProtoInfo::AddColorOutProto(renderer::MapRenderer& renderer) {
	if (t_catalogue_.mutable_map()->mutable_underlayer_color()->col_case() == 1) {
		renderer.underlayer_color_ = t_catalogue_.mutable_map()->mutable_underlayer_color()->str_color();
	}
	else if (t_catalogue_.mutable_map()->mutable_underlayer_color()->col_case() == 2) {
		svg::Rgb rgb;
		rgb.red = t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgb_color()->r();
		rgb.green = t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgb_color()->g();
		rgb.blue = t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgb_color()->b();
		renderer.underlayer_color_ = rgb;
	}
	else if (t_catalogue_.mutable_map()->mutable_underlayer_color()->col_case() == 3) {
		svg::Rgba rgba;
		rgba.red = t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgba_color()->r();
		rgba.green = t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgba_color()->g();
		rgba.blue = t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgba_color()->b();
		rgba.opacity = t_catalogue_.mutable_map()->mutable_underlayer_color()->mutable_rgba_color()->opacity();
		renderer.underlayer_color_ = rgba;
	}
	else {
		renderer.underlayer_color_ = std::monostate{};
	}
}

void proto_info::ProtoInfo::AddColorPaletteOutProto(renderer::MapRenderer& renderer) {
	for (size_t i = 0; i < t_catalogue_.mutable_map()->color_palette__size(); ++i) {
		if (t_catalogue_.mutable_map()->color_palette_(i).col_case() == 1) {
			renderer.color_palette_.push_back(t_catalogue_.mutable_map()->mutable_color_palette_(i)->str_color());
		}
		else if (t_catalogue_.mutable_map()->mutable_color_palette_(i)->col_case() == 2) {
			svg::Rgb rgb;
			rgb.red = t_catalogue_.mutable_map()->mutable_color_palette_(i)->mutable_rgb_color()->r();
			rgb.green = t_catalogue_.mutable_map()->mutable_color_palette_(i)->mutable_rgb_color()->g();
			rgb.blue = t_catalogue_.mutable_map()->mutable_color_palette_(i)->mutable_rgb_color()->b();
			renderer.color_palette_.push_back(rgb);
		}
		else if (t_catalogue_.mutable_map()->mutable_color_palette_(i)->col_case() == 3) {
			svg::Rgba rgba;
			rgba.red = t_catalogue_.mutable_map()->mutable_color_palette_(i)->mutable_rgba_color()->r();
			rgba.green = t_catalogue_.mutable_map()->mutable_color_palette_(i)->mutable_rgba_color()->g();
			rgba.blue = t_catalogue_.mutable_map()->mutable_color_palette_(i)->mutable_rgba_color()->b();
			rgba.opacity = t_catalogue_.mutable_map()->mutable_color_palette_(i)->mutable_rgba_color()->opacity();
			renderer.color_palette_.push_back(rgba);
		}
		else {
			renderer.color_palette_.push_back(std::monostate{});
		}
	}
}

void proto_info::ProtoInfo::HelperAddGraphInProto(router::TransportRouter& route) {
	for (size_t i = 0; i < route.GetGraph().GetEdgesVector().size(); ++i) {
		t_catalogue_proto::Edge edge;
		edge.set_from(route.GetGraph().GetEdgesVector()[i].from);
		edge.set_to(route.GetGraph().GetEdgesVector()[i].to);
		edge.set_weight(route.GetGraph().GetEdgesVector()[i].weight);
		*t_catalogue_.mutable_transport_router()->mutable_graph()->mutable_edges()->Add() = edge;
	}

	for (size_t i = 0; i < route.GetGraph().GetIncidenceLists().size(); ++i) {
		t_catalogue_proto::IncidenceList incidence_list;
		for (size_t j = 0; j < route.GetGraph().GetIncidenceLists()[i].size(); ++j) {
			*incidence_list.mutable_list()->Add() = route.GetGraph().GetIncidenceLists()[i][j];
		}
		*t_catalogue_.mutable_transport_router()->mutable_graph()->mutable_incidence_lists()->Add() = incidence_list;
	}
}

void proto_info::ProtoInfo::HelperAddEdgesInProto(router::TransportRouter& route) {
	for (auto& [id, info] : route.GetEdges()) {
		t_catalogue_proto::EdgeInfo edge_info;
		edge_info.set_edge_id(id);
		edge_info.set_stop_name(info.stop->stop_name);
		edge_info.set_bus_name(info.bus->bus_name);
		edge_info.set_count(info.count);
		*t_catalogue_.mutable_transport_router()->mutable_edges_()->Add() = edge_info;
	}
}

void proto_info::ProtoInfo::HelperAddRouterInProto(router::TransportRouter& route) {
	for (size_t i = 0; i < route.GetRouter()->GetRoutesInternalData().size(); ++i) {
		t_catalogue_proto::HelpRepeated help_repeated;
		for (size_t j = 0; j < route.GetRouter()->GetRoutesInternalData()[i].size(); ++j) {
			t_catalogue_proto::HelpOpt help_opt;
			if (route.GetRouter()->GetRoutesInternalData()[i][j].has_value()) {
				help_opt.mutable_help_opt()->set_weight(route.GetRouter()->GetRoutesInternalData()[i][j].value().weight);
				if (route.GetRouter()->GetRoutesInternalData()[i][j].value().prev_edge.has_value()) {
					help_opt.mutable_help_opt()->set_prev_edge(route.GetRouter()->
						GetRoutesInternalData()[i][j].value().prev_edge.value());
				}
			}
			*help_repeated.mutable_help_repeated()->Add() = help_opt;
		}
		*t_catalogue_.mutable_transport_router()->mutable_router_()->
			mutable_routes_internal_data()->mutable_routes()->Add() = help_repeated;
	}
}

void proto_info::ProtoInfo::ParseProtoGraph(router::TransportRouter& route) {
	for (size_t i = 0; i < t_catalogue_.mutable_transport_router()->mutable_graph()->edges_size(); ++i) {
		graph::Edge<double> edge;
		edge.from = t_catalogue_.mutable_transport_router()->mutable_graph()->mutable_edges(i)->from();
		edge.to = t_catalogue_.mutable_transport_router()->mutable_graph()->mutable_edges(i)->to();
		edge.weight = t_catalogue_.mutable_transport_router()->mutable_graph()->mutable_edges(i)->weight();
		route.GetGraph().GetEdgesVector().push_back(edge);
	}

	for (size_t i = 0; i < t_catalogue_.mutable_transport_router()->mutable_graph()->incidence_lists_size(); ++i) {
		std::vector<graph::EdgeId> incidence_list;
		incidence_list.reserve(t_catalogue_.mutable_transport_router()->
			mutable_graph()->mutable_incidence_lists(i)->list_size());
		for (size_t j = 0; j < t_catalogue_.mutable_transport_router()->
			mutable_graph()->mutable_incidence_lists(i)->list_size(); ++j) {
			incidence_list.push_back(t_catalogue_.mutable_transport_router()->
				mutable_graph()->mutable_incidence_lists(i)->list(j));
		}
		route.GetGraph().GetIncidenceLists().push_back(incidence_list);
	}
}

void proto_info::ProtoInfo::ParseProtoEdgeInfo(router::TransportRouter& route, transport_catalogue::TransportCatalogue& db) {
	for (size_t i = 0; i < t_catalogue_.mutable_transport_router()->edges__size(); ++i) {
		router::EdgeInfo edge_info;
		edge_info.stop = db.FindStop(t_catalogue_.mutable_transport_router()->mutable_edges_(i)->stop_name());
		edge_info.bus = db.FindBus(t_catalogue_.mutable_transport_router()->mutable_edges_(i)->bus_name());
		edge_info.count = t_catalogue_.mutable_transport_router()->mutable_edges_(i)->count();
		route.GetEdges().insert({ t_catalogue_.mutable_transport_router()->mutable_edges_(i)->edge_id(), edge_info });
	}
}

void proto_info::ProtoInfo::ParseProtoRouter(router::TransportRouter& route) {
	std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>> buffer;
	buffer.reserve(t_catalogue_.mutable_transport_router()->
		mutable_router_()->mutable_routes_internal_data()->routes_size());

	for (size_t i = 0; i < t_catalogue_.mutable_transport_router()->
		mutable_router_()->mutable_routes_internal_data()->routes_size(); ++i) {
		std::vector<std::optional<graph::Router<double>::RouteInternalData>> result;
		result.reserve(t_catalogue_.mutable_transport_router()->
			mutable_router_()->mutable_routes_internal_data()->mutable_routes(i)->help_repeated_size());

		for (size_t j = 0; j < t_catalogue_.mutable_transport_router()->
			mutable_router_()->mutable_routes_internal_data()->mutable_routes(i)->help_repeated_size(); ++j) {
			graph::Router<double>::RouteInternalData route_internal_data;
			if (t_catalogue_.mutable_transport_router()->
				mutable_router_()->mutable_routes_internal_data()->
				mutable_routes(i)->mutable_help_repeated(j)->h_opt_case() == 1) {
				route_internal_data.weight = t_catalogue_.mutable_transport_router()->
					mutable_router_()->mutable_routes_internal_data()->
					mutable_routes(i)->mutable_help_repeated(j)->mutable_help_opt()->weight();
				if (t_catalogue_.mutable_transport_router()->
					mutable_router_()->mutable_routes_internal_data()->
					mutable_routes(i)->mutable_help_repeated(j)->mutable_help_opt()->TEST_case() == 2) {
					route_internal_data.prev_edge = std::make_optional<size_t>(t_catalogue_.mutable_transport_router()->
						mutable_router_()->mutable_routes_internal_data()->
						mutable_routes(i)->mutable_help_repeated(j)->mutable_help_opt()->prev_edge());
				}
				result.push_back(std::make_optional<graph::Router<double>::RouteInternalData>(route_internal_data));
			}
			else {
				result.push_back(std::nullopt);
			}
			
		}
		buffer.push_back(result);
	}

	route.GetRouter() = std::make_unique<graph::Router<double>>(route.GetGraph(), buffer);
}

std::vector<domain::Stop> proto_info::ProtoInfo::ParseProtoStops() {
	std::vector<domain::Stop> result_stops;
	result_stops.reserve(t_catalogue_.stops_size());

	for (size_t i = 0; i < t_catalogue_.stops_size(); ++i) {
		domain::Stop stop;
		stop.vertex_id = t_catalogue_.mutable_stops(i)->vertex_id();
		stop.stop_name = t_catalogue_.mutable_stops(i)->stop_name();
		stop.coordinates_.lat = t_catalogue_.mutable_stops(i)->coordinates_().lat();
		stop.coordinates_.lng = static_cast<double>(t_catalogue_.mutable_stops(i)->coordinates_().lng());
		result_stops.push_back(stop);
	}
	return result_stops;
}

std::vector<domain::Distance> proto_info::ProtoInfo::ParseProtoDistance() {
	std::vector<domain::Distance> result_distance;
	result_distance.reserve(t_catalogue_.stops_distance_size());

	for (size_t i = 0; i < t_catalogue_.stops_distance_size(); ++i) {
		domain::Distance distance;
		distance.stop_to_stop_distance.insert({ t_catalogue_.mutable_stops_distance(i)->stop_one(),
			t_catalogue_.mutable_stops_distance(i)->stop_two(), t_catalogue_.mutable_stops_distance(i)->distance() });
		result_distance.push_back(distance);
	}

	return result_distance;
}
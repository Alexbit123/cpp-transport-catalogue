#include "map_renderer.h"

constexpr double EPSILON = 1e-6;

namespace renderer {
	bool IsZero(double value) {		//После удаления табуляции и, например, последующем редактировании кода 
		return std::abs(value) < EPSILON;   //сочетание клавишь ctrl+k+d возвращают табуляцию ))))
	}
	svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}
	std::vector<std::string_view> MapRenderer::GetSortBuses(
		const std::unordered_map<std::string_view,
		domain::Bus*>& info_bus) const {
		std::vector<std::string_view> buses_name;
		for (auto& [key, value] : info_bus) {
			buses_name.push_back(key);
		}
		std::sort(buses_name.begin(), buses_name.end());

		return buses_name;
	}
	StopCoord MapRenderer::GetCoord(const std::unordered_map<std::string_view,
		domain::Bus*>& info_bus, std::vector<std::string_view>& buses_name) const {
		std::vector<geo::Coordinates> geo_coords;
		std::vector<svg::Point> screen_coord;
		StopCoord buf;
		int index_offset_coord = 0, counter = 0;
		for (std::string_view name : buses_name) {
			for (auto elem : info_bus.at(name)->route) {
				geo_coords.push_back(elem->coordinates_);
			}
		}
		SphereProjector proj(geo_coords.begin(), geo_coords.end(), width_, height_, padding_);

		for (const auto& geo_coord : geo_coords) {
			screen_coord.push_back(proj(geo_coord));
		}

		for (std::string_view name : buses_name) {
			index_offset_coord += counter;
			counter = 0;
			for (size_t i = 0; i < info_bus.at(name)->route.size(); ++i) {
				buf.stop_to_coord[info_bus.at(name)->route[i]->stop_name] = screen_coord[i + index_offset_coord];
				++counter;
			}

		}

		return buf;
	}
	void MapRenderer::VisualizePolyline(svg::Document& doc,
		const std::unordered_map<std::string_view,
		domain::Bus*>& info_bus, std::vector<std::string_view>& buses_name,
		StopCoord& stop_to_coord) const {
		int index_offset_color = 0;
		for (size_t i = 0; i < buses_name.size(); ++i) {
			if (info_bus.at(buses_name[i])->route.size() != 0) {
				svg::Polyline polyline;
				for (size_t j = 0; j < info_bus.at(buses_name[i])->route.size(); ++j) {
					polyline.AddPoint(stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->route[j]->stop_name))
						.SetFillColor("none")
						.SetStrokeColor(color_palette_[(i - index_offset_color) % color_palette_.size()])
						.SetStrokeWidth(line_width_)
						.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
						.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				}
				doc.Add(polyline);
			}
			else {
				++index_offset_color;
			}
		}
	}
	void MapRenderer::VisualizeNameRoute(svg::Document& doc,
		const std::unordered_map<std::string_view, domain::Bus*>& info_bus,
		std::vector<std::string_view>& buses_name, StopCoord& stop_to_coord) const {
		int index_offset_color = 0;
		for (size_t i = 0; i < buses_name.size(); ++i) {
			if (info_bus.at(buses_name[i])->route.size() != 0) {
				svg::Text text_name_route, text_substrate;
				if (info_bus.at(buses_name[i])->is_roundtrip) {
					text_substrate.SetData(info_bus.at(buses_name[i])->bus_name)
						.SetFillColor(underlayer_color_).SetFontFamily("Verdana")
						.SetFontSize(bus_label_font_size_).SetFontWeight("bold")
						.SetOffset({ bus_label_offset[0], bus_label_offset[1] })
						.SetPosition(stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->route[0]->stop_name))
						.SetStrokeColor(underlayer_color_)
						.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
						.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
						.SetStrokeWidth(underlayer_width_);
					text_name_route.SetData(info_bus.at(buses_name[i])->bus_name)
						.SetFillColor(color_palette_[(i - index_offset_color) % color_palette_.size()])
						.SetOffset({ bus_label_offset[0], bus_label_offset[1] })
						.SetPosition(stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->route[0]->stop_name))
						.SetFontSize(bus_label_font_size_).SetFontWeight("bold")
						.SetFontFamily("Verdana");
					doc.Add(text_substrate);
					doc.Add(text_name_route);
				}
				else {
					text_substrate.SetData(info_bus.at(buses_name[i])->bus_name)
						.SetFillColor(underlayer_color_).SetFontFamily("Verdana")
						.SetFontSize(bus_label_font_size_).SetFontWeight("bold")
						.SetOffset({ bus_label_offset[0], bus_label_offset[1] })
						.SetPosition(stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->route[0]->stop_name))
						.SetStrokeColor(underlayer_color_)
						.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
						.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
						.SetStrokeWidth(underlayer_width_);
					text_name_route.SetData(info_bus.at(buses_name[i])->bus_name)
						.SetFillColor(color_palette_[(i - index_offset_color) % color_palette_.size()])
						.SetOffset({ bus_label_offset[0], bus_label_offset[1] })
						.SetPosition(stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->route[0]->stop_name))
						.SetFontSize(bus_label_font_size_).SetFontWeight("bold")
						.SetFontFamily("Verdana");
					doc.Add(text_substrate);
					doc.Add(text_name_route);

					if (info_bus.at(buses_name[i])->route[0]->stop_name !=
						info_bus.at(buses_name[i])->route[info_bus.at(buses_name[i])->route.size() / 2]->stop_name ||
						(stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->route[0]->stop_name).x !=
							stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->
								route[info_bus.at(buses_name[i])->route.size() / 2]->stop_name).x &&
							stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->route[0]->stop_name).y !=
							stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->
								route[info_bus.at(buses_name[i])->route.size() / 2]->stop_name).y)) {
						text_substrate.SetData(info_bus.at(buses_name[i])->bus_name)
							.SetFillColor(underlayer_color_).SetFontFamily("Verdana")
							.SetFontSize(bus_label_font_size_).SetFontWeight("bold")
							.SetOffset({ bus_label_offset[0], bus_label_offset[1] })
							.SetPosition(stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->
								route[info_bus.at(buses_name[i])->route.size() / 2]->stop_name))
							.SetStrokeColor(underlayer_color_)
							.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
							.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
							.SetStrokeWidth(underlayer_width_);
						text_name_route.SetData(info_bus.at(buses_name[i])->bus_name)
							.SetFillColor(color_palette_[(i - index_offset_color) % color_palette_.size()])
							.SetOffset({ bus_label_offset[0], bus_label_offset[1] })
							.SetPosition(stop_to_coord.stop_to_coord.at(info_bus.at(buses_name[i])->
								route[info_bus.at(buses_name[i])->route.size() / 2]->stop_name))
							.SetFontSize(bus_label_font_size_).SetFontWeight("bold")
							.SetFontFamily("Verdana");
						doc.Add(text_substrate);
						doc.Add(text_name_route);
					}
				}
			}
			else {
				++index_offset_color;
			}
		}
	}
	void MapRenderer::VisualizeStop(svg::Document& doc, StopCoord& stop_to_coord) const {
		svg::Circle circle;
		for (auto& [name_stop, coord] : stop_to_coord.stop_to_coord) {
			circle.SetCenter(coord)
				.SetRadius(stop_radius_).SetFillColor("white");
			doc.Add(circle);
		}
	}
	void MapRenderer::VisualizeNameStop(svg::Document& doc, StopCoord& stop_to_coord) const {
		svg::Text text_substrate, text_name_route;
		for (auto& [name_stop, coord] : stop_to_coord.stop_to_coord) {
			text_substrate.SetData(name_stop)
				.SetFillColor(underlayer_color_).SetFontFamily("Verdana")
				.SetFontSize(stop_label_font_size_)
				.SetOffset({ stop_label_offset[0], stop_label_offset[1] })
				.SetPosition(coord)
				.SetStrokeColor(underlayer_color_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
				.SetStrokeWidth(underlayer_width_);
			text_name_route.SetData(name_stop)
				.SetFillColor("black")
				.SetOffset({ stop_label_offset[0], stop_label_offset[1] })
				.SetPosition(coord)
				.SetFontSize(stop_label_font_size_)
				.SetFontFamily("Verdana");
			doc.Add(text_substrate);
			doc.Add(text_name_route);
		}
	}
	svg::Document MapRenderer::GetMap(const std::unordered_map<std::string_view,
		domain::Bus*>& info_bus) const {
		svg::Document doc;
		std::vector<std::string_view> buses_name = GetSortBuses(info_bus);
		StopCoord stop_to_coord = GetCoord(info_bus, buses_name);

		VisualizePolyline(doc, info_bus, buses_name, stop_to_coord);
		VisualizeNameRoute(doc, info_bus, buses_name, stop_to_coord);
		VisualizeStop(doc, stop_to_coord);
		VisualizeNameStop(doc, stop_to_coord);

		return doc;
	}
}//close renderer

#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <map>

namespace renderer {
	bool IsZero(double value);
											
	class SphereProjector {
	public:
		// points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
		template <typename PointInputIt>
		SphereProjector(PointInputIt points_begin, PointInputIt points_end,
			double max_width, double max_height, double padding)
			: padding_(padding) //
		{
			// ���� ����� ����������� ����� �� ������, ��������� ������
			if (points_begin == points_end) {
				return;
			}

			// ������� ����� � ����������� � ������������ ��������
			const auto [left_it, right_it] = std::minmax_element(
				points_begin, points_end,
				[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
			min_lon_ = left_it->lng;
			const double max_lon = right_it->lng;

			// ������� ����� � ����������� � ������������ �������
			const auto [bottom_it, top_it] = std::minmax_element(
				points_begin, points_end,
				[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
			const double min_lat = bottom_it->lat;
			max_lat_ = top_it->lat;

			// ��������� ����������� ��������������� ����� ���������� x
			std::optional<double> width_zoom;
			if (!IsZero(max_lon - min_lon_)) {
				width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
			}

			// ��������� ����������� ��������������� ����� ���������� y
			std::optional<double> height_zoom;
			if (!IsZero(max_lat_ - min_lat)) {
				height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
			}

			if (width_zoom && height_zoom) {
				// ������������ ��������������� �� ������ � ������ ���������,
				// ���� ����������� �� ���
				zoom_coeff_ = std::min(*width_zoom, *height_zoom);
			}
			else if (width_zoom) {
				// ����������� ��������������� �� ������ ���������, ���������� ���
				zoom_coeff_ = *width_zoom;
			}
			else if (height_zoom) {
				// ����������� ��������������� �� ������ ���������, ���������� ���
				zoom_coeff_ = *height_zoom;
			}
		}

		// ���������� ������ � ������� � ���������� ������ SVG-�����������
		svg::Point operator()(geo::Coordinates coords) const;

	private:
		double padding_;
		double min_lon_ = 0;
		double max_lat_ = 0;
		double zoom_coeff_ = 0;
	};

	struct StopCoord {
		std::map<std::string, svg::Point> stop_to_coord;
	};

	struct MapRenderer {
	private:
		std::vector<std::string_view> GetSortBuses(const std::unordered_map<std::string_view,
			domain::Bus*>& info_bus) const;

		StopCoord GetCoord(const std::unordered_map<std::string_view, domain::Bus*>& info_bus,
			std::vector<std::string_view>& buses_name) const;

		void VisualizePolyline(svg::Document& doc, const std::unordered_map<std::string_view, domain::Bus*>& info_bus,
			std::vector<std::string_view>& buses_name, StopCoord& stop_to_coord) const;

		void VisualizeNameRoute(svg::Document& doc, const std::unordered_map<std::string_view, domain::Bus*>& info_bus,
			std::vector<std::string_view>& buses_name, StopCoord& stop_to_coord) const;

		void VisualizeStop(svg::Document& doc, StopCoord& stop_to_coord) const;

		void VisualizeNameStop(svg::Document& doc, StopCoord& stop_to_coord) const;
	public:
		svg::Document GetMap(const std::unordered_map<std::string_view, domain::Bus*>& info_bus) const;

		double width_, height_, padding_, line_width_, stop_radius_;
		int bus_label_font_size_, stop_label_font_size_;
		std::vector<double> bus_label_offset, stop_label_offset;
		svg::Color underlayer_color_;
		double underlayer_width_;
		std::vector<svg::Color> color_palette_;
	};

}//close renderer



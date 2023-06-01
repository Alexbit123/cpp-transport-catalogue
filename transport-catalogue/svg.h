#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <string_view>

namespace svg {
	using namespace std::literals;

	struct Rgb {
		Rgb() = default;
		Rgb(uint8_t red, uint8_t green, uint8_t blue) 
			: red(red), green(green), blue(blue) {
		}

		uint8_t red = 0, green = 0, blue = 0;
	};

	struct Rgba {
		Rgba() = default;
		Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
			: red(red), green(green), blue(blue), opacity(opacity) {
		}

		uint8_t red = 0, green = 0, blue = 0;
		double opacity = 1.0;
	};

	using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

	inline const Color NoneColor{ "none" };


	struct OstreamColorPrinter {
		std::ostream& out;

		void operator()(std::monostate) const {
			out << "none"sv;
		}
		void operator()(std::string str) const {
			out << str;
		}
		void operator()(Rgb color) const {
			out << "rgb("sv << static_cast<int>(color.red) 
				<< ","sv << static_cast<int>(color.green) << ","sv 
				<< static_cast<int>(color.blue) <<")";
		}
		void operator()(Rgba color) const {
			out << "rgba("sv << static_cast<int>(color.red) << ","sv 
				<< static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) 
				<< ","sv << color.opacity << ")";
		}
	};

	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE,
	};

	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
	};

	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0;
		double y = 0;
	};

	std::ostream& operator<<(std::ostream& out, const StrokeLineCap& elem);

	std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& elem);

	/*
	 * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
	 * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
	 */
	struct RenderContext {
		RenderContext(std::ostream& out)
			: out(out) {
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out)
			, indent_step(indent_step)
			, indent(indent) {
		}

		RenderContext Indented() const {
			return { out, indent_step, indent + indent_step };
		}

		void RenderIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};

	/*
	 * ����������� ������� ����� Object ������ ��� ���������������� ��������
	 * ���������� ����� SVG-���������
	 * ��������� ������� "��������� �����" ��� ������ ����������� ����
	 */
	class Object {
	public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;

	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};

	class ObjectContainer {
	public:
		template <typename Obj>
		void Add(Obj obj) {
			AddPtr(std::make_unique<Obj>(std::move(obj)));
		}

		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

		virtual ~ObjectContainer() = default;

	};

	class Drawable {
	public:
		virtual void Draw(svg::ObjectContainer& container) const = 0;

		virtual ~Drawable() = default;
	};

	template<typename Owner>
	class PathProps {
	public:
		//����� �������� �������� fill � ���� �������. 
		//�� ��������� �������� �� ���������.
		Owner& SetFillColor(Color fill_color) {
			fill_color_ = fill_color;
			return AsOwner();
		}

		//����� �������� �������� stroke � ���� �������. 
		//�� ��������� �������� �� ���������.
		Owner& SetStrokeColor(Color stroke_color) {
			stroke_color_ = stroke_color;
			return AsOwner();
		}

		//����� �������� �������� stroke-width � ������� �����. 
		//�� ��������� �������� �� ���������.
		Owner& SetStrokeWidth(double width) {
			width_ = width;
			return AsOwner();
		}

		//����� �������� �������� stroke-linecap � ��� ����� ����� �����.
		//�� ��������� �������� �� ���������.
		Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
			line_cap_ = line_cap;
			return AsOwner();
		}

		//����� �������� �������� stroke-linejoin � ��� ����� ���������� �����. 
		//�� ��������� �������� �� ���������.
		Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
			line_join_ = line_join;
			return AsOwner();
		}

	protected:
		~PathProps() = default;

		void RenderAttrs(std::ostream& out) const {

			if (fill_color_) {
				out << " fill=\""sv; 
				std::visit(OstreamColorPrinter{ out }, *fill_color_); 
				out << "\""sv;
			}
			if (stroke_color_) {
				out << " stroke=\""sv; 
				std::visit(OstreamColorPrinter{ out }, *stroke_color_); 
				out << "\""sv;
			}
			if (width_) {
				out << " stroke-width=\""sv << *width_ << "\""sv;
			}
			if (line_cap_) {
				out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
			}
			if (line_join_) {
				out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
			}
		}

	private:
		Owner& AsOwner() {
			// static_cast ��������� ����������� *this � Owner&,
			// ���� ����� Owner � ��������� PathProps
			return static_cast<Owner&>(*this);
		}

		std::optional<Color> fill_color_;
		std::optional<Color> stroke_color_;
		std::optional<double> width_;
		std::optional<StrokeLineCap> line_cap_;
		std::optional<StrokeLineJoin> line_join_;
	};


	/*
	 * ����� Circle ���������� ������� <circle> ��� ����������� �����
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_;
		double radius_ = 1.0;
	};

	/*
	 * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	 */
	class Polyline final : public Object, public PathProps<Polyline> {
	public:
		// ��������� ��������� ������� � ������� �����
		Polyline& AddPoint(Point point);

	private:
		void RenderObject(const RenderContext& context) const override;

		std::vector<Point> vertex;
	};

	/*
	 * ����� Text ���������� ������� <text> ��� ����������� ������
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	 */
	class Text final : public Object, public PathProps<Text> {
	public:
		// ����� ���������� ������� ����� (�������� x � y)
		Text& SetPosition(Point pos);

		// ����� �������� ������������ ������� ����� (�������� dx, dy)
		Text& SetOffset(Point offset);

		// ����� ������� ������ (������� font-size)
		Text& SetFontSize(uint32_t size);

		// ����� �������� ������ (������� font-family)
		Text& SetFontFamily(std::string font_family);

		// ����� ������� ������ (������� font-weight)
		Text& SetFontWeight(std::string font_weight);

		// ����� ��������� ���������� ������� (������������ ������ ���� text)
		Text& SetData(std::string data);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point pos_;
		Point offset_;
		uint32_t size_ = 1;
		std::string font_family_;
		std::string font_weight_;
		std::string data_;
	};

	class Document : public ObjectContainer {
	public:
		/*
		 ����� Add ��������� � svg-�������� ����� ������-��������� svg::Object.
		 ������ �������������:
		 Document doc;
		 doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
		*/

		// ��������� � svg-�������� ������-��������� svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj) override;

		// ������� � ostream svg-������������� ���������
		void Render(std::ostream& out) const;

		// ������ ������ � ������, ����������� ��� ���������� ������ Document

	private:
		std::vector<std::unique_ptr<Object>> objects;
	};

}  // namespace svg
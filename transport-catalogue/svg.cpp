#include "svg.h"

namespace svg {

	using namespace std::literals;

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}

	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\""sv;
		RenderAttrs(out);
		out << "/>"sv;
	}

	// ---------- Polyline ------------------

	Polyline& Polyline::AddPoint(Point point) {
		vertex.push_back(point);
		return *this;
	}

	void Polyline::RenderObject(const RenderContext& context) const {
		bool is_start = false;
		auto& out = context.out;
		out << "<polyline points=\""sv;
		for (auto& point : vertex) {
			if (is_start) {
				out << " ";
			}
			else {
				is_start = true;
			}
			out << point.x << "," << point.y;
		}
		out << "\"";
		RenderAttrs(out);
		out<< "/>";
	}

	// ---------- Text ------------------

	void Text::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		std::string result = "";
		for (char ch : data_) {
			switch (ch) {
			case '"': { result += "&quot;"; break; }
			case '<': { result += "&lt;"; break; }
			case '>': { result += "&gt;"; break; }
			case '\'': { result += "&apos;"; break; }
			case '&': { result += "&amp;"; break; }
			default: { result += ch; break; }
			}
		}
		if (!font_family_.empty() && !font_weight_.empty()) {
			out << "<text";
			RenderAttrs(out);
			out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x;
			out << "\" dy=\"" << offset_.y << "\" font-size=\"" << size_ << "\" font-family=\"" << font_family_;
			out << "\" font-weight=\"" << font_weight_;
			out << "\">" << result << "</text>";
		}
		else if (!font_family_.empty() && font_weight_.empty()) {
			out << "<text";
			RenderAttrs(out);
			out<< " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x;
			out << "\" dy=\"" << offset_.y << "\" font-size=\"" << size_ << "\" font-family=\"" << font_family_;
			out << "\">" << result << "</text>";
		}
		else if (font_family_.empty() && !font_weight_.empty()) {
			out << "<text";
			RenderAttrs(out);
			out<< " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x;
			out << "\" dy=\"" << offset_.y << "\" font-size=\"" << size_;
			out << "\" font-weight=\"" << font_weight_;
			out << "\">" << result << "</text>";
		}
		else {
			out << "<text";
			RenderAttrs(out);
			out<< " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x;
			out << "\" dy=\"" << offset_.y << "\" font-size=\"" << size_;
			out << "\">" << result << "</text>";
		}

	}

	Text& Text::SetPosition(Point pos) {
		pos_ = pos;
		return *this;
	}

	Text& Text::SetOffset(Point offset) {
		offset_ = offset;
		return *this;
	}

	Text& Text::SetFontSize(uint32_t size) {
		size_ = size;
		return *this;
	}

	Text& Text::SetFontFamily(std::string font_family) {
		font_family_ = font_family;
		return *this;
	}

	Text& Text::SetFontWeight(std::string font_weight) {
		font_weight_ = font_weight;
		return *this;
	}

	Text& Text::SetData(std::string data) {
		data_ = data;
		return *this;
	}

	// ---------- Document ------------------

	void Document::AddPtr(std::unique_ptr<Object>&& obj) {
		objects.emplace_back(std::move(obj));
	}

	void Document::Render(std::ostream& out) const {
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
		for (auto& i : objects) {
			i->Render({ out, 2, 2 });
		}
		out << "</svg>"sv;
	}

	std::ostream& operator<<(std::ostream& out, const StrokeLineCap& elem) {
		switch (elem) {
		case StrokeLineCap::BUTT: { out << "butt"; break; }
		case StrokeLineCap::ROUND: { out << "round"; break; }
		case StrokeLineCap::SQUARE: { out << "square"; break; }
		default: break;
		}
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& elem) {
		switch (elem) {
		case StrokeLineJoin::ARCS: { out << "arcs"; break; }
		case StrokeLineJoin::BEVEL: { out << "bevel"; break; }
		case StrokeLineJoin::MITER: { out << "miter"; break; }
		case StrokeLineJoin::MITER_CLIP: { out << "miter-clip"; break; }
		case StrokeLineJoin::ROUND: { out << "round"; break; }
		default: break;
		}
		return out;
	}

}  // namespace svg
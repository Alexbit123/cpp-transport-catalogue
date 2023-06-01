#include "json.h"

using namespace std;

namespace json {

	namespace {

		Node LoadNode(istream& input);

		Node LoadArray(istream& input) {
			Array result;
			char c;

			for (; input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}

			if (result.empty() && c == ']') {
				return(Array{});
			}
			else if (result.empty()) {
				throw ParsingError(""s);
			}
			
			return Node(move(result));
		}

		Number LoadNumber(std::istream& input) {
			using namespace std::literals;

			std::string parsed_num;

			// Считывает в parsed_num очередной символ из input
			auto read_char = [&parsed_num, &input] {
				parsed_num += static_cast<char>(input.get());
				if (!input) {
					throw ParsingError("Failed to read number from stream"s);
				}
			};

			// Считывает одну или более цифр в parsed_num из input
			auto read_digits = [&input, read_char] {
				if (!std::isdigit(input.peek())) {
					throw ParsingError("A digit is expected"s);
				}
				while (std::isdigit(input.peek())) {
					read_char();
				}
			};

			if (input.peek() == '-') {
				read_char();
			}
			// Парсим целую часть числа
			if (input.peek() == '0') {
				read_char();
				// После 0 в JSON не могут идти другие цифры
			}
			else {
				read_digits();
			}

			bool is_int = true;
			// Парсим дробную часть числа
			if (input.peek() == '.') {
				read_char();
				read_digits();
				is_int = false;
			}

			// Парсим экспоненциальную часть числа
			if (int ch = input.peek(); ch == 'e' || ch == 'E') {
				read_char();
				if (ch = input.peek(); ch == '+' || ch == '-') {
					read_char();
				}
				read_digits();
				is_int = false;
			}

			try {
				if (is_int) {
					// Сначала пробуем преобразовать строку в int
					try {
						return std::stoi(parsed_num);
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return std::stod(parsed_num);
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		std::string LoadString(std::istream& input) {
			using namespace std::literals;

			auto it = std::istreambuf_iterator<char>(input);
			auto end = std::istreambuf_iterator<char>();
			std::string s;
			while (true) {
				if (it == end) {
					// Поток закончился до того, как встретили закрывающую кавычку?
					throw ParsingError("String parsing error");
				}
				const char ch = *it;
				if (ch == '"') {
					// Встретили закрывающую кавычку
					++it;
					break;
				}
				else if (ch == '\\') {
					// Встретили начало escape-последовательности
					++it;
					if (it == end) {
						// Поток завершился сразу после символа обратной косой черты
						throw ParsingError("String parsing error");
					}
					const char escaped_char = *(it);
					// Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
					switch (escaped_char) {
					case 'n':
						s.push_back('\n');
						break;
					case 't':
						s.push_back('\t');
						break;
					case 'r':
						s.push_back('\r');
						break;
					case '"':
						s.push_back('"');
						break;
					case '\\':
						s.push_back('\\');
						break;
					default:
						// Встретили неизвестную escape-последовательность
						throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
					}
				}
				else if (ch == '\n' || ch == '\r') {
					// Строковый литерал внутри- JSON не может прерываться символами \r или \n
					throw ParsingError("Unexpected end of line"s);
				}
				else {
					// Просто считываем очередной символ и помещаем его в результирующую строку
					s.push_back(ch);
				}
				++it;
			}

			return s;
		}

		Node LoadDict(istream& input) {
			Dict result;
			char c;

			for (; input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}

				string key = LoadString(input);
				input >> c;
				result.insert({ move(key), LoadNode(input) });
			}
			
			if (result.empty() && c == '}') {
				return (Dict{});
			}
			else if (result.empty()) {
				throw ParsingError(""s);
			}

			return Node(move(result));
		}

		Node LoadNull(istream& input) {
			string s = "n";
			nullptr_t null_ = nullptr;
			for (char c; input >> c && c != '\0' && c != ',';) {
				switch (c) {
				case '\n':
					s.push_back('\n');
					break;
				case '\t':
					s.push_back('\t');
					break;
				case '\r':
					s.push_back('\r');
					break;
				default:
					s += c;
				}
			}
			if (s == "null") {
				return Node{ move(null_) };
			}
			else {
				throw ParsingError(""s);
			}
			
		}

		Node LoadBool(istream& input) {
			string s = "";
			for (char c; input >> c && c != '\0' && c != ',' && c != 'e';) {
				switch (c) {
				case '\n':
					s.push_back('\n');
					break;
				case '\t':
					s.push_back('\t');
					break;
				case '\r':
					s.push_back('\r');
					break;
				default:
					s += c;
				}
			}
			if (s == "ru") {
				return Node{ move(true) };
			}
			else if (s == "als") {
				return Node{ move(false) };
			}
			else {
				throw ParsingError(""s);
			}

		}

		Node LoadNode(istream& input) {
			char c;
			input >> c;

			if (c == '[') {
				return LoadArray(input);
			}
			else if (c == '{') {
				return LoadDict(input);
			}
			else if (c == '"') {
				return LoadString(input);
			}
			else if (c == 'n') {
				return LoadNull(input);
			}
			else if (c == 't' || c == 'f') {
				return LoadBool(input);
			}
			else {
				input.putback(c);
				auto variant_ = LoadNumber(input);
				if (holds_alternative<int>(variant_)) {
					return get<int>(variant_);
				}
				else {
					return get<double>(variant_);
				}
			}
		}

	}  // namespace

	/*Node::Node(nullptr_t value)
		: value_(value) {
	}

	Node::Node(Array array)
		: value_(move(array)) {
	}

	Node::Node(Dict map)
		: value_(move(map)) {
	}

	Node::Node(bool value)
		: value_(value) {
	}

	Node::Node(int value)
		: value_(value) {
	}

	Node::Node(double value)
		: value_(value) {
	}

	Node::Node(string value)
		: value_(value) {
	}*/

	const Array& Node::AsArray() const {
		if (IsArray()) {
			return get<Array>(*this);
		}
		else {
			throw logic_error("");
		}
	}

	const Dict& Node::AsMap() const {
		if (IsMap()) {
			return get<Dict>(*this);
		}
		else {
			throw logic_error("");
		}
	}

	bool Node::AsBool() const {
		if (IsBool()) {
			return get<bool>(*this);
		}
		else {
			throw logic_error("");
		}
	}

	int Node::AsInt() const {
		if (IsInt()) {
			return get<int>(*this);
		}
		else {
			throw logic_error("");
		}
	}

	double Node::AsDouble() const {
		if (IsInt()) {
			return static_cast<double>(get<int>(*this));
		}
		if (IsDouble()) {
			return get<double>(*this);
		}
		else {
			throw logic_error("");
		}
	}

	const string& Node::AsString() const {
		if (IsString()) {
			return get<string>(*this);
		}
		else {
			throw logic_error("");
		}
	}

	bool Node::IsNull() const {
		return holds_alternative<nullptr_t>(*this) ? true : false;
	}

	bool Node::IsArray() const {
		return holds_alternative<Array>(*this) ? true : false;
	}

	bool Node::IsMap() const {
		return holds_alternative<Dict>(*this) ? true : false;
	}

	bool Node::IsBool() const {
		return holds_alternative<bool>(*this) ? true : false;
	}

	bool Node::IsInt() const {
		return holds_alternative<int>(*this) ? true : false;
	}

	bool Node::IsDouble() const {
		return holds_alternative<int>(*this) ||
			holds_alternative<double>(*this) ? true : false;
	}

	bool Node::IsPureDouble() const {
		return holds_alternative<double>(*this) ? true : false;
	}

	bool Node::IsString() const {
		return holds_alternative<string>(*this) ? true : false;
	}

	Document::Document(Node root)
		: root_(move(root)) {
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	Document Load(istream& input) {
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, std::ostream& output) {
		PrintNode(doc.GetRoot(), output);
	}

	void PrintNode(const Node& node, std::ostream& out) {
		std::visit(PrintValue{out}, node.GetValue());
	}

	bool operator==(const Node& node1, const Node& node2) {
		return node1.GetValue() == node2.GetValue() ? true : false;
	}

	bool operator!=(const Node& node1, const Node& node2) {
		return !(node1 == node2);
	}

	bool operator==(const Document& doc1, const Document& doc2) {
		return doc1.GetRoot() == doc2.GetRoot() ? true : false;
	}

	bool operator!=(const Document& doc1, const Document& doc2) {
		return !(doc1 == doc2);
	}

	void PrintValue::operator()(std::nullptr_t) const {
		out << "null";
	}

	void PrintValue::operator()(const bool value) const {
		value ? out << "true" : out << "false";
	}

	void PrintValue::operator()(const Array& value) const {
		bool is_first = true;
		out << "[";
		for (auto& elem : value) {
			if (!is_first) {
				out << ", ";
				PrintNode(elem, out);
			}
			else {
				is_first = false;
				PrintNode(elem, out);
			}

		}
		out << "]";
	}

	void PrintValue::operator()(const Dict& value) const {
		bool is_first = true;
		out << "{ ";
		for (auto& [str, elem] : value) {
			if (!is_first) {
				out << ", \"" << str << "\": ";
				PrintNode(elem, out);
			}
			else {
				is_first = false;
				out << "\"" << str << "\": ";
				PrintNode(elem, out);
			}
		}
		out << " }";
	}

	void PrintValue::operator()(const std::string& value) const {
		string s = "";
		s += "\"";
		for (char ch : value) {
			switch (ch) {
			case '\n':
				s += "\\n";
				break;
			case '\r':
				s += "\\r";
				break;
			case '\"':
				s += "\\\"";
				break;
			case '\\':
				s += "\\\\";
				break;
			default:
				s.push_back(ch);
			}
		}
		s += "\"";
		out << s;
	}

	void PrintValue::operator()(const int value) const {
		out << value;
	}

	void PrintValue::operator()(const double value) const {
		out << value;
	}

}  // namespace json
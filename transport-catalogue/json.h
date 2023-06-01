#pragma once

#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <optional>

namespace json {
	using namespace std::string_literals;

	class Node;

	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;
	using Number = std::variant<int, double>;
	using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

	// Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node : private Value {
	public:
		using variant::variant;

		const Array& AsArray() const;
		const Dict& AsMap() const;
		bool AsBool() const;
		int AsInt() const;
		double AsDouble() const;
		const std::string& AsString() const;

		const Value& GetValue() const { return *this; }

		bool IsNull() const;
		bool IsArray() const;
		bool IsMap() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsDouble() const; //¬озвращает true, если в Node хранитс€ int либо double.
		bool IsPureDouble() const; //¬озвращает true, если в Node хранитс€ double.
		bool IsString() const;
	};

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

	private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

	//  онтекст вывода, хранит ссылку на поток вывода и текущий отсуп
	struct PrintContext {
		std::ostream& out;
		int indent_step = 4;
		int indent = 0;

		void PrintIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		// ¬озвращает новый контекст вывода с увеличенным смещением
		PrintContext Indented() const {
			return { out, indent_step, indent_step + indent };
		}
	};

	struct PrintValue {
		std::ostream& out;

		void operator()(std::nullptr_t) const;
		void operator()(const bool value) const;
		void operator()(const Array& value) const;
		void operator()(const Dict& value) const;
		void operator()(const std::string& value) const;
		void operator()(const int value) const;
		void operator()(const double value) const;
	};

	void PrintNode(const Node& node, std::ostream& out);

	bool operator==(const Node& node1, const Node& node2);
	bool operator!=(const Node& node1, const Node& node2);

	bool operator==(const Document& doc1, const Document& doc2);
	bool operator!=(const Document& doc1, const Document& doc2);

}  // namespace json
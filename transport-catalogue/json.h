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

	// ��� ������ ������ ������������� ��� ������� �������� JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node {
	public:
		using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

		Node() = default;
		Node(nullptr_t value);
		Node(Array array);
		Node(Dict map);
		Node(bool value);
		Node(int value);
		Node(double value);
		Node(std::string value);

		const Array& AsArray() const;
		const Dict& AsMap() const;
		bool AsBool() const;
		int AsInt() const;
		double AsDouble() const;
		const std::string& AsString() const;

		const Value& GetValue() const { return value_; }

		bool IsNull() const;
		bool IsArray() const;
		bool IsMap() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsDouble() const; //���������� true, ���� � Node �������� int ���� double.
		bool IsPureDouble() const; //���������� true, ���� � Node �������� double.
		bool IsString() const;

	private:
		Value value_;
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

	// �������� ������, ������ ������ �� ����� ������ � ������� �����
	struct PrintContext {
		std::ostream& out;
		int indent_step = 4;
		int indent = 0;

		void PrintIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		// ���������� ����� �������� ������ � ����������� ���������
		PrintContext Indented() const {
			return { out, indent_step, indent_step + indent };
		}
	};

	void PrintNode(const Node& node, std::ostream& out);

	// ���������� ������� PrintValue ��� ������ �������� null
	void PrintValue(std::nullptr_t, std::ostream& out);

	// ���������� ������� PrintValue ��� ������ �������� bool
	void PrintValue(const bool value, std::ostream& out);

	// ���������� ������� PrintValue ��� ������ �������� Array
	void PrintValue(const Array& value, std::ostream& out);

	// ���������� ������� PrintValue ��� ������ �������� Dict
	void PrintValue(const Dict& value, std::ostream& out);

	// ���������� ������� PrintValue ��� ������ �������� string
	void PrintValue(const std::string& value, std::ostream& out);
	
	// ���������� ������� PrintValue ��� ������ �������� int
	void PrintValue(const int value, std::ostream& out);

	// ���������� ������� PrintValue ��� ������ �������� double
	void PrintValue(const double value, std::ostream& out);

	bool operator==(const Node& node1, const Node& node2);
	bool operator!=(const Node& node1, const Node& node2);

	bool operator==(const Document& doc1, const Document& doc2);
	bool operator!=(const Document& doc1, const Document& doc2);

}  // namespace json
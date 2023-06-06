#include "json.h"
namespace json {
	using namespace std;

	class BaseContext;
	class DictValueContext;
	class DictItemContext;
	class ArrayItemContext;

	class Builder {
	public:
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		DictValueContext Key(std::string key);
		BaseContext Value(Node::Value elem);
		Builder& EndDict();
		Builder& EndArray();
		Node Build();

	private:
		Node root_;
		Node last_element;
		std::vector<Node*> nodes_stack_;
	};

	class BaseContext {
	public:
		BaseContext(Builder& builder);

		DictValueContext Key(std::string value);

		BaseContext Value(Node::Value value);

		DictItemContext StartDict();

		ArrayItemContext StartArray();

		Builder& EndDict();

		Builder& EndArray();

		Node Build();

	protected:
		Builder& builder_;
	};

	class DictValueContext : public BaseContext {
	public:
		using BaseContext::BaseContext;
		using BaseContext::StartDict;
		using BaseContext::StartArray;

		DictItemContext Value(Node::Value value);
		DictValueContext Key(std::string value) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	};

	class DictItemContext : public BaseContext {
	public:
		using BaseContext::BaseContext;
		using BaseContext::Key;
		using BaseContext::EndDict;

		BaseContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	};

	class ArrayItemContext : public BaseContext {
	public:
		using BaseContext::BaseContext;
		using BaseContext::StartDict;
		using BaseContext::StartArray;
		using BaseContext::EndArray;

		ArrayItemContext Value(Node::Value value);
		DictValueContext Key(std::string value) = delete;
		Builder& EndDict() = delete;
		Node Build() = delete;
	};
}
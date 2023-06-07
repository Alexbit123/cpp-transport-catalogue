#include "json_builder.h"
namespace json {
	Builder::DictItemContext Builder::StartDict() {
		Dict result;
		Node* buf = new Node(result);
		AddContainer(buf);
		return *this;
	}

	Builder::ArrayItemContext Builder::StartArray() {
		Array result;
		Node* buf = new Node(result);
		AddContainer(buf);
		return *this;
	}

	Builder::DictValueContext Builder::Key(std::string key) {
		if (nodes_stack_.empty()) {
			throw logic_error("Error add Key");
		}
		else if (!nodes_stack_.back()->IsDict()) {
			throw logic_error("Error add Key");
		}
		else if (last_element.IsString()) {
			if (nodes_stack_.back()->AsDict().count(last_element.AsString()) != 0) {
				throw logic_error("Error add Key");
			}
		}
		Node* buf = new Node(std::move(key));
		nodes_stack_.back()->AsDict()[buf->AsString()];
		last_element = *buf;
		return *this;
	}

	Builder::BaseContext Builder::Value(Node::Value elem) {
		if (nodes_stack_.empty() && root_.IsNull()) {
			root_.GetValue() = elem;
		}
		else if (nodes_stack_.empty() && !root_.IsNull()) {
			throw logic_error("Error add Value");
		}
		else if (nodes_stack_.back()->IsDict() && last_element.IsString()) {
			nodes_stack_.back()->AsDict().at(last_element.AsString()).GetValue() = elem;
		}
		else if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().emplace_back(elem);
		}
		else {
			throw logic_error("Error add Value");
		}

		last_element.GetValue() = elem;
		return *this;
	}

	Builder& Builder::EndDict() {
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
			throw logic_error("Last element is not an Dict");
		}
		nodes_stack_.pop_back();
		return *this;
	}

	Builder& Builder::EndArray() {
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
			throw logic_error("Last element is not an Array");
		}
		nodes_stack_.pop_back();
		return *this;
	}

	Node Builder::Build() {
		if (!nodes_stack_.empty()) {
			throw logic_error("Error Build");
		}
		else if (nodes_stack_.empty() && root_.IsNull() && last_element.IsNull()) {
			throw logic_error("Error Build");
		}
		return root_;
	}

	void Builder::AddContainer(Node* buf) {
		if (nodes_stack_.empty()) {
			root_ = *buf;
			nodes_stack_.push_back(&root_);
		}
		else if (nodes_stack_.back()->IsDict() && last_element.IsString()) {
			nodes_stack_.back()->AsDict().at(last_element.AsString()) = *buf;
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(last_element.AsString()));
		}
		else if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().push_back(*buf);
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else {
			throw logic_error("Error add container");
		}
		last_element = *buf;
	}

	Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {}

	Builder::DictValueContext Builder::BaseContext::Key(std::string value) {
		return builder_.Key(value);
	}

	Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {
		return builder_.Value(value);
	}

	Builder::DictItemContext Builder::BaseContext::StartDict() {
		return builder_.StartDict();
	}

	Builder::ArrayItemContext Builder::BaseContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& Builder::BaseContext::EndDict() {
		return builder_.EndDict();
	}

	Builder& Builder::BaseContext::EndArray() {
		return builder_.EndArray();
	}

	Node Builder::BaseContext::Build() {
		return builder_.Build();
	}

	Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value) {
		BaseContext temp = builder_.Value(value);
		return static_cast<DictItemContext&>(temp);
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
		BaseContext temp = builder_.Value(value);
		return static_cast<ArrayItemContext&>(temp);
	}
}//json
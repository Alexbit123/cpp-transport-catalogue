#include "json_builder.h"
namespace json {
	DictItemContext Builder::StartDict() {
		Dict result;
		Node* buf = new Node(result);
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
			throw logic_error("Error add Dict");
		}

		last_element = *buf;
		return *this;
	}
	ArrayItemContext Builder::StartArray() {
		Array result;
		Node* buf = new Node(result);
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
			throw logic_error("Error add Array");
		}

		last_element = *buf;
		return *this;
	}
	DictValueContext Builder::Key(std::string key) {
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
	BaseContext Builder::Value(Node::Value elem) {
		Node* buf;
		if (std::holds_alternative<int>(elem)) {
			buf = new Node(std::get<int>(elem));
		}
		else if (std::holds_alternative<double>(elem)) {
			buf = new Node(std::get<double>(elem));
		}
		else if (std::holds_alternative<std::string>(elem)) {
			buf = new Node(std::get<std::string>(std::move(elem)));
		}
		else if (std::holds_alternative<Dict>(elem)) {
			buf = new Node(std::get<Dict>(std::move(elem)));
		}
		else if (std::holds_alternative<Array>(elem)) {
			buf = new Node(std::get<Array>(std::move(elem)));
		}
		else if (std::holds_alternative<bool>(elem)) {
			buf = new Node(std::get<bool>(elem));
		}
		else if (std::holds_alternative<double>(elem)) {
			buf = new Node(std::get<double>(elem));
		}
		else {
			buf = new Node(std::get<nullptr_t>(elem));
		}
		if (nodes_stack_.empty() && root_.IsNull()) {
			root_ = *buf;
		}
		else if (nodes_stack_.empty() && !root_.IsNull()) {
			throw logic_error("Error add Value");
		}
		else if (nodes_stack_.back()->IsDict() && last_element.IsString()) {
			nodes_stack_.back()->AsDict().at(last_element.AsString()) = *buf;
		}
		else if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().push_back(*buf);
		}
		else {
			throw logic_error("Error add Value");
		}

		last_element = *buf;
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

	BaseContext::BaseContext(Builder& builder) : builder_(builder) {}
	DictValueContext BaseContext::Key(std::string value) {
		return builder_.Key(value);
	}
	BaseContext BaseContext::Value(Node::Value value) {
		return builder_.Value(value);
	}
	DictItemContext BaseContext::StartDict() {
		return builder_.StartDict();
	}
	ArrayItemContext BaseContext::StartArray() {
		return builder_.StartArray();
	}
	Builder& BaseContext::EndDict() {
		return builder_.EndDict();
	}
	Builder& BaseContext::EndArray() {
		return builder_.EndArray();
	}
	Node BaseContext::Build() {
		return builder_.Build();
	}
	DictItemContext DictValueContext::Value(Node::Value value) {
		BaseContext temp = builder_.Value(value);
		return static_cast<DictItemContext&>(temp);
	}
	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		BaseContext temp = builder_.Value(value);
		return static_cast<ArrayItemContext&>(temp);
	}
}//json
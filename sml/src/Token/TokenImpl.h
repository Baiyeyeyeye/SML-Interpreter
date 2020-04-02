#pragma once

#include <memory>
#include <utility>
#include "Token.h"

using namespace std;

struct Token::Impl {
	Impl(int row, int column, int type, string value);
	
	template<typename StringIterator>
	[[nodiscard]] char handleEscape(StringIterator &&it) const;
	
	template<typename PlainValueT>
	const PlainValueT &plain();
	
	void initReal();
	
	void initBool();
	
	void initInt();
	
	void initChar();
	
	void initString();
	
	const int row;
	const int column;
	const int type;
	const std::string value;
	shared_ptr<void> plainValue;
};

Token::Impl::Impl(int row, int column, int type, string value)
	: row(row), column(column), type(type), value(std::move(value)) {
	
}

template<typename StringIterator>
char Token::Impl::handleEscape(StringIterator &&it) const {
	++it;
	char tmp{};
	if (isdigit(*it)) {
		int number{};
		for (int i = 0; i < 3; ++i) {
			number *= 10;
			number += *it++ - '0';
		}
		tmp = static_cast<char>(number);
		return tmp;
	}
	switch (*it++) {
	case 'a':
		tmp = '\a';
		break;
	case 'b':
		tmp = '\b';
		break;
	case 'n':
		tmp = '\n';
		break;
	case 'r':
		tmp = '\r';
		break;
	case 't':
		tmp = '\t';
		break;
	case 'v':
		tmp = '\v';
		break;
	case '"':
		tmp = '"';
		break;
	case '\\':
		tmp = '\\';
		break;
	}
	return tmp;
}

void Token::Impl::initReal() {
	double res;
	if (auto pos = value.find('~'); pos != string::npos) {
		auto tmp = value;
		tmp[pos] = '-';
		res = stod(tmp);
	} else {
		res = stod(value);
	}
	plainValue = make_shared<double>(res);
}

void Token::Impl::initBool() {
	bool res = value == "true";
	plainValue = make_shared<bool>(res);
}

void Token::Impl::initInt() {
	const string &str = value;
	int res;
	if (str.size() > 2 && str[0] == '0' && tolower(str[1]) == 'x') {
		res = stoi(str, nullptr, 16);
	} else {
		res = stoi(str);
	}
	plainValue = make_shared<int>(res);
}

void Token::Impl::initChar() {
	char c;
	auto tmp = value.substr(2, value.size() - 3);
	if (tmp.size() == 1) {
		c = tmp[0];
	} else {
		c = handleEscape(tmp.begin());
	}
	plainValue = make_shared<char>(c);
}

void Token::Impl::initString() {
	string tmp;
	tmp.reserve(value.size());
	for (auto it = value.begin() + 1; it < value.end() - 1;) {
		if (*it == '\\') {
			tmp += handleEscape(it);
			continue;
		} else {
			tmp += *it++;
		}
	}
	tmp.shrink_to_fit();
	plainValue = make_shared<string>(tmp);
}

template<typename PlainValueT>
const PlainValueT &Token::Impl::plain() {
	return *static_pointer_cast<PlainValueT>(plainValue).get();
}

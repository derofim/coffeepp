#include "file.hpp"

#include "line.hpp"
#include "IndentionType.hpp"

#include <fstream>
#include <iostream>

File::File(std::istream& in, const std::string& name) : indent(0), name(name) {
	bool waitForEndComment = false;
	bool wasBeginScope = false;
	while (true) {
		std::string buf;
		std::getline(in, buf);
		if (!in) {
			break;
		}
		auto line = std::make_shared<Line>(buf, waitForEndComment, wasBeginScope, indentionType);
		wasBeginScope = line->isBeginScope();
		while (!waitForEndComment && line->getIndent() > indent) {
			++indent;
		}
		if (line->isBeginComment() || waitForEndComment) {
			waitForEndComment = true;
		} else {
			closeScopes(line->getIndent());
		}
		lines.push_back(line);
	}
	closeScopes(0);
}

std::string File::getNewBuf() const {
	std::string buf;
	auto pos = name.find_last_of(".");
	if (pos != std::string::npos) {
		buf += "#include \"";
		buf += name.substr(0, pos);
		buf += ".hpp\"\n\n";
	}
	for (const auto& line : lines) {
		buf += line->getNewBuf();
	}
	return buf;
}

std::string File::getHeaderBuf() const {
	std::string buf = "#pragma once\n\n";
	for (const auto& line : lines) {
		buf += line->getHeaderBuf();
	}
	return buf;
}

void File::closeScopes(int newIndent) {
	while (newIndent < indent) {
		--indent;
		std::string closingBrace;
		if (indentionType) {
			for (size_t i = 0; i < indent * indentionType->number; ++i) {
				closingBrace += indentionType->tab ? '\t' : ' ';
			}
		}
		closingBrace += '}';
		lines.push_back(std::make_shared<Line>(closingBrace, false, false, indentionType));
	}
}

#include <string>
#include <sstream>

#include "errors.h"
#include "guide.h"

std::string BaseException::traceback() const
{
	// we need the starting index of the line to display the full line
	int lnStart = this->position_.start - (this->position_.column - 1);
	// reveals the amount of correct tokens before error actually starts
	int errStart = this->position_.start - lnStart;
	
	std::string gutter = std::to_string(this->position_.line) + " | ";
	
	return "Exception in " + Source::uri + ":\n" +
		gutter + Source::text.substr(lnStart, this->position_.end) + '\n' +
		std::string(gutter.size(), ' ') + std::string(errStart - lnStart, '~') +
		std::string((long)this->position_.end - errStart, '^') + '\n' +
		this->brief_;
};

std::ostream& operator<<(std::ostream& os, const BaseException& n)
{
	os << n.traceback();
	return os;
}
#include "cantera/base/ctexceptions.h"

#include "application.h"
#include "cantera/base/global.h"
#include "cantera/base/stringUtils.h"

#include <sstream>
#include <typeinfo>

namespace Cantera {

// *** Exceptions ***

static const char* stars = "***********************************************************************\n";

CanteraError::CanteraError(std::string procedure, std::string msg) :
    msg_(msg),
    procedure_(procedure),
    saved_(false)
{
}

CanteraError::CanteraError(std::string procedure) :
    procedure_(procedure),
    saved_(false)
{
}

void CanteraError::save()
{
    if (!saved_) {
        Application::Instance()->addError(procedure_, getMessage());
        saved_ = true;
    }
}

const char* CanteraError::what() const throw() {
    try {
        formattedMessage_ = stars;
        formattedMessage_ += getClass() + " thrown by " + procedure_ + ":\n" + getMessage();
        if (formattedMessage_.compare(formattedMessage_.size()-1, 1, "\n")) {
            formattedMessage_.append("\n");
        }
        formattedMessage_ += stars;
    } catch (...) {
        // Something went terribly wrong and we couldn't even format the message.
    }
    return formattedMessage_.c_str();
    }

std::string CanteraError::getMessage() const {
    return msg_;
}

std::string ArraySizeError::getMessage() const {
    std::stringstream ss;
    ss << "Array size (" << sz_ << ") too small. Must be at least " << reqd_ << ".";
    return ss.str();
}

std::string ElementRangeError::getMessage() const {
    std::stringstream ss;
    ss << "Element index " << m_ << " outside valid range of 0 to " << (mmax_-1) << ".";
    return ss.str();
}

// *** Warnings ***

void deprecatedMethod(std::string classnm, std::string oldnm, std::string newnm)
{
    writelog(">>>> WARNING: method "+oldnm+" of class "+classnm
             +" is deprecated.\n");
    writelog("         Use method "+newnm+" instead.\n");
    writelog("         (If you want to rescue this method from deprecated\n");
    writelog("         status, see http://www.cantera.org/deprecated.html)");
}

void removeAtVersion(std::string func, std::string version)
{
    writelog("Removed procedure: "+func+"\n");
    writelog("Removed in version: "+version+"\n");
    throw CanteraError("removeAtVersion: "+ func,"procedure has been removed.");
}

} // namespace Cantera

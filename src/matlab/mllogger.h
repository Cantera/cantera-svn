/**
 * @file mlloger.h
 */
#ifndef MLLOGGER_H
#define MLLOGGER_H

#include "ctmatutils.h"
#include "cantera/base/logger.h"

namespace Cantera
{

class ML_Logger : public Logger
{
public:
    ML_Logger() {}
    virtual ~ML_Logger() {}

    virtual void write(const std::string& s) {
        mexPrintf("%s", s.c_str());
    }

    virtual void writeendl(const std::string& msg) {
        mexPrintf("\n");
    }

    virtual void error(const std::string& msg) {
        mexErrMsgTxt(msg.c_str());
    }

    DEPRECATED(virtual int env()) {
        return 1;
    }
};

}

#endif

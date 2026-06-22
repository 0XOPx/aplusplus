// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef STANDARD_LIBRARY_HPP
#define STANDARD_LIBRARY_HPP

#include "interpreter.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <ctime>

namespace StdLib {

inline void registerAll(std::shared_ptr<Environment>& env) {
    // Math
    env->set("sqrt",  AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        double v = a.empty() ? 0 : (a[0]->kind==AppValue::Kind::FLOAT ? a[0]->fval : (double)a[0]->ival);
        return AppValue::makeFloat(std::sqrt(v));
    }));
    env->set("abs",   AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty()) return AppValue::makeInt(0);
        if (a[0]->kind==AppValue::Kind::FLOAT) return AppValue::makeFloat(std::fabs(a[0]->fval));
        return AppValue::makeInt(std::abs(a[0]->ival));
    }));
    env->set("pow",   AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.size() < 2) return AppValue::makeInt(0);
        double base = a[0]->kind==AppValue::Kind::FLOAT ? a[0]->fval : (double)a[0]->ival;
        double exp  = a[1]->kind==AppValue::Kind::FLOAT ? a[1]->fval : (double)a[1]->ival;
        return AppValue::makeFloat(std::pow(base, exp));
    }));
    env->set("floor", AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty()) return AppValue::makeInt(0);
        double v = a[0]->kind==AppValue::Kind::FLOAT ? a[0]->fval : (double)a[0]->ival;
        return AppValue::makeInt((long long)std::floor(v));
    }));
    env->set("ceil",  AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty()) return AppValue::makeInt(0);
        double v = a[0]->kind==AppValue::Kind::FLOAT ? a[0]->fval : (double)a[0]->ival;
        return AppValue::makeInt((long long)std::ceil(v));
    }));
    env->set("round", AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty()) return AppValue::makeInt(0);
        double v = a[0]->kind==AppValue::Kind::FLOAT ? a[0]->fval : (double)a[0]->ival;
        return AppValue::makeInt((long long)std::round(v));
    }));
    env->set("max",   AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.size() < 2) return a.empty() ? AppValue::makeNil() : a[0];
        double l = a[0]->kind==AppValue::Kind::FLOAT ? a[0]->fval : (double)a[0]->ival;
        double r = a[1]->kind==AppValue::Kind::FLOAT ? a[1]->fval : (double)a[1]->ival;
        return l >= r ? a[0] : a[1];
    }));
    env->set("min",   AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.size() < 2) return a.empty() ? AppValue::makeNil() : a[0];
        double l = a[0]->kind==AppValue::Kind::FLOAT ? a[0]->fval : (double)a[0]->ival;
        double r = a[1]->kind==AppValue::Kind::FLOAT ? a[1]->fval : (double)a[1]->ival;
        return l <= r ? a[0] : a[1];
    }));

    // String
    env->set("upper", AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty() || a[0]->kind!=AppValue::Kind::STRING) return AppValue::makeString("");
        std::string s = a[0]->sval;
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return AppValue::makeString(s);
    }));
    env->set("lower", AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty() || a[0]->kind!=AppValue::Kind::STRING) return AppValue::makeString("");
        std::string s = a[0]->sval;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return AppValue::makeString(s);
    }));
    env->set("split", AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.size() < 2 || a[0]->kind!=AppValue::Kind::STRING) return AppValue::makeList({});
        char delim = a[1]->sval.empty() ? ' ' : a[1]->sval[0];
        std::vector<AppValuePtr> parts;
        std::stringstream ss(a[0]->sval);
        std::string token;
        while (std::getline(ss, token, delim)) parts.push_back(AppValue::makeString(token));
        return AppValue::makeList(parts);
    }));
    env->set("trim",  AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty() || a[0]->kind!=AppValue::Kind::STRING) return AppValue::makeString("");
        std::string s = a[0]->sval;
        size_t l = s.find_first_not_of(" \t\r\n");
        size_t r = s.find_last_not_of(" \t\r\n");
        return AppValue::makeString(l==std::string::npos ? "" : s.substr(l, r-l+1));
    }));

    // List
    env->set("push",  AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.size() < 2 || a[0]->kind!=AppValue::Kind::LIST) return AppValue::makeNil();
        a[0]->list.push_back(a[1]);
        return a[0];
    }));
    env->set("pop",   AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty() || a[0]->kind!=AppValue::Kind::LIST || a[0]->list.empty())
            return AppValue::makeNil();
        auto v = a[0]->list.back();
        a[0]->list.pop_back();
        return v;
    }));
    env->set("range", AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty()) return AppValue::makeList({});
        long long start = 0, end_ = 0, step = 1;
        if (a.size() == 1) { end_ = a[0]->ival; }
        else { start = a[0]->ival; end_ = a[1]->ival; }
        if (a.size() >= 3) step = a[2]->ival;
        if (step == 0) return AppValue::makeList({});
        std::vector<AppValuePtr> items;
        for (long long i = start; step > 0 ? i < end_ : i > end_; i += step)
            items.push_back(AppValue::makeInt(i));
        return AppValue::makeList(items);
    }));

    // Type checks
    env->set("type",  AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty()) return AppValue::makeString("null");
        return AppValue::makeString(a[0]->typeName);
    }));
    env->set("bool",  AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty()) return AppValue::makeBool(false);
        return AppValue::makeBool(a[0]->isTruthy());
    }));
    env->set("float", AppValue::makeNative([](std::vector<AppValuePtr> a) -> AppValuePtr {
        if (a.empty()) return AppValue::makeFloat(0.0);
        if (a[0]->kind==AppValue::Kind::FLOAT) return a[0];
        if (a[0]->kind==AppValue::Kind::INT) return AppValue::makeFloat((double)a[0]->ival);
        if (a[0]->kind==AppValue::Kind::STRING) return AppValue::makeFloat(std::stod(a[0]->sval));
        return AppValue::makeFloat(0.0);
    }));
}

} // namespace StdLib

#endif // STANDARD_LIBRARY_HPP

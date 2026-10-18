// Utils.h - date helpers (ISO yyyy-mm-dd <-> day numbers), rounding, JSON alias.
#pragma once
#include <string>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
#include "json.hpp"

using json = nlohmann::json;

// Error that carries an HTTP status so the API layer can answer properly.
struct ApiError : public std::runtime_error {
    int status;
    ApiError(int s, const std::string& m) : std::runtime_error(m), status(s) {}
};

namespace util {

// "No expiry" (non-perishable goods) is stored as a very large day number.
const long NO_EXPIRY = 99999999L;

inline long daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (long)doe - 719468;
}

inline void civilFromDays(long z, int& y, int& m, int& d) {
    z += 719468;
    long era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = (unsigned)(z - era * 146097);
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = (int)(yoe + era * 400);
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned mp = (5 * doy + 2) / 153;
    d = (int)(doy - (153 * mp + 2) / 5 + 1);
    m = (int)(mp < 10 ? mp + 3 : mp - 9);
    y += (m <= 2);
}

inline std::string formatDate(long day) {
    if (day >= NO_EXPIRY) return "";
    int y, m, d;
    civilFromDays(day, y, m, d);
    char buf[48];
    std::snprintf(buf, sizeof buf, "%04d-%02d-%02d", y, m, d);
    return buf;
}

// Strict yyyy-mm-dd parser. Empty string means "no expiry".
inline bool parseDate(const std::string& s, long& out) {
    if (s.empty()) { out = NO_EXPIRY; return true; }
    int y, m, d;
    char extra;
    if (std::sscanf(s.c_str(), "%d-%d-%d%c", &y, &m, &d, &extra) != 3) return false;
    if (m < 1 || m > 12 || d < 1 || d > 31 || y < 1970 || y > 9000) return false;
    long v = daysFromCivil(y, m, d);
    if (formatDate(v) != s) return false;  // rejects 2026-02-31 etc.
    out = v;
    return true;
}

inline long today() {
    if (const char* e = std::getenv("SMARTSTOCK_TODAY")) {  // handy for demos/tests
        long d;
        if (parseDate(e, d) && d != NO_EXPIRY) return d;
    }
    std::time_t t = std::time(nullptr);
    std::tm lt;
#ifdef _WIN32
    localtime_s(&lt, &t);
#else
    localtime_r(&t, &lt);
#endif
    return daysFromCivil(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
}

inline double r2(double v) { return std::round(v * 100.0) / 100.0; }

}  // namespace util

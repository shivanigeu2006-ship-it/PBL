#!/bin/sh

set -e
curl -L -o httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
curl -L -o json.hpp  https://github.com/nlohmann/json/releases/latest/download/json.hpp
echo "Done: httplib.h and json.hpp downloaded."

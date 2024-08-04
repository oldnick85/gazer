rm -rf external
git clone --depth=1 -c advice.detachedHead=false --branch=v0.14.3 https://github.com/yhirose/cpp-httplib.git external/cpp-httplib
git clone --depth=1 -c advice.detachedHead=false --branch=v3.11.3 https://github.com/nlohmann/json.git external/json
git clone --depth=1 -c advice.detachedHead=false --branch=v1.14.1 https://github.com/gabime/spdlog.git external/spdlog
sudo apt install clang-tidy clang-format
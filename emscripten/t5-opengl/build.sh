# DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1
# 使 函数 emscripten_webgl_create_context 支持 css selector
emcc -std=c++11 -s USE_WEBGL2=1 ./lib.cpp -o lib.js -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1

idf_component_register(SRCS "main.cpp" INCLUDE_DIRS "." "firmware" REQUIRES esp32_jet)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -fno-math-errno -Wframe-larger-than=2048)

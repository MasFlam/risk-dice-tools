cmake_minimum_required(VERSION 3.29)

#set(CMAKE_VERBOSE_MAKEFILE on)

project(RiskDice VERSION 0.1.0 LANGUAGES C)

add_executable(riskdice riskdice.c)

target_compile_features(riskdice PRIVATE c_std_11)

target_compile_options(riskdice PRIVATE
	"-sSTRICT"
	"-Wall"
	"-Wpedantic"
)

target_link_options(riskdice PRIVATE
	"-sSTRICT"
	"--no-entry"
	"--emit-tsd=riskdice.ts.d"
	"-sENVIRONMENT=web"
	"-sEXPORT_ES6=1"
	"-sUSE_ES6_IMPORT_META=0"
	"-sALLOW_MEMORY_GROWTH=1"
	"-sEXPORTED_FUNCTIONS=_malloc,_free,_calc_battle_distrib,_destroy_battle_distrib,_apply_balance"
	"-sEXPORTED_RUNTIME_METHODS=getValue,setValue"
)

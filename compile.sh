#!/bin/sh

emcc riskdice.c -O3 -o riskdice.js \
	-sEXPORTED_FUNCTIONS=_malloc,_free \
	-sEXPORTED_RUNTIME_METHODS=ccall,setValue,getValue \
	-sMODULARIZE

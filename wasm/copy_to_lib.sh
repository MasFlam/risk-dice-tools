#!/bin/sh

cat prepend.js build/riskdice.js | sed s/\'riskdice\.wasm\'/viteWasmFileUrl/g >../src/lib/wasm/riskdice.js
cat prepend.d.ts build/riskdice.ts.d append.d.ts >../src/riskdice.d.ts
cp build/riskdice.wasm ../src/lib/wasm/riskdice.wasm

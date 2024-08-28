# Risk Dice Tools

[Risk Dice Tools](https://riskdice.masflam.com) is collection of tools for use with the *Risk: Global Domination* board game.
These calculators can analyze battles potentially involving capitals, using both
True Random and Balanced Blitz dice modes. The math behind them is exactly
the same as that used in the video game adaptation by SMG Studios. The only difference
being that SMG Risk approximates the underlying true random dice roll distributions for
Balance Blitz, whereas these tools do not. That's why they can fail on large inputs.
More precisely, when (*attacking troops* &times; *defending troops*) is too large,
the WASM module can't allocate enough memory for its calculations.

## Building

The tools rely a WASM module for all the underlying calculations, which you have to compile using Emscripten.
So first of all, make sure you have [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) installed and configured properly.
Then, just run `make compile` and it should compile the C code into `site/riskdice.wasm` and the accompanyig `site/riskdice.js`.
And you're done. The entire website can be statically hosted, no APIs on the server side; all the computation happens in the browser window.

## License

This repository is licensed under the GNU Affero General Public License version 3.

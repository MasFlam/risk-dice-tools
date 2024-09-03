# Risk Dice Tools

[Risk Dice Tools](https://riskdice.masflam.com) is collection of tools for use with the *Risk: Global Domination* board game.
These calculators can analyze battles potentially involving capitals and zombies, using both
True Random and Balanced Blitz dice modes. The math behind them is exactly
the same as that used in the video game adaptation by SMG Studios. The only difference
being that SMG Risk approximates the underlying true random dice roll distributions for
Balance Blitz, whereas these tools do not. That's why they can fail on large inputs.
More precisely, when (*attacking troops* &times; *defending troops*) is too large,
the WASM module can't allocate enough memory for its calculations.

## Building

First you have to build the WASM module. For that you need [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
and [CMake](https://cmake.org). I also haven't tested (and probably won't test) whether the CMake configuration works on anything other than Linux.

```
$ cd wasm
$ emcmake cmake -B build
$ make -C build
```

Then you copy the build artifacts over to the source tree: (*I should automate this better somehow*)

```
$ ./copy_to_lib.sh   # Needs to be run inside `/wasm`
```

And finally build the whole website, for which you need npm of something like that (comes with [Node.js](https://nodejs.org)):

```
$ cd ..
$ npm install
$ npm run build
```

You should end up with a static side generated in a directory called `build` located at the root of the repository.
Host it using nginx or whatever.

## License

This repository is licensed under the GNU Affero General Public License version 3.

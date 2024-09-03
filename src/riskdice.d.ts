declare module "$lib/wasm/riskdice" {
// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
declare namespace RuntimeExports {
    /**
     * @param {number} ptr
     * @param {string} type
     */
    function getValue(ptr: number, type?: string): any;
    /**
     * @param {number} ptr
     * @param {number} value
     * @param {string} type
     */
    function setValue(ptr: number, value: number, type?: string): void;
}
interface WasmModule {
  _calc_battle_distrib(_0: number, _1: number): number;
  _malloc(_0: number): number;
  _free(_0: number): void;
  _destroy_battle_distrib(_0: number): void;
  _apply_balance(_0: number, _1: number): number;
}

export type MainModule = WasmModule & typeof RuntimeExports;
export default function MainModuleFactory (options?: unknown): Promise<MainModule>;
} // declare module "riskdice"

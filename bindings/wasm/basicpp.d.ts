export interface BasicPlusPlusEngine {
  exec(code: string): number;
  eval(expression: string): number | string;
  getVersion(): string;
  shutdown(): void;
}

export declare function createBASICEngine(ramPoolBytes?: number): Promise<BasicPlusPlusEngine>;

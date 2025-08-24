export interface Vec2 {
  x: number;
  y: number;
}

export interface Color {
  r: number;
  g: number;
  b: number;
  a: number;
}

export interface Transform {
  x: number;
  y: number;
  rotation?: number;
  scaleX?: number;
  scaleY?: number;
  originX?: number;
  originY?: number;
}

export declare class TensaiEngine {
  constructor(
    title: string,
    width: number,
    height: number,
    fullscreen: boolean,
    vsync: boolean,
  );

  load: (() => void) | null;
  update: ((dt: number) => void) | null;
  draw: (() => void) | null;

  run(): void;
  quit(): void;

  getWidth(): number;
  getHeight(): number;
  setTitle(title: string): void;
  setFullscreen(fullscreen: boolean): void;

  loadTexture(path: string): string | undefined;
  loadFont(path: string, size: number): string | undefined;
  loadSound(path: string): string | undefined;
  loadMusic(path: string): string | undefined;

  isKeyDown(key: number): boolean;
  isKeyPressed(key: number): boolean;
  isKeyReleased(key: number): boolean;

  isMouseDown(button: number): boolean;
  isMousePressed(button: number): boolean;
  isMouseReleased(button: number): boolean;

  getMouseX(): number;
  getMouseY(): number;
  getMousePosition(): Vec2;

  getDelta(): number;
  getTime(): number;
  getFPS(): number;

  clear(r?: number, g?: number, b?: number, a?: number): void;
  setColor(r: number, g: number, b: number, a?: number): void;

  drawPoint(x: number, y: number): void;
  drawLine(x1: number, y1: number, x2: number, y2: number): void;
  drawRect(
    x: number,
    y: number,
    width: number,
    height: number,
    filled?: boolean,
  ): void;
  drawCircle(x: number, y: number, radius: number, filled?: boolean): void;

  drawTexture(
    texture: string,
    x: number,
    y: number,
    rotation?: number,
    scaleX?: number,
    scaleY?: number,
    originX?: number,
    originY?: number,
    tintR?: number,
    tintG?: number,
    tintB?: number,
    tintA?: number,
  ): void;

  drawText(
    text: string,
    x: number,
    y: number,
    r?: number,
    g?: number,
    b?: number,
    a?: number,
  ): void;

  setFont(fontKey: string): void;

  playSound(path: string, volume?: number): void;
  playMusic(path: string, loops?: number): void;
  stopMusic(): void;
  setMusicVolume(volume: number): void;

  randomInt(min: number, max: number): number;
  randomFloat(min?: number, max?: number): number;
  randomBool(): boolean;
}

export declare function Tensai(
  title: string,
  width: number,
  height: number,
  fullscreen: boolean,
  vsync: boolean,
): TensaiEngine;

export declare const Keys: {
  readonly A: number;
  readonly B: number;
  readonly C: number;
  readonly D: number;
  readonly E: number;
  readonly F: number;
  readonly G: number;
  readonly H: number;
  readonly I: number;
  readonly J: number;
  readonly K: number;
  readonly L: number;
  readonly M: number;
  readonly N: number;
  readonly O: number;
  readonly P: number;
  readonly Q: number;
  readonly R: number;
  readonly S: number;
  readonly T: number;
  readonly U: number;
  readonly V: number;
  readonly W: number;
  readonly X: number;
  readonly Y: number;
  readonly Z: number;
  readonly SPACE: number;
  readonly RETURN: number;
  readonly ESCAPE: number;
  readonly UP: number;
  readonly DOWN: number;
  readonly LEFT: number;
  readonly RIGHT: number;
  readonly LSHIFT: number;
  readonly RSHIFT: number;
  readonly LCTRL: number;
  readonly RCTRL: number;
  readonly LALT: number;
  readonly RALT: number;
  readonly TAB: number;
  readonly BACKSPACE: number;
  readonly DELETE: number;
  readonly HOME: number;
  readonly END: number;
  readonly PAGEUP: number;
  readonly PAGEDOWN: number;
  readonly F1: number;
  readonly F2: number;
  readonly F3: number;
  readonly F4: number;
  readonly F5: number;
  readonly F6: number;
  readonly F7: number;
  readonly F8: number;
  readonly F9: number;
  readonly F10: number;
  readonly F11: number;
  readonly F12: number;
  readonly "0": number;
  readonly "1": number;
  readonly "2": number;
  readonly "3": number;
  readonly "4": number;
  readonly "5": number;
  readonly "6": number;
  readonly "7": number;
  readonly "8": number;
  readonly "9": number;
};

export declare const Mouse: {
  readonly LEFT: number;
  readonly MIDDLE: number;
  readonly RIGHT: number;
};

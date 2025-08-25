const { Tensai, Keys, Mouse } = require('../index.js');

const tensai = Tensai("Tensai Game Example", 1920, 1080, true, true);

let playerX = 400;
let playerY = 300;
let playerSpeed = 200;
let particles = [];
let texture = null;
let font = null;
let backgroundMusic = null;
let jumpSound = null;

tensai.load = () => {
  console.log("Loading game assets...");
  
  texture = tensai.loadTexture("assets/player.png");
  font = tensai.loadFont("assets/font.ttf", 24);
  backgroundMusic = tensai.loadMusic("assets/background.ogg");
  jumpSound = tensai.loadSound("assets/jump.wav");
  
  //if (font) tensai.setFont(font);
  if (backgroundMusic) tensai.playMusic(backgroundMusic);
  
  for (let i = 0; i < 50; i++) {
    particles.push({
      x: tensai.randomFloat(0, 800),
      y: tensai.randomFloat(0, 600),
      vx: tensai.randomFloat(-50, 50),
      vy: tensai.randomFloat(-50, 50),
      color: {
        r: tensai.randomInt(100, 255),
        g: tensai.randomInt(100, 255),
        b: tensai.randomInt(100, 255)
      },
      size: tensai.randomFloat(2, 8),
      life: tensai.randomFloat(2, 5),
      maxLife: tensai.randomFloat(2, 5)
    });
  }
};

tensai.update = (dt) => {
  const speed = playerSpeed * dt;
  
  if (tensai.isKeyDown(Keys.LEFT) || tensai.isKeyDown(Keys.A)) {
    playerX -= speed;
  }
  if (tensai.isKeyDown(Keys.RIGHT) || tensai.isKeyDown(Keys.D)) {
    playerX += speed;
  }
  if (tensai.isKeyDown(Keys.UP) || tensai.isKeyDown(Keys.W)) {
    playerY -= speed;
  }
  if (tensai.isKeyDown(Keys.DOWN) || tensai.isKeyDown(Keys.S)) {
    playerY += speed;
  }
  
  if (tensai.isKeyPressed(Keys.SPACE)) {
    if (jumpSound) tensai.playSound(jumpSound);
    
    for (let i = 0; i < 10; i++) {
      particles.push({
        x: playerX + tensai.randomFloat(-20, 20),
        y: playerY + tensai.randomFloat(-20, 20),
        vx: tensai.randomFloat(-100, 100),
        vy: tensai.randomFloat(-100, 100),
        color: {
          r: tensai.randomInt(200, 255),
          g: tensai.randomInt(50, 150),
          b: tensai.randomInt(50, 150)
        },
        size: tensai.randomFloat(3, 10),
        life: tensai.randomFloat(1, 3),
        maxLife: tensai.randomFloat(1, 3)
      });
    }
  }
  
  if (tensai.isKeyPressed(Keys.ESCAPE)) {
    tensai.quit();
  }
  
  playerX = Math.max(0, Math.min(tensai.getWidth(), playerX));
  playerY = Math.max(0, Math.min(tensai.getHeight(), playerY));
  
  for (let i = particles.length - 1; i >= 0; i--) {
    const p = particles[i];
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    p.life -= dt;
    
    p.vx *= 0.98;
    p.vy *= 0.98;
    
    if (p.x < 0 || p.x > tensai.getWidth()) p.vx *= -0.8;
    if (p.y < 0 || p.y > tensai.getHeight()) p.vy *= -0.8;
    
    if (p.life <= 0) {
      particles.splice(i, 1);
    }
  }
  
  if (tensai.isMousePressed(Mouse.LEFT)) {
    const mousePos = tensai.getMousePosition();
    for (let i = 0; i < 15; i++) {
      particles.push({
        x: mousePos.x + tensai.randomFloat(-10, 10),
        y: mousePos.y + tensai.randomFloat(-10, 10),
        vx: tensai.randomFloat(-150, 150),
        vy: tensai.randomFloat(-150, 150),
        color: {
          r: tensai.randomInt(100, 255),
          g: tensai.randomInt(100, 255),
          b: tensai.randomInt(200, 255)
        },
        size: tensai.randomFloat(2, 6),
        life: tensai.randomFloat(0.5, 2),
        maxLife: tensai.randomFloat(0.5, 2)
      });
    }
  }
};

tensai.draw = () => {
  tensai.clear(20, 25, 40);
  
  tensai.setColor(50, 50, 100, 100);
  for (let i = 0; i < 20; i++) {
    tensai.drawCircle(
      tensai.randomFloat(0, tensai.getWidth()),
      tensai.randomFloat(0, tensai.getHeight()),
      tensai.randomFloat(50, 200),
      false
    );
  }
  
  for (const p of particles) {
    const alpha = Math.floor((p.life / p.maxLife) * 255);
    tensai.setColor(p.color.r, p.color.g, p.color.b, alpha);
    tensai.drawCircle(p.x, p.y, p.size, true);
  }
  
  if (texture) {
    const time = tensai.getTime();
    const bounce = Math.sin(time * 4) * 10;
    tensai.drawTexture(
      texture,
      playerX,
      playerY + bounce,
      time * 0.5,
      1.5,
      1.5,
      32,
      32,
      255,
      255,
      255,
      255,
      "radians"
    );
  } else {
    tensai.setColor(255, 100, 100);
    tensai.drawCircle(playerX, playerY, 20, true);
  }
  
  tensai.setColor(255, 255, 255);
  const mousePos = tensai.getMousePosition();
  tensai.drawLine(mousePos.x - 10, mousePos.y, mousePos.x + 10, mousePos.y);
  tensai.drawLine(mousePos.x, mousePos.y - 10, mousePos.x, mousePos.y + 10);
  
  tensai.setColor(255, 255, 255);
  tensai.drawText(`FPS: ${Math.round(tensai.getFPS())}`, 50, 10);
  tensai.drawText(`Time: ${tensai.getTime().toFixed(2)}s`, 10, 40);
  tensai.drawText(`Particles: ${particles.length}`, 10, 70);
  tensai.drawText(`Player: (${Math.round(playerX)}, ${Math.round(playerY)})`, 10, 100);
  tensai.drawText(`Mouse: (${Math.round(mousePos.x)}, ${Math.round(mousePos.y)})`, 10, 130);
  tensai.drawText("WASD/Arrows: Move | Space: Particles | LClick: Mouse particles | ESC: Quit", 10, tensai.getHeight() - 30);
  
  tensai.setColor(100, 255, 100, 128);
  tensai.drawRect(playerX - 30, playerY - 30, 60, 60, false);
  
  tensai.setColor(255, 255, 100);
  for (let i = 0; i < 360; i += 30) {
    const rad = (i * Math.PI) / 180;
    const x1 = playerX + Math.cos(rad) * 40;
    const y1 = playerY + Math.sin(rad) * 40;
    const x2 = playerX + Math.cos(rad) * 50;
    const y2 = playerY + Math.sin(rad) * 50;
    tensai.drawLine(x1, y1, x2, y2);
  }
  
  const polygonPoints = [];
  const sides = 6;
  const radius = 80;
  for (let i = 0; i < sides; i++) {
    const angle = (i * 2 * Math.PI) / sides + tensai.getTime();
    polygonPoints.push({
      x: tensai.getWidth() - 100 + Math.cos(angle) * radius,
      y: 100 + Math.sin(angle) * radius
    });
  }
  
  tensai.setColor(255, 150, 50, 180);
  if (polygonPoints.length >= 3) {
    for (let i = 0; i < polygonPoints.length; i++) {
      const next = (i + 1) % polygonPoints.length;
      tensai.drawLine(
        polygonPoints[i].x, polygonPoints[i].y,
        polygonPoints[next].x, polygonPoints[next].y
      );
    }
  }
};

console.log("Starting Tensai game engine...");
console.log("Controls: WASD/Arrow keys to move, Space for particles, Left click for mouse particles, ESC to quit");

tensai.run();

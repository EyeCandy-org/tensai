/*
This was written by Claude and slightly changed formatting. used this prompt:
"using this as an example, make me a platformer game."
Attachments: test/test.js
*/


const { Tensai, Keys, Mouse } = require('../index.js');
const tensai = Tensai("Tensai Platformer", 1280, 720, true, true);
const GRAVITY = 980;
const JUMP_FORCE = -450;
const PLAYER_SPEED = 300;
const GROUND_FRICTION = 0.85;
const AIR_RESISTANCE = 0.95;
let player = {
  x: 100,
  y: 300,
  vx: 0,
  vy: 0,
  width: 32,
  height: 32,
  grounded: false,
  canJump: false,
  facing: 1,
  health: 100,
  score: 0,
  jumpCount: 0,
  maxJumps: 2
};

let camera = { x: 0, y: 0 };
let platforms = [];
let enemies = [];
let collectibles = [];
let particles = [];
let gameState = "playing";
let levelTime = 0;
let playerTexture = null;
let font = null;
let jumpSound = null;
let collectSound = null;
let hitSound = null;
let bgMusic = null;
tensai.load = () => {
  playerTexture = tensai.loadTexture("assets/player.png");
  font = tensai.loadFont("assets/font.ttf", 18);
  if (font) tensai.setFont(font);
  jumpSound = tensai.loadSound("assets/jump.wav");
  collectSound = tensai.loadSound("assets/collect.wav");
  hitSound = tensai.loadSound("assets/hit.wav");
  bgMusic = tensai.loadMusic("assets/background.ogg");
  if (bgMusic) tensai.playMusic(bgMusic);
  generateLevel();
};

function generateLevel() {
  platforms = [
    { x: 0, y: 680, width: 1280, height: 40, type: "ground" },
    { x: 200, y: 580, width: 150, height: 20, type: "platform" },
    { x: 450, y: 480, width: 100, height: 20, type: "platform" },
    { x: 650, y: 380, width: 150, height: 20, type: "platform" },
    { x: 900, y: 280, width: 100, height: 20, type: "platform" },
    { x: 1100, y: 480, width: 150, height: 20, type: "platform" },
    { x: 1400, y: 580, width: 200, height: 20, type: "platform" },
    { x: 1700, y: 400, width: 100, height: 20, type: "platform" },
    { x: 1900, y: 300, width: 150, height: 20, type: "platform" },
    { x: 2200, y: 500, width: 100, height: 20, type: "platform" },
    { x: 2500, y: 200, width: 200, height: 20, type: "platform" },
  ];

  enemies = [
    { x: 300, y: 540, vx: 50, width: 24, height: 24, health: 1, dir: 1, platform: 1 },
    { x: 700, y: 340, vx: 30, width: 24, height: 24, health: 1, dir: -1, platform: 3 },
    { x: 1150, y: 440, vx: 40, width: 24, height: 24, health: 1, dir: 1, platform: 5 },
    { x: 1950, y: 260, vx: 35, width: 24, height: 24, health: 1, dir: -1, platform: 8 },
  ];

  collectibles = [
    { x: 250, y: 540, type: "coin", collected: false, value: 10 },
    { x: 480, y: 440, type: "coin", collected: false, value: 10 },
    { x: 720, y: 340, type: "gem", collected: false, value: 50 },
    { x: 950, y: 240, type: "coin", collected: false, value: 10 },
    { x: 1450, y: 540, type: "gem", collected: false, value: 50 },
    { x: 1750, y: 360, type: "coin", collected: false, value: 10 },
    { x: 2000, y: 260, type: "coin", collected: false, value: 10 },
    { x: 2300, y: 460, type: "coin", collected: false, value: 10 },
    { x: 2600, y: 160, type: "gem", collected: false, value: 50 },
  ];
}

function checkCollision(rect1, rect2) {
  return rect1.x < rect2.x + rect2.width &&
         rect1.x + rect1.width > rect2.x &&
         rect1.y < rect2.y + rect2.height &&
         rect1.y + rect1.height > rect2.y;
}

function addParticle(x, y, color, count = 5) {
  for (let i = 0; i < count; i++) {
    particles.push({
      x: x + tensai.randomFloat(-10, 10),
      y: y + tensai.randomFloat(-10, 10),
      vx: tensai.randomFloat(-100, 100),
      vy: tensai.randomFloat(-200, -50),
      color: color,
      size: tensai.randomFloat(2, 5),
      life: tensai.randomFloat(0.5, 1.5),
      maxLife: tensai.randomFloat(0.5, 1.5)
    });
  }
}

tensai.update = (dt) => {
  if (gameState !== "playing") return;
  levelTime += dt;
  const prevX = player.x;
  const prevY = player.y;
  if (tensai.isKeyDown(Keys.LEFT) || tensai.isKeyDown(Keys.A)) {
    player.vx -= PLAYER_SPEED * dt * 2;
    player.facing = -1;
  }
  if (tensai.isKeyDown(Keys.RIGHT) || tensai.isKeyDown(Keys.D)) {
    player.vx += PLAYER_SPEED * dt * 2;
    player.facing = 1;
  }
  
  if ((tensai.isKeyPressed(Keys.SPACE) || tensai.isKeyPressed(Keys.UP) || tensai.isKeyPressed(Keys.W)) && player.jumpCount < player.maxJumps) {
    player.vy = JUMP_FORCE;
    player.grounded = false;
    player.canJump = false;
    player.jumpCount++;
    if (jumpSound) tensai.playSound(jumpSound);
    addParticle(player.x + player.width/2, player.y + player.height, {r: 100, g: 200, b: 255});
  }
  
  if (tensai.isKeyPressed(Keys.ESCAPE)) {
    tensai.quit();
  }
  
  player.vx = Math.max(-PLAYER_SPEED, Math.min(PLAYER_SPEED, player.vx));
  if (player.grounded) {
    player.vx *= GROUND_FRICTION;
  } else {
    player.vx *= AIR_RESISTANCE;
    player.vy += GRAVITY * dt;
  }
  
  player.x += player.vx * dt;
  player.y += player.vy * dt;
  player.grounded = false;
  for (const platform of platforms) {
    if (checkCollision(player, platform)) {
      if (prevY + player.height <= platform.y && player.vy >= 0) {
        player.y = platform.y - player.height;
        player.vy = 0;
        player.grounded = true;
        player.canJump = true;
        player.jumpCount = 0;
      } else if (prevY >= platform.y + platform.height && player.vy <= 0) {
        player.y = platform.y + platform.height;
        player.vy = 0;
      } else if (prevX + player.width <= platform.x) {
        player.x = platform.x - player.width;
        player.vx = 0;
      } else if (prevX >= platform.x + platform.width) {
        player.x = platform.x + platform.width;
        player.vx = 0;
      }
    }
  }
  
  for (let i = enemies.length - 1; i >= 0; i--) {
    const enemy = enemies[i];
    enemy.x += enemy.vx * enemy.dir * dt;
    const platform = platforms[enemy.platform];
    if (platform) {
      if (enemy.x <= platform.x || enemy.x + enemy.width >= platform.x + platform.width) {
        enemy.dir *= -1;
      }
      enemy.x = Math.max(platform.x, Math.min(platform.x + platform.width - enemy.width, enemy.x));
    }
    
    if (checkCollision(player, enemy)) {
      if (prevY + player.height <= enemy.y && player.vy >= 0) {
        enemies.splice(i, 1);
        player.vy = JUMP_FORCE * 0.7;
        player.score += 100;
        addParticle(enemy.x + enemy.width/2, enemy.y + enemy.height/2, {r: 255, g: 100, b: 100}, 8);
        if (hitSound) tensai.playSound(hitSound);
      } else {
        player.health -= 20;
        player.vx = player.facing * -200;
        player.vy = -200;
        addParticle(player.x + player.width/2, player.y + player.height/2, {r: 255, g: 50, b: 50}, 10);
        if (hitSound) tensai.playSound(hitSound);
        if (player.health <= 0) {
          gameState = "gameover";
        }
      }
    }
  }
  
  for (const collectible of collectibles) {
    if (!collectible.collected && checkCollision(player, { x: collectible.x, y: collectible.y, width: 16, height: 16 })) {
      collectible.collected = true;
      player.score += collectible.value;
      const color = collectible.type === "gem" ? {r: 255, g: 100, b: 255} : {r: 255, g: 255, b: 100};
      addParticle(collectible.x + 8, collectible.y + 8, color, 6);
      if (collectSound) tensai.playSound(collectSound);
    }
  }
  
  for (let i = particles.length - 1; i >= 0; i--) {
    const p = particles[i];
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    p.vy += 300 * dt;
    p.life -= dt;
    p.vx *= 0.98;
    if (p.life <= 0) {
      particles.splice(i, 1);
    }
  }
  
  camera.x = player.x - tensai.getWidth() / 2;
  camera.y = player.y - tensai.getHeight() / 2;
  camera.x = Math.max(0, camera.x);
  camera.y = Math.max(-200, Math.min(100, camera.y));
  if (player.y > tensai.getHeight() + 100) {
    player.health = 0;
    gameState = "gameover";
  }
  
  if (tensai.isKeyPressed(Keys.R)) {
    player = { x: 100, y: 300, vx: 0, vy: 0, width: 32, height: 32, grounded: false, canJump: false, facing: 1, health: 100, score: 0, jumpCount: 0, maxJumps: 2 };
    particles = [];
    levelTime = 0;
    gameState = "playing";
    generateLevel();
  }
};

tensai.draw = () => {
  tensai.clear(50, 150, 255);
  const gradient = Math.sin(levelTime * 0.5) * 20 + 50;
  tensai.setColor(100, 180 + gradient, 255, 50);
  for (let i = 0; i < 10; i++) {
    tensai.drawCircle(
      200 + i * 150 - camera.x * 0.1,
      100 + Math.sin(levelTime + i) * 30 - camera.y * 0.1,
      30 + Math.sin(levelTime * 2 + i) * 10,
      false
    );
  }
  
  for (const platform of platforms) {
    const screenX = platform.x - camera.x;
    const screenY = platform.y - camera.y;
    if (screenX + platform.width > -50 && screenX < tensai.getWidth() + 50) {
      if (platform.type === "ground") {
        tensai.setColor(101, 67, 33);
      } else {
        tensai.setColor(139, 69, 19);
      }
      tensai.drawRect(screenX, screenY, platform.width, platform.height, true);
      tensai.setColor(160, 82, 45);
      tensai.drawRect(screenX, screenY, platform.width, 4, true);
    }
  }
  
  for (const enemy of enemies) {
    const screenX = enemy.x - camera.x;
    const screenY = enemy.y - camera.y;
    if (screenX + enemy.width > -50 && screenX < tensai.getWidth() + 50) {
      const bounce = Math.sin(levelTime * 6) * 2;
      tensai.setColor(255, 50, 50);
      tensai.drawCircle(screenX + enemy.width/2, screenY + enemy.height/2 + bounce, enemy.width/2, true);
      
      tensai.setColor(200, 30, 30);
      tensai.drawCircle(screenX + enemy.width/2 - 4 * enemy.dir, screenY + enemy.height/2 - 4 + bounce, 3, true);
      tensai.drawCircle(screenX + enemy.width/2 + 4 * enemy.dir, screenY + enemy.height/2 - 4 + bounce, 3, true);
    }
  }
  
  for (const collectible of collectibles) {
    if (collectible.collected) continue;
    const screenX = collectible.x - camera.x;
    const screenY = collectible.y - camera.y;
    if (screenX > -50 && screenX < tensai.getWidth() + 50) {
      const float = Math.sin(levelTime * 4 + collectible.x * 0.01) * 5;
      const rotation = levelTime * 2;
      if (collectible.type === "coin") {
        tensai.setColor(255, 215, 0);
        tensai.drawCircle(screenX + 8, screenY + 8 + float, 8, true);
      } else {
        tensai.setColor(255, 0, 255);
        const diamondSize = 10;
        const diamondPoints = [
          { x: screenX + 8, y: screenY + 8 + float - diamondSize },
          { x: screenX + 8 + diamondSize, y: screenY + 8 + float },
          { x: screenX + 8, y: screenY + 8 + float + diamondSize },
          { x: screenX + 8 - diamondSize, y: screenY + 8 + float }
        ];
        tensai.drawPolygon(diamondPoints, true);
      }
    }
  }
  
  for (const p of particles) {
    const screenX = p.x - camera.x;
    const screenY = p.y - camera.y;
    const alpha = Math.floor((p.life / p.maxLife) * 255);
    tensai.setColor(p.color.r, p.color.g, p.color.b, alpha);
    tensai.drawCircle(screenX, screenY, p.size, true);
  }
  
  const screenX = player.x - camera.x;
  const screenY = player.y - camera.y;
  if (playerTexture) {
    const bounce = player.grounded ? Math.sin(levelTime * 8) * 2 : 0;
    tensai.drawTexture(
      playerTexture,
      screenX,
      screenY + bounce,
      0,
      player.facing,
      1,
      player.width/2,
      player.height/2,
      255, 255, 255, 255
    );
  } else {
    tensai.setColor(100, 255, 100);
    tensai.drawRect(screenX, screenY, player.width, player.height, true);
    tensai.setColor(50, 200, 50);
    if (player.facing === 1) {
      tensai.drawCircle(screenX + 24, screenY + 8, 3, true);
    } else {
      tensai.drawCircle(screenX + 8, screenY + 8, 3, true);
    }
  }
  
  tensai.setColor(255, 255, 255);
  tensai.drawText(`Score: ${player.score}`, 10, 10);
  tensai.drawText(`Health: ${Math.max(0, player.health)}`, 10, 35);
  tensai.drawText(`Time: ${levelTime.toFixed(1)}s`, 10, 60);
  tensai.drawText(`FPS: ${Math.round(tensai.getFPS())}`, 10, 85);
  const healthBarWidth = 200;
  const healthPercent = Math.max(0, player.health / 100);
  tensai.setColor(100, 100, 100);
  tensai.drawRect(tensai.getWidth() - healthBarWidth - 10, 10, healthBarWidth, 20, true);
  tensai.setColor(255 * (1 - healthPercent), 255 * healthPercent, 0);
  tensai.drawRect(tensai.getWidth() - healthBarWidth - 10, 10, healthBarWidth * healthPercent, 20, true);
  tensai.setColor(255, 255, 255);
  tensai.drawText("Controls: A/D or Arrows - Move | Space/W/Up - Jump | ESC - Quit", 10, tensai.getHeight() - 25);
  if (gameState === "gameover") {
    tensai.setColor(0, 0, 0, 180);
    tensai.drawRect(0, 0, tensai.getWidth(), tensai.getHeight(), true);
    tensai.setColor(255, 50, 50);
    tensai.drawText("GAME OVER", tensai.getWidth()/2 - 80, tensai.getHeight()/2 - 40);
    tensai.setColor(255, 255, 255);
    tensai.drawText(`Final Score: ${player.score}`, tensai.getWidth()/2 - 70, tensai.getHeight()/2);
    tensai.drawText("Press R to Restart", tensai.getWidth()/2 - 70, tensai.getHeight()/2 + 30);
  }
};

console.log("Starting Tensai Platformer...");
console.log("Jump on enemies to defeat them! Collect coins and gems for points!");
tensai.run();

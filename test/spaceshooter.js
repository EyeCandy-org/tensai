/*
This was written by Claude and slightly modified and formatted. Prompt:
"using the attached example, write a good, multi-level shooter game."
Attachments: test/platformer.js
*/

const { Tensai, Keys, Mouse } = require('../index.js');
const tensai = Tensai("Tensai Space Shooter", 1920, 1080, true, true);
const GAME_STATES = {
    MENU: 0,
    PLAYING: 1,
    GAME_OVER: 2,
    LEVEL_COMPLETE: 3,
    PAUSED: 4
};

let gameState = GAME_STATES.MENU;
let currentLevel = 1;
let maxLevel = 5;
let score = 0;
let lives = 3;
let levelTimer = 0;
let lastEnemySpawn = 0;
let player = {
    x: 960,
    y: 900,
    width: 40,
    height: 40,
    speed: 400,
    health: 100,
    maxHealth: 100,
    fireRate: 0.15,
    lastShot: 0,
    weapon: 'basic',
    powerupTimer: 0
};

let bullets = [];
let enemies = [];
let enemyBullets = [];
let explosions = [];
let powerups = [];
let stars = [];
let texture = null;
let font = null;
let backgroundMusic = null;
let shootSound = null;
let explosionSound = null;
let powerupSound = null;
function initializeLevel(level) {
    enemies = [];
    bullets = [];
    enemyBullets = [];
    explosions = [];
    powerups = [];
    levelTimer = 0;
    lastEnemySpawn = 0;
    for (let i = 0; i < 100; i++) {
        stars.push({
            x: tensai.randomFloat(0, tensai.getWidth()),
            y: tensai.randomFloat(0, tensai.getHeight()),
            speed: tensai.randomFloat(50, 200),
            brightness: tensai.randomFloat(0.3, 1.0)
        });
    }
}

function createEnemy(type = 'basic') {
    const enemy = {
        x: tensai.randomFloat(50, tensai.getWidth() - 50),
        y: -50,
        width: 30,
        height: 30,
        speed: 100 + currentLevel * 20,
        health: currentLevel * 2,
        maxHealth: currentLevel * 2,
        type: type,
        lastShot: 0,
        fireRate: 1.0,
        points: 100,
        zigzagOffset: tensai.randomFloat(0, Math.PI * 2)
    };
    
    switch(type) {
        case 'fast':
            enemy.speed *= 1.5;
            enemy.width = 25;
            enemy.height = 25;
            enemy.health = Math.ceil(enemy.health * 0.7);
            enemy.points = 150;
            break;
        case 'heavy':
            enemy.speed *= 0.6;
            enemy.width = 45;
            enemy.height = 45;
            enemy.health *= 2;
            enemy.fireRate = 0.7;
            enemy.points = 200;
            break;
        case 'boss':
            enemy.width = 100;
            enemy.height = 100;
            enemy.health *= 10;
            enemy.speed *= 0.3;
            enemy.fireRate = 0.3;
            enemy.points = 1000;
            enemy.x = tensai.getWidth() / 2 - 50;
            break;
    }
    
    return enemy;
}

function createBullet(x, y, vx, vy, isPlayer = true, damage = 1) {
    return {
        x: x,
        y: y,
        vx: vx,
        vy: vy,
        width: isPlayer ? 4 : 6,
        height: isPlayer ? 10 : 8,
        damage: damage,
        isPlayer: isPlayer,
        life: 3.0
    };
}

function createExplosion(x, y, size = 1) {
    const explosion = {
        x: x,
        y: y,
        size: size,
        maxSize: 50 * size,
        life: 0.5,
        maxLife: 0.5,
        particles: []
    };
    
    for (let i = 0; i < 20 * size; i++) {
        explosion.particles.push({
            x: x + tensai.randomFloat(-10, 10),
            y: y + tensai.randomFloat(-10, 10),
            vx: tensai.randomFloat(-200, 200),
            vy: tensai.randomFloat(-200, 200),
            life: tensai.randomFloat(0.2, 0.8),
            maxLife: tensai.randomFloat(0.2, 0.8),
            color: {
                r: tensai.randomInt(200, 255),
                g: tensai.randomInt(100, 200),
                b: tensai.randomInt(0, 100)
            }
        });
    }
    
    explosions.push(explosion);
}

function createPowerup(x, y) {
    const types = ['health', 'rapidfire', 'spread', 'shield'];
    const type = types[tensai.randomInt(0, types.length)];
    powerups.push({
        x: x,
        y: y,
        width: 25,
        height: 25,
        type: type,
        bobOffset: tensai.randomFloat(0, Math.PI * 2),
        life: 10.0
    });
}

function checkCollision(obj1, obj2) {
    return obj1.x < obj2.x + obj2.width &&
           obj1.x + obj1.width > obj2.x &&
           obj1.y < obj2.y + obj2.height &&
           obj1.y + obj1.height > obj2.y;
}

tensai.load = () => {
    console.log("Loading shooter game assets...");
    texture = tensai.loadTexture("assets/player.png");
    font = tensai.loadFont("assets/font.ttf", 24);
    if (font) tensai.setFont(font);
    shootSound = tensai.loadSound("assets/shoot.wav");
    explosionSound = tensai.loadSound("assets/explosion.wav");
    powerupSound = tensai.loadSound("assets/powerup.wav");
    backgroundMusic = tensai.loadMusic("assets/background.ogg");
    if (backgroundMusic) tensai.playMusic(backgroundMusic);
    initializeLevel(1);
};

tensai.update = (dt) => {
    switch(gameState) {
        case GAME_STATES.MENU:
            updateMenu(dt);
            break;
        case GAME_STATES.PLAYING:
            updateGame(dt);
            break;
        case GAME_STATES.GAME_OVER:
        case GAME_STATES.LEVEL_COMPLETE:
            updateTransition(dt);
            break;
        case GAME_STATES.PAUSED:
            updatePaused(dt);
            break;
    }
};

function updateMenu(dt) {
    updateStars(dt);
    if (tensai.isKeyPressed(Keys.SPACE)) {
        gameState = GAME_STATES.PLAYING;
        score = 0;
        lives = 3;
        currentLevel = 1;
        player.health = player.maxHealth;
        player.x = 960;
        player.y = 900;
        player.weapon = 'basic';
        player.fireRate = 0.15;
        player.powerupTimer = 0;
        initializeLevel(currentLevel);
    }
    
    if (tensai.isKeyPressed(Keys.ESCAPE)) {
        tensai.quit();
    }
}

function updatePaused(dt) {
    if (tensai.isKeyPressed(Keys.P)) {
        gameState = GAME_STATES.PLAYING;
    }
}

function updateTransition(dt) {
    updateStars(dt);
    if (tensai.isKeyPressed(Keys.SPACE)) {
        if (gameState === GAME_STATES.GAME_OVER) {
            gameState = GAME_STATES.MENU;
        } else if (gameState === GAME_STATES.LEVEL_COMPLETE) {
            currentLevel++;
            if (currentLevel > maxLevel) {
                gameState = GAME_STATES.MENU;
                currentLevel = 1;
            } else {
                gameState = GAME_STATES.PLAYING;
                player.health = Math.min(player.maxHealth, player.health + 25);
                initializeLevel(currentLevel);
            }
        }
    }
}

function updateGame(dt) {
    if (tensai.isKeyPressed(Keys.P)) {
        gameState = GAME_STATES.PAUSED;
        return;
    }
    
    if (tensai.isKeyPressed(Keys.ESCAPE)) {
        gameState = GAME_STATES.MENU;
        return;
    }
    
    levelTimer += dt;
    updatePlayer(dt);
    updateBullets(dt);
    updateEnemies(dt);
    updateExplosions(dt);
    updatePowerups(dt);
    updateStars(dt);
    spawnEnemies(dt);
    handleCollisions();
    if (player.health <= 0) {
        lives--;
        if (lives <= 0) {
            gameState = GAME_STATES.GAME_OVER;
        } else {
            player.health = player.maxHealth;
            player.x = tensai.getWidth() / 2;
            player.y = tensai.getHeight() - 100;
        }
    }
    
    if (enemies.length === 0 && levelTimer > 3.0) {
        gameState = GAME_STATES.LEVEL_COMPLETE;
    }
}

function updatePlayer(dt) {
    const speed = player.speed * dt;
    if (tensai.isKeyDown(Keys.LEFT) || tensai.isKeyDown(Keys.A)) {
        player.x = Math.max(0, player.x - speed);
    }
    if (tensai.isKeyDown(Keys.RIGHT) || tensai.isKeyDown(Keys.D)) {
        player.x = Math.min(tensai.getWidth() - player.width, player.x + speed);
    }
    if (tensai.isKeyDown(Keys.UP) || tensai.isKeyDown(Keys.W)) {
        player.y = Math.max(tensai.getHeight() / 2, player.y - speed);
    }
    if (tensai.isKeyDown(Keys.DOWN) || tensai.isKeyDown(Keys.S)) {
        player.y = Math.min(tensai.getHeight() - player.height, player.y + speed);
    }
    
    if (player.powerupTimer > 0) {
        player.powerupTimer -= dt;
        if (player.powerupTimer <= 0) {
            player.weapon = 'basic';
            player.fireRate = 0.15;
        }
    }
    
    if (tensai.isKeyDown(Keys.SPACE) && tensai.getTime() - player.lastShot > player.fireRate) {
        shootPlayerBullet();
        player.lastShot = tensai.getTime();
    }
}

function shootPlayerBullet() {
    switch(player.weapon) {
        case 'basic':
            bullets.push(createBullet(player.x + player.width/2, player.y, 0, -600, true));
            break;
        case 'spread':
            bullets.push(createBullet(player.x + player.width/2, player.y, -100, -600, true));
            bullets.push(createBullet(player.x + player.width/2, player.y, 0, -600, true));
            bullets.push(createBullet(player.x + player.width/2, player.y, 100, -600, true));
            break;
        case 'rapidfire':
            bullets.push(createBullet(player.x + player.width/2, player.y, 0, -800, true, 2));
            break;
    }
}

function updateBullets(dt) {
    for (let i = bullets.length - 1; i >= 0; i--) {
        const bullet = bullets[i];
        bullet.x += bullet.vx * dt;
        bullet.y += bullet.vy * dt;
        bullet.life -= dt;
        if (bullet.y < -10 || bullet.y > tensai.getHeight() + 10 || 
            bullet.x < -10 || bullet.x > tensai.getWidth() + 10 || bullet.life <= 0) {
            bullets.splice(i, 1);
        }
    }
    
    for (let i = enemyBullets.length - 1; i >= 0; i--) {
        const bullet = enemyBullets[i];
        bullet.x += bullet.vx * dt;
        bullet.y += bullet.vy * dt;
        bullet.life -= dt;
        if (bullet.y < -10 || bullet.y > tensai.getHeight() + 10 || 
            bullet.x < -10 || bullet.x > tensai.getWidth() + 10 || bullet.life <= 0) {
            enemyBullets.splice(i, 1);
        }
    }
}

function updateEnemies(dt) {
    for (let i = enemies.length - 1; i >= 0; i--) {
        const enemy = enemies[i];
        if (enemy.type === 'fast') {
            enemy.y += enemy.speed * dt;
            enemy.x += Math.sin(tensai.getTime() * 3 + enemy.zigzagOffset) * 100 * dt;
        } else if (enemy.type === 'boss' && enemy.y > 100) {
            enemy.y = 100;
            enemy.x += Math.sin(tensai.getTime()) * 50 * dt;
        } else {
            enemy.y += enemy.speed * dt;
        }
        
        enemy.x = Math.max(0, Math.min(tensai.getWidth() - enemy.width, enemy.x));
        if (tensai.getTime() - enemy.lastShot > enemy.fireRate && enemy.y > 0) {
            const dx = player.x - enemy.x;
            const dy = player.y - enemy.y;
            const dist = Math.sqrt(dx * dx + dy * dy);
            if (dist < 400) {
                enemyBullets.push(createBullet(
                    enemy.x + enemy.width/2,
                    enemy.y + enemy.height,
                    (dx / dist) * 200,
                    (dy / dist) * 200,
                    false
                ));
                enemy.lastShot = tensai.getTime();
            }
        }
        
        if (enemy.y > tensai.getHeight() + 50) {
            enemies.splice(i, 1);
        }
    }
}

function spawnEnemies(dt) {
    const spawnRate = Math.max(0.5, 2.0 - currentLevel * 0.2);
    if (tensai.getTime() - lastEnemySpawn > spawnRate && enemies.length < 10) {
        let enemyType = 'basic';
        
        if (currentLevel > 1 && Math.random() < 0.3) {
            enemyType = 'fast';
        }
        if (currentLevel > 2 && Math.random() < 0.2) {
            enemyType = 'heavy';
        }
        if (currentLevel % 3 === 0 && enemies.length === 0 && levelTimer > 3.0) {
            enemyType = 'boss';
        }
        
        enemies.push(createEnemy(enemyType));
        lastEnemySpawn = tensai.getTime();
    }
}

function updateExplosions(dt) {
    for (let i = explosions.length - 1; i >= 0; i--) {
        const explosion = explosions[i];
        explosion.life -= dt;
        for (let j = explosion.particles.length - 1; j >= 0; j--) {
            const particle = explosion.particles[j];
            particle.x += particle.vx * dt;
            particle.y += particle.vy * dt;
            particle.life -= dt;
            if (particle.life <= 0) {
                explosion.particles.splice(j, 1);
            }
        }
        
        if (explosion.life <= 0) {
            explosions.splice(i, 1);
        }
    }
}

function updatePowerups(dt) {
    for (let i = powerups.length - 1; i >= 0; i--) {
        const powerup = powerups[i];
        powerup.y += 50 * dt;
        powerup.life -= dt;
        if (powerup.y > tensai.getHeight() || powerup.life <= 0) {
            powerups.splice(i, 1);
        }
    }
}

function updateStars(dt) {
    for (const star of stars) {
        star.y += star.speed * dt;
        if (star.y > tensai.getHeight()) {
            star.y = -5;
            star.x = tensai.randomFloat(0, tensai.getWidth());
        }
    }
}

function handleCollisions() {
    for (let i = bullets.length - 1; i >= 0; i--) {
        const bullet = bullets[i];
        for (let j = enemies.length - 1; j >= 0; j--) {
            const enemy = enemies[j];
            if (checkCollision(bullet, enemy)) {
                enemy.health -= bullet.damage;
                bullets.splice(i, 1);
                if (enemy.health <= 0) {
                    score += enemy.points;
                    createExplosion(enemy.x + enemy.width/2, enemy.y + enemy.height/2);
                    if (Math.random() < 0.3) {
                        createPowerup(enemy.x, enemy.y);
                    }
                    
                    enemies.splice(j, 1);
                }
                break;
            }
        }
    }
    
    for (let i = enemyBullets.length - 1; i >= 0; i--) {
        const bullet = enemyBullets[i];
        if (checkCollision(bullet, player)) {
            player.health -= 10;
            enemyBullets.splice(i, 1);
        }
    }
    
    for (let i = enemies.length - 1; i >= 0; i--) {
        const enemy = enemies[i];
        if (checkCollision(enemy, player)) {
            player.health -= 20;
            createExplosion(enemy.x + enemy.width/2, enemy.y + enemy.height/2);
            enemies.splice(i, 1);
        }
    }
    
    for (let i = powerups.length - 1; i >= 0; i--) {
        const powerup = powerups[i];
        if (checkCollision(powerup, player)) {
            switch(powerup.type) {
                case 'health':
                    player.health = Math.min(player.maxHealth, player.health + 30);
                    break;
                case 'rapidfire':
                    player.weapon = 'rapidfire';
                    player.fireRate = 0.08;
                    player.powerupTimer = 10.0;
                    break;
                case 'spread':
                    player.weapon = 'spread';
                    player.fireRate = 0.2;
                    player.powerupTimer = 8.0;
                    break;
                case 'shield':
                    player.health = player.maxHealth;
                    break;
            }
            
            powerups.splice(i, 1);
        }
    }
}

tensai.draw = () => {
    switch(gameState) {
        case GAME_STATES.MENU:
            drawMenu();
            break;
        case GAME_STATES.PLAYING:
            drawGame();
            break;
        case GAME_STATES.GAME_OVER:
            drawGameOver();
            break;
        case GAME_STATES.LEVEL_COMPLETE:
            drawLevelComplete();
            break;
        case GAME_STATES.PAUSED:
            drawGame();
            drawPaused();
            break;
    }
};

function drawMenu() {
    tensai.clear(5, 10, 25);
    for (const star of stars) {
        tensai.setColor(255, 255, 255, Math.floor(star.brightness * 100));
        tensai.drawCircle(star.x, star.y, 1, true);
    }
    
    tensai.setColor(255, 255, 255);
    tensai.drawText("Tensai Space Shooter", tensai.getWidth()/2 - 200, 300);
    tensai.setColor(200, 200, 200);
    tensai.drawText("SPACE to Start", tensai.getWidth()/2 - 100, 500);
    tensai.drawText("WASD/Arrows: Move | SPACE: Shoot | P: Pause | ESC: Quit", tensai.getWidth()/2 - 300, 600);
    tensai.drawText("Survive 5 levels of intense space combat!", tensai.getWidth()/2 - 250, 700);
}

function drawGame() {
    tensai.clear(5, 10, 25);
    for (const star of stars) {
        tensai.setColor(255, 255, 255, Math.floor(star.brightness * 255));
        tensai.drawCircle(star.x, star.y, 1, true);
    }
    
    drawPlayer();
    drawBullets();
    drawEnemies();
    drawExplosions();
    drawPowerups();
    drawUI();
}

function drawPlayer() {
    if (player.powerupTimer > 0 && Math.sin(tensai.getTime() * 10) > 0) {
        tensai.setColor(255, 255, 100);
    } else {
        tensai.setColor(100, 255, 100);
    }
    
    tensai.drawRect(player.x, player.y, player.width, player.height, true);
    tensai.setColor(50, 200, 50);
    tensai.drawRect(player.x + 5, player.y + 5, player.width - 10, player.height - 10, true);
}

function drawBullets() {
    tensai.setColor(255, 255, 100);
    for (const bullet of bullets) {
        tensai.drawRect(bullet.x, bullet.y, bullet.width, bullet.height, true);
    }
    
    tensai.setColor(255, 100, 100);
    for (const bullet of enemyBullets) {
        tensai.drawRect(bullet.x, bullet.y, bullet.width, bullet.height, true);
    }
}

function drawEnemies() {
    for (const enemy of enemies) {
        switch(enemy.type) {
            case 'basic':
                tensai.setColor(255, 100, 100);
                break;
            case 'fast':
                tensai.setColor(255, 200, 100);
                break;
            case 'heavy':
                tensai.setColor(150, 100, 255);
                break;
            case 'boss':
                tensai.setColor(255, 50, 50);
                break;
        }
        
        tensai.drawRect(enemy.x, enemy.y, enemy.width, enemy.height, true);
        const healthPercent = enemy.health / enemy.maxHealth;
        tensai.setColor(255 * (1 - healthPercent), 255 * healthPercent, 0);
        tensai.drawRect(enemy.x, enemy.y - 8, enemy.width * healthPercent, 4, true);
    }
}

function drawExplosions() {
    for (const explosion of explosions) {
        for (const particle of explosion.particles) {
            const alpha = Math.floor((particle.life / particle.maxLife) * 255);
            tensai.setColor(particle.color.r, particle.color.g, particle.color.b, alpha);
            tensai.drawCircle(particle.x, particle.y, 3, true);
        }
    }
}

function drawPowerups() {
    for (const powerup of powerups) {
        const bob = Math.sin(tensai.getTime() * 4 + powerup.bobOffset) * 5;
        switch(powerup.type) {
            case 'health':
                tensai.setColor(100, 255, 100);
                break;
            case 'rapidfire':
                tensai.setColor(255, 255, 100);
                break;
            case 'spread':
                tensai.setColor(100, 200, 255);
                break;
            case 'shield':
                tensai.setColor(200, 100, 255);
                break;
        }
        
        tensai.drawRect(powerup.x, powerup.y + bob, powerup.width, powerup.height, true);
    }
}

function drawUI() {
    tensai.setColor(255, 255, 255);
    tensai.drawText(`Score: ${score}`, 20, 20);
    tensai.drawText(`Lives: ${lives}`, 20, 50);
    tensai.drawText(`Level: ${currentLevel}`, 20, 80);
    tensai.drawText(`FPS: ${Math.round(tensai.getFPS())}`, 20, 110);
    const healthPercent = player.health / player.maxHealth;
    tensai.setColor(255 * (1 - healthPercent), 255 * healthPercent, 0);
    tensai.drawRect(250, 20, 200 * healthPercent, 20, true);
    tensai.setColor(255, 255, 255);
    tensai.drawRect(250, 20, 200, 20, false);
    tensai.drawText("Health", 250, 45);
    if (player.powerupTimer > 0) {
        tensai.setColor(255, 255, 100);
        tensai.drawText(`Powerup: ${player.weapon.toUpperCase()} (${player.powerupTimer.toFixed(1)}s)`, 20, 140);
    }
}

function drawGameOver() {
    drawGame();
    tensai.setColor(0, 0, 0, 180);
    tensai.drawRect(0, 0, tensai.getWidth(), tensai.getHeight(), true);
    tensai.setColor(255, 100, 100);
    tensai.drawText("GAME OVER", tensai.getWidth()/2 - 120, 400);
    tensai.setColor(255, 255, 255);
    tensai.drawText(`Final Score: ${score}`, tensai.getWidth()/2 - 100, 500);
    tensai.drawText("SPACE to return to menu", tensai.getWidth()/2 - 150, 600);
}

function drawLevelComplete() {
    drawGame();
    tensai.setColor(0, 0, 0, 180);
    tensai.drawRect(0, 0, tensai.getWidth(), tensai.getHeight(), true);
    tensai.setColor(100, 255, 100);
    tensai.drawText(`LEVEL ${currentLevel} COMPLETE!`, tensai.getWidth()/2 - 200, 400);
    tensai.setColor(255, 255, 255);
    tensai.drawText(`Score: ${score}`, tensai.getWidth()/2 - 80, 500);
    if (currentLevel < maxLevel) {
        tensai.drawText("SPACE to continue to next level", tensai.getWidth()/2 - 180, 600);
    } else {
        tensai.drawText("YOU WIN! SPACE to return to menu", tensai.getWidth()/2 - 200, 600);
    }
}

function drawPaused() {
    tensai.setColor(0, 0, 0, 180);
    tensai.drawRect(0, 0, tensai.getWidth(), tensai.getHeight(), true);
    tensai.setColor(255, 255, 255);
    tensai.drawText("PAUSED", tensai.getWidth()/2 - 80, 400);
    tensai.drawText("Press P to resume", tensai.getWidth()/2 - 120, 500);
}

console.log("Starting Tensai Space Shooter...");
console.log("Prepare for intense space combat!");
tensai.run();
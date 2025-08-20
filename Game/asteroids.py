import math
import random
import pygame
import socket
import json
import numpy as np

# --------- Socket ----------
HOST = "127.0.0.1"
PORT = 5000

# --------- Config ----------
WIDTH, HEIGHT = 450, 450
FPS = 60
SHIP_THRUST = 200
SHIP_ROT_SPEED = 220  # degrees per second
FRICTION = 10
BULLET_SPEED = 500
BULLET_LIFETIME = 1.6
ASTEROID_MIN_SPEED = 10
ASTEROID_MAX_SPEED = 60
ASTEROID_SIZES = {3: 40, 2: 24, 1: 12}
START_ASTEROIDS = 20
INVULNERABLE_TIME = 2.0

# Colors
BG = (9, 9, 20)
WHITE = (240, 240, 240)
PINK = (240, 100, 180)
YELLOW = (240, 220, 90)
RED = (220, 90, 90)

def wrap_position(pos):
    x, y = pos
    x %= WIDTH
    y %= HEIGHT
    return x, y

def vec_from_angle_deg(angle_deg):
    rad = math.radians(angle_deg)
    return math.cos(rad), -math.sin(rad)

def dist(a, b):
    return math.hypot(a[0]-b[0], a[1]-b[1])

class Ship:
    def __init__(self):
        self.pos = (WIDTH / 2, HEIGHT / 2)
        self.vel = [0.0, 0.0]
        self.angle = 90.0
        self.radius = 12
        self.thrusting = False
        self.invulnerable = True
        self.invulnerable_timer = INVULNERABLE_TIME

    def update(self, dt):
        if self.thrusting:
            dx, dy = vec_from_angle_deg(self.angle)
            self.vel[0] += dx * SHIP_THRUST * dt
            self.vel[1] += dy * SHIP_THRUST * dt

        speed = np.sqrt(self.vel[0]**2 + self.vel[1]**2)
        if speed != 0:
            accel = FRICTION * dt
            if accel > speed:
                self.vel[0] = 0
                self.vel[1] = 0
            else:
                self.vel[0] -= self.vel[0] * FRICTION * dt / speed
                self.vel[1] -= self.vel[1] * FRICTION * dt / speed

        self.pos = wrap_position((self.pos[0] + self.vel[0] * dt, self.pos[1] + self.vel[1] * dt))

        if self.invulnerable:
            self.invulnerable_timer -= dt
            if self.invulnerable_timer <= 0:
                self.invulnerable = False

    def draw(self, surf):
        pts = []
        nose = vec_from_angle_deg(self.angle)
        left = vec_from_angle_deg(self.angle + 130)
        right = vec_from_angle_deg(self.angle - 130)
        scale = 14
        pts.append((self.pos[0] + nose[0]*scale, self.pos[1] + nose[1]*scale))
        pts.append((self.pos[0] + left[0]*scale*0.9, self.pos[1] + left[1]*scale*0.9))
        pts.append((self.pos[0] + right[0]*scale*0.9, self.pos[1] + right[1]*scale*0.9))

        color = YELLOW if self.invulnerable and (int(pygame.time.get_ticks() / 120) % 2 == 0) else PINK
        pygame.draw.polygon(surf, color, pts, 2)

        if self.thrusting:
            flame_dir = vec_from_angle_deg(self.angle + 180)
            fscale = 8 + random.uniform(-2, 2)
            f1 = (self.pos[0] + flame_dir[0]*fscale, self.pos[1] + flame_dir[1]*fscale)
            pygame.draw.line(surf, RED, ((pts[1][0]+pts[2][0])/2, (pts[1][1]+pts[2][1])/2), f1, 3)

class Bullet:
    def __init__(self, pos, angle, ship_vel):
        self.pos = list(pos)
        dx, dy = vec_from_angle_deg(angle)
        self.vel = [ship_vel[0] + dx * BULLET_SPEED, ship_vel[1] + dy * BULLET_SPEED]
        self.life = BULLET_LIFETIME

    def update(self, dt):
        self.pos[0] += self.vel[0] * dt
        self.pos[1] += self.vel[1] * dt
        self.pos = list(wrap_position(self.pos))
        self.life -= dt

    def draw(self, surf):
        pygame.draw.circle(surf, YELLOW, (int(self.pos[0]), int(self.pos[1])), 2)

    def alive(self):
        return self.life > 0

class Asteroid:
    def __init__(self, pos=None, size=3):
        if pos is None:
            side = random.choice(['left','right','top','bottom'])
            if side == 'left':
                x = -ASTEROID_SIZES[size]*2
                y = random.uniform(0, HEIGHT)
            elif side == 'right':
                x = WIDTH + ASTEROID_SIZES[size]*2
                y = random.uniform(0, HEIGHT)
            elif side == 'top':
                x = random.uniform(0, WIDTH)
                y = -ASTEROID_SIZES[size]*2
            else:
                x = random.uniform(0, WIDTH)
                y = HEIGHT + ASTEROID_SIZES[size]*2
            self.pos = [x, y]
        else:
            self.pos = [pos[0], pos[1]]

        self.size = size
        self.radius = ASTEROID_SIZES[size]
        angle = random.uniform(0, 360)
        spd = random.uniform(ASTEROID_MIN_SPEED, ASTEROID_MAX_SPEED) * (1.2 if size==1 else 1.0)
        dx, dy = vec_from_angle_deg(angle)
        self.vel = [dx*spd, dy*spd]
        self.vertex_count = random.randint(8, 12)
        self.jag = [random.uniform(0.7, 1.3) for _ in range(self.vertex_count)]

    def update(self, dt):
        self.pos[0] += self.vel[0] * dt
        self.pos[1] += self.vel[1] * dt
        self.pos = list(wrap_position(self.pos))

    def draw(self, surf):
        cx, cy = int(self.pos[0]), int(self.pos[1])
        pts = []
        for i in range(self.vertex_count):
            ang = 360.0 * i / self.vertex_count
            vx, vy = vec_from_angle_deg(ang)
            r = self.radius * self.jag[i]
            pts.append((cx + vx*r, cy + vy*r))
        pygame.draw.polygon(surf, WHITE, pts, 2)

class Game:
    def __init__(self):
        pygame.init()
        pygame.display.set_caption("Asteroids — Python")
        self.screen = pygame.display.set_mode((WIDTH, HEIGHT))
        self.clock = pygame.time.Clock()
        self.font = pygame.font.SysFont("Consolas", 20)
        self.bigfont = pygame.font.SysFont("Consolas", 48)
        self.last_received_key = None
        self.start_out()
        self.reset()

    def start_out(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind((HOST, PORT))
        server_socket.listen(1)
        self.connection, _ = server_socket.accept()
        self.connection.setblocking(False)  # non-blocking

    def receive_keypress(self):
        try:
            data = self.connection.recv(1)
            if data:
                char = data.decode().strip()
                if char:
                    self.last_received_key = char
        except BlockingIOError:
            pass

    def send_out(self):
        decimals = 2
        ship_data = {
            "pos": [round(self.ship.pos[0], decimals), round(self.ship.pos[1], decimals)],
            "vel": [round(self.ship.vel[0], decimals), round(self.ship.vel[1], decimals)],
            "angle": round(self.ship.angle, decimals)
        }
        asteroids_data = []
        for a in self.asteroids:
            asteroids_data.append({
                "pos": [round(a.pos[0], decimals), round(a.pos[1], decimals)],
                "vel": [round(a.vel[0], decimals), round(a.vel[1], decimals)],
                "radius": round(a.radius, decimals)
            })
        game_state = {
            "ship": ship_data,
            "asteroids": asteroids_data
        }
        msg = json.dumps(game_state) + "\n"
        try:
            self.connection.sendall(msg.encode())
        except (BrokenPipeError, ConnectionResetError):
            print("Client disconnected")
            self.connection.close()

    def reset(self):
        self.ship = Ship()
        self.bullets = []
        self.asteroids = []
        self.score = 0
        self.lives = 1
        self.spawn_wave(START_ASTEROIDS)
        self.state = 'TITLE'

    def spawn_wave(self, n):
        for _ in range(n):
            a = Asteroid(size=3)
            if dist(a.pos, self.ship.pos) < 120:
                a.pos = [a.pos[0] + 200, a.pos[1] + 200]
            self.asteroids.append(a)

    def update(self, dt):
        if self.state != 'PLAYING':
            return
        self.ship.update(dt)
        for b in self.bullets:
            b.update(dt)
        for a in self.asteroids:
            a.update(dt)
        self.bullets = [b for b in self.bullets if b.alive()]
        to_remove_ast = []
        to_add_ast = []
        bullet_remove_idxs = set()
        for bi, b in enumerate(self.bullets):
            for ai, a in enumerate(self.asteroids):
                if dist(b.pos, a.pos) < a.radius + 2:
                    bullet_remove_idxs.add(bi)
                    to_remove_ast.append(ai)
                    self.score += 100 * a.size
                    to_add_ast.extend(self.split_asteroid(a))
                    break
        self.bullets = [b for i, b in enumerate(self.bullets) if i not in bullet_remove_idxs]
        for idx in sorted(set(to_remove_ast), reverse=True):
            if 0 <= idx < len(self.asteroids):
                del self.asteroids[idx]
        self.asteroids.extend(to_add_ast)

        if not self.ship.invulnerable:
            for ai, a in enumerate(self.asteroids):
                if dist(self.ship.pos, a.pos) < a.radius + self.ship.radius - 3:
                    self.lives -= 1
                    if self.lives <= 0:
                        self.state = 'GAMEOVER'
                    else:
                        self.ship = Ship()
                        self.ship.invulnerable = True
                        self.ship.invulnerable_timer = INVULNERABLE_TIME
                    del self.asteroids[ai]
                    break

        if not self.asteroids:
            self.spawn_wave(START_ASTEROIDS + 1 + (self.score // 1000))

    def split_asteroid(self, asteroid):
        if asteroid.size > 1:
            children = []
            for _ in range(2):
                new_a = Asteroid(pos=asteroid.pos, size=asteroid.size-1)
                new_a.vel[0] += random.uniform(-1.2, 1.2)
                new_a.vel[1] += random.uniform(-1.2, 1.2)
                children.append(new_a)
            return children
        else:
            return []

    def draw_ui(self, surf):
        stext = self.font.render(f"Score: {self.score}", True, WHITE)
        surf.blit(stext, (10, 8))
        lives_text = self.font.render("Lives: " + "❤" * self.lives, True, WHITE)
        surf.blit(lives_text, (WIDTH - 140, 8))

    def draw(self):
        self.screen.fill(BG)
        for a in self.asteroids:
            a.draw(self.screen)
        for b in self.bullets:
            b.draw(self.screen)
        if self.ship:
            self.ship.draw(self.screen)
        self.draw_ui(self.screen)

        if self.state == 'TITLE':
            title = self.bigfont.render("ASTEROIDS", True, WHITE)
            hint = self.font.render("Press ENTER to start  —  Arrow keys/AWD to move  Space to shoot", True, WHITE)
            self.screen.blit(title, title.get_rect(center=(WIDTH/2, HEIGHT/2-40)))
            self.screen.blit(hint, hint.get_rect(center=(WIDTH/2, HEIGHT/2+20)))
        elif self.state == 'PAUSED':
            ptext = self.bigfont.render("PAUSED", True, WHITE)
            self.screen.blit(ptext, ptext.get_rect(center=(WIDTH/2, HEIGHT/2)))
        elif self.state == 'GAMEOVER':
            gtext = self.bigfont.render("GAME OVER", True, RED)
            score_text = self.font.render(f"Final Score: {self.score}", True, WHITE)
            restart = self.font.render("Press ENTER to restart", True, WHITE)
            self.screen.blit(gtext, gtext.get_rect(center=(WIDTH/2, HEIGHT/2-30)))
            self.screen.blit(score_text, score_text.get_rect(center=(WIDTH/2, HEIGHT/2+10)))
            self.screen.blit(restart, restart.get_rect(center=(WIDTH/2, HEIGHT/2+50)))

        pygame.display.flip()

    def run(self):
        running = True
        rot_dir = 0

        while running:
            dt = self.clock.tick(FPS) / 1000.0
            rot_dir = 0
            self.ship.thrusting = False

            # --- Receive TCP key if available ---
            tcp_key = None
            try:
                self.connection.setblocking(False)
                data = self.connection.recv(1)
                if data:
                    tcp_key = data.decode("utf-8").lower()
            except BlockingIOError:
                pass
            finally:
                self.connection.setblocking(True)

            # --- Input Handling ---
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                elif event.type == pygame.KEYDOWN:
                    key = event.key

                    if key == pygame.K_SPACE:
                        if self.state == 'PLAYING':
                            dx, dy = vec_from_angle_deg(self.ship.angle)
                            spawn = (self.ship.pos[0] + dx * 18, self.ship.pos[1] + dy * 18)
                            self.bullets.append(Bullet(spawn, self.ship.angle, self.ship.vel))
                    elif key == pygame.K_p:
                        self.state = 'PAUSED' if self.state == 'PLAYING' else 'PLAYING'
                    elif key == pygame.K_RETURN:
                        if self.state in ('TITLE', 'GAMEOVER'):
                            self.reset()
                            self.state = 'PLAYING'
                    elif key == pygame.K_ESCAPE:
                        running = False

            # --- TCP Input Emulation ---
            if tcp_key:
                if tcp_key == 'a':
                    rot_dir = -1
                elif tcp_key == 'd':
                    rot_dir = 1
                elif tcp_key == 'w':
                    self.ship.thrusting = True

            # --- Rotation ---
            if self.state == 'PLAYING' and rot_dir != 0:
                self.ship.angle += rot_dir * SHIP_ROT_SPEED * dt
                self.ship.angle %= 360

            # --- Game Logic ---
            self.update(dt)
            self.draw()
            self.send_out()

        pygame.quit()

if __name__ == '__main__':
    Game().run()

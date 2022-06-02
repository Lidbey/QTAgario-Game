from math import sqrt
from typing import Tuple
import multiprocessing
import socket
import random
import time
import sys
import os

def create_socket(host: str, port: int) -> socket.socket:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    return s


def random_unit_vector() -> Tuple[float, float]:
    x, y = [random.uniform(0, 1) for _ in range(2)]
    length = sqrt(x**2 + y**2) + 1e-4
    xdir, ydir = [random.choice([1, -1]) for _ in range(2)]
    return xdir * x / length, ydir * y / length


def simulate_movement(s: socket.socket) -> None:
    timeout = 0
    if len(sys.argv) > 4:
        s.send((sys.argv[4] + "\r\n").encode())
    while True:
        if s.recv(1024).decode() == 'Here:You are dead message' or timeout > 10:
            break
        x, y = random_unit_vector()
        speed = 0
        s.send(f"{x};{y};{speed};0\r\n".encode())
        time.sleep(random.randint(3, 8))
        #timeout += 1
    p.sock.close()


if __name__ == "__main__":
    
    host = sys.argv[1]
    port = int(sys.argv[2])
    number_of_bots = int(sys.argv[3]) if len(sys.argv) > 3 else 1
    processes = [multiprocessing.Process(target=simulate_movement, args=[create_socket(host, port)]) for _ in range(number_of_bots)]
    for p in processes:
        p.start()
    for p in processes:
        p.join()

from math import sqrt
from typing import Tuple
import multiprocessing
import socket
import random
import time
import sys


def create_socket(host: str, port: int) -> socket.socket:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    return s


def random_unit_vector() -> Tuple[float, float]:
    x, y = [random.uniform(0, 1) for _ in range(2)]
    length = sqrt(x**2 + y**2) + 1e-4
    return x / length, y / length


def simulate_movement(s: socket.socket) -> None:
    while True:
        if s.recv(1024).decode() == 'Here:You are dead message':
            break
        x, y = random_unit_vector()
        speed = 0
        s.send(f"{x};{y};{speed}\r\n".encode())
        time.sleep(1./50)  # wait 1/50 of a second


if __name__ == "__main__":
    host = sys.argv[1]
    port = int(sys.argv[2])
    number_of_bots = int(sys.argv[3]) if len(sys.argv) > 3 else 1

    s = create_socket(host, port)

    pool = multiprocessing.Pool(processes=number_of_bots)
    pool.map(simulate_movement, [s] * number_of_bots)
    pool.close()

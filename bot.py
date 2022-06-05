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
    xdir, ydir = [random.choice([1, -1]) for _ in range(2)]
    return xdir * x / length, ydir * y / length


def unit_vector_to_coords(x: float, y: float) -> Tuple[int, int]:
    length = sqrt(x**2 + y**2) + 1e-4
    return x / length, y / length


def at_destination(my_x: int, my_y: int, dest_x: int, dest_y: int) -> bool:
    x_distance = abs(my_x - dest_x)
    y_distance = abs(my_y - dest_y)

    arival_treshold = 40
    if x_distance <= arival_treshold and y_distance <= arival_treshold:
        return True

    return False


def simulate_movement(s: socket.socket) -> None:
    tactic = 0  # 0 = random, 1 = go to destination
    my_x, my_y = 0, 0

    ID = int(str(s.recv(1024).decode()).split(':')[1].split('\r\n')[0])
    print('myid:', ID, sep=' ')

    # nie pamietam co to robi XD
    if len(sys.argv) > 4:
        s.send((sys.argv[4]+"\r\n").encode())

    while True:
        message = str(s.recv(1024).decode())

        if message == 'Here:You are dead message':
            break
        print(message)
        if 'T' in message:
            message = message.split(';')
            tactic = int(message[1])
            if tactic == 1:
                dest_x = message[2]
                dest_y = message[3]
                x, y = unit_vector_to_coords(dest_x, dest_y)
                s.send(f"{x};{y};{0};0\r\n".encode())    
            print('tactic',tactic)

        if 'PPPP' in message and 'OOOO' in message:
            tmp = str(message[4:]).split('\t')
            for player in tmp:
                player_data = player.split(';')
                if player_data[0] == ID:
                    my_x = int(player_data[1])
                    my_y = int(player_data[2])
            print('my_x, my_y', my_x, my_y)

        if tactic == 1:
            print('in tactic11',my_x, my_y, dest_x, dest_y)
            if at_destination(my_x, my_y, dest_x, dest_y):
                tactic = 0
                break

        if tactic == 0:
            x, y = random_unit_vector()
            speed = 0
            s.send(f"{x};{y};{speed};0\r\n".encode())
            time.sleep(random.randint(3, 8))

    p.sock.close()


if __name__ == "__main__":
    host = sys.argv[1]
    port = int(sys.argv[2])
    number_of_bots = int(sys.argv[3]) if len(sys.argv) > 3 else 1
    processes = [
        multiprocessing.Process(
            target=simulate_movement,
            args=[create_socket(host, port)]
        )
        for _ in range(number_of_bots)
    ]

    for p in processes:
        p.start()
    for p in processes:
        p.join()

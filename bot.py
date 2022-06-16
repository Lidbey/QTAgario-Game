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

    buffer = ''
    message = ''
    player_data = []
    sleep_time = time.time()
    dest_x = 0
    speed = 0
    dest_y = 0
    while True:
        is_message_complete = False
        while is_message_complete is False:
            buffer += str(s.recv(8).decode())
            messages_list = buffer.split('\r\n')
            if len(messages_list) > 1:
                buffer = '\r\n'.join(messages_list[1:])
                message = messages_list[0]
                is_message_complete = True

        if 'T' in message:
            message = message.split(';')
            tactic = int(message[1])
            if tactic == 1:
                dest_x = float(message[2])
                dest_y = float(message[3])
                x, y = unit_vector_to_coords(dest_x - my_x, dest_y - my_y)
                s.send(f"{x};{y};{0};0\r\n".encode())
                speed = 0
            if tactic == 2:
                dest_player_id = int(message[2])
                speed = 1

        if 'PPPP' in message and 'OOOO' in message:
            after_pppp = message[
                message.index('PPPP') + len('PPPP'): message.index('OOOO')-1
            ]
            all_players = str(after_pppp).split('\t')
            for player in all_players:
                player_data = player.split(';')

                if tactic == 2 and int(player_data[0]) == dest_player_id:
                    dest_x = float(player_data[1])
                    dest_y = float(player_data[2])

                if int(player_data[0]) == ID:
                    my_x = float(player_data[1])
                    my_y = float(player_data[2])

        if tactic == 0:
            current_time = time.time()
            if current_time >= sleep_time:
                sleep_time = current_time + random.randint(3, 8)
                x, y = random_unit_vector()
                speed = 0
                s.send(f"{x};{y};{speed};0\r\n".encode())

        if tactic == 1:
            if at_destination(my_x, my_y, dest_x, dest_y):
                tactic = 0

        if tactic == 2:
            x, y = unit_vector_to_coords(dest_x - my_x, dest_y - my_y)
            s.send(f"{x};{y};{speed};0\r\n".encode())


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

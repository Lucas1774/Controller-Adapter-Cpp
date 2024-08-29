from pynput.mouse import Controller
import time

mouse = Controller()

while True:
    position = mouse.position
    print(f"{position[0]}, {position[1]}")
    time.sleep(1)

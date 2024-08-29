from pynput.mouse import Controller
import time
import keyboard

mouse = Controller()
positions = []
row = []

while True:
    position = mouse.position
    if keyboard.is_pressed('r'):
        row.append(position)
        print(f"Saved position: {position}")
    if keyboard.is_pressed('s'):
        positions.append(row)
        print(f"Saved row: {row}")
        row = []
    if keyboard.is_pressed('q'):
        break
    time.sleep(0.5)
with open('positions.txt', 'w') as f:
    f.write("{ \n")
    for row in positions:
        f.write("{")
        for position in row:
            f.write(f"{{{str(position[0])}, {str(position[1])}}}, ")
        f.write("}, \n")
    f.write("};")
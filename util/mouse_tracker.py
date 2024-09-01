from pynput.mouse import Listener as MouseListener, Button
import keyboard

positions = []
row = []


def on_click(x, y, button, pressed):
    """
    - ctrl + left click to save a position
    - ctrl + middle click to save a row
    - ctrl + right click to dump to file and exit
    """
    global row
    global positions
    if pressed and keyboard.is_pressed("ctrl"):
        if button == Button.left:
            row.append((x, y))
            print(f"Saved pos: {(x, y)}")
        elif button == Button.middle:
            positions.append(row)
            print(f"Saved row: {row}")
            row = []
        elif button == Button.right:
            with open("positions.txt", "w") as f:
                f.write("{ \n")
                for row in positions:
                    f.write("{")
                    for position in row:
                        f.write(f"{{{position[0]}, {position[1]}}}, ")
                    f.write("}, \n")
                f.write("};")
            print("Saved file: positions.txt")
            return False


with MouseListener(on_click=on_click) as mouse_listener:
    mouse_listener.join()

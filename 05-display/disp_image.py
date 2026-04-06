import time
import serial
from PIL import Image

def main():
    ser = serial.Serial(port='COM5', baudrate=115200, timeout=1)
    
    image = Image.open('C:/Repositories/pico/mcu/05-display/get.jpg')
    width, height = image.size
    
    ser.write("disp_screen 000000\n".encode('ascii'))
    time.sleep(0.1)
    
    for y in range(height):
        for x in range(width):
            r, g, b = image.getpixel((x, y))
            color = (r << 16) | (g << 8) | b
            ser.write(f"disp_px {x} {y} {color:06X}\n".encode('ascii'))
    
    time.sleep(0.1)
    ser.close()

if __name__ == "__main__":
    main()
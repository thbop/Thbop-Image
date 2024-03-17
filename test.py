import tii
from PIL import Image


pimg = Image.open('cat.png')
pix = pimg.load()

img = tii.Image(pimg.width, pimg.height)

for j in range(pimg.height):
    for i in range(pimg.width):
        img.set(i, j, tii.Color.load(pix[i, j]))

img.export('t.ti')
pimg.close()

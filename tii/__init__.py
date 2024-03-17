
class Color:
    def __init__(self, r=0, g=0, b=0):
        self.r = r
        self.g = g
        self.b = b

    def __eq__(self, other):
        return self.r == other.r and self.g == other.g and self.b == other.g
    def __repr__(self):
        return f'Color({self.r}, {self.g}, {self.b})'
    
    def export(self):
        return self.r.to_bytes(1, 'little') + self.g.to_bytes(1, 'little') + self.b.to_bytes(1, 'little')

    def load(data):
        c = Color()
        c.r = data[0]
        c.g = data[1]
        c.b = data[2]
        return c

class Image:
    def __init__(self, width, height, palette_size=1, palette=[Color(0, 0, 0)], pixels=[]):
        self.width = width
        self.height = height
        self.palette_size = palette_size
        self.palette = palette
        if pixels != []:
            self.pixels = pixels
        else:
            self.pixels = [ 0 for i in range(width*height) ]
    
    def set(self, x, y, color):
        colori = None
        for i, c in enumerate(self.palette):
            if c == color:
                colori = i
        
        if colori != None: # Existing color
            self.pixels[x + y * self.width] = colori
        elif len(self.palette) < 256: # Add color
            colori = len(self.palette)
            self.palette.append(color)
            self.pixels[x + y * self.width] = colori
        # else:
        #     print(f'Could not set pixel ({x}, {y}) to color: {color}!')
    
    def export( self, file_name ):
        data = b'thbop'

        data += self.width.to_bytes(1, 'little')
        data += self.height.to_bytes(1, 'little')

        data += self.palette_size.to_bytes(1, 'little')
        for c in self.palette:
            data += c.export()
        
        for p in self.pixels:
            data += p.to_bytes(1, 'little')

        with open(file_name, 'wb') as f:
            f.write(data)
    





from PIL import Image

colors = {}
image = Image.open('file.jpg')
image.putalpha(255)
image_data = image.load()
height, width = image.size
for y in range(width):
    for x in range(height):
        if not image_data[x, y] in colors:
            colors[image_data[x, y]] = 1
        else:
            colors[image_data[x, y]] += 1

sort_by_value = dict(sorted(colors.items(), key=lambda item: item[1]))

colors = {k: v for k, v in colors.items() if v > 25}
for y in range(width):
    for x in range(height):
        if image_data[x, y] in colors:
            image_data[x, y] = 0, 0, 0, 0
        # else image_data[]
print(colors)
image.save('file.png')


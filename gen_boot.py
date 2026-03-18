from PIL import Image, ImageDraw, ImageFont
import os

def create_boot_image():
    W, H = 240, 240 # Or 360? Wait, the watch screen might be 240. The previous script generated 360x360.
    # Let's generate 240x240 for safety and scale? Actually, let's use 360x360 because gen_bg.py used 360x360.
    W, H = 360, 360
    img = Image.new('RGB', (W, H), (10, 10, 10)) # Very dark grey background
    draw = ImageDraw.Draw(img)

    # Load Fonts
    try:
        font_large = ImageFont.truetype("arial.ttf", 30)
        font_med = ImageFont.truetype("arial.ttf", 20)
    except:
        font_path = "C:/Windows/Fonts/arial.ttf" 
        font_large = ImageFont.truetype(font_path, 30)
        font_med = ImageFont.truetype(font_path, 20)

    # Draw Text
    text1 = "Bienvenue sur le"
    text2 = "KnobTouch !"
    text3 = "Demarrage en cours..."

    # Center text
    bbox1 = draw.textbbox((0, 0), text1, font=font_large)
    w1 = bbox1[2] - bbox1[0]
    draw.text(((W - w1) / 2, 120), text1, font=font_large, fill=(255, 255, 255))

    bbox2 = draw.textbbox((0, 0), text2, font=font_large)
    w2 = bbox2[2] - bbox2[0]
    # neon blue color
    draw.text(((W - w2) / 2, 160), text2, font=font_large, fill=(100, 200, 255))

    bbox3 = draw.textbbox((0, 0), text3, font=font_med)
    w3 = bbox3[2] - bbox3[0]
    draw.text(((W - w3) / 2, 280), text3, font=font_med, fill=(150, 150, 150))

    # Generate C File
    with open("img_boot_bg.c", "w") as f:
        f.write("#include \"lvgl.h\"\n\n")
        f.write("#ifndef LV_ATTRIBUTE_MEM_ALIGN\n#define LV_ATTRIBUTE_MEM_ALIGN\n#endif\n\n")
        
        f.write(f"const LV_ATTRIBUTE_MEM_ALIGN uint8_t img_boot_bg_map[{W*H*2}] = {{\n")
        
        pixels = img.load()
        for y in range(H):
            row_str = "    "
            for x in range(W):
                r, g, b = pixels[x, y]
                # RGB565 Little Endian conversion
                val = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                low = val & 0xFF
                high = (val >> 8) & 0xFF
                row_str += f"0x{low:02x}, 0x{high:02x}, "
            f.write(row_str + "\n")
            
        f.write("};\n\n")
        
        f.write("const lv_img_dsc_t img_boot_bg = {\n")
        f.write("  .header.always_zero = 0,\n")
        f.write(f"  .header.w = {W},\n")
        f.write(f"  .header.h = {H},\n")
        f.write("  .data_size = " + str(W * H * 2) + ",\n")
        f.write("  .header.cf = LV_IMG_CF_TRUE_COLOR,\n")
        f.write("  .data = img_boot_bg_map,\n")
        f.write("};\n")

    img.save("preview_boot.png")
    print("Generated img_boot_bg.c and preview_boot.png")

if __name__ == "__main__":
    create_boot_image()

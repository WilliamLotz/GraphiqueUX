from PIL import Image, ImageDraw, ImageFont
import struct
import os

def create_image():
    # 1. Create Image
    W, H = 360, 360
    img = Image.new('RGB', (W, H), (0, 0, 0)) # Black Background
    draw = ImageDraw.Draw(img)

    # 2. Load Fonts
    # Try to find Arial
    font_path = "arial.ttf"
    try:
        font_large = ImageFont.truetype(font_path, 40) # Very Large Title
        font_med = ImageFont.truetype(font_path, 24)   # Medium Labels
    except:
        # Fallback if arial not found in local dir, try windows
        font_path = "C:/Windows/Fonts/arial.ttf" 
        font_large = ImageFont.truetype(font_path, 40)
        font_med = ImageFont.truetype(font_path, 24)

    # 3. Draw Text
    # Title "INDEX kWh"
    text = "INDEX kWh"
    # Centered horizontally, top margin
    bbox = draw.textbbox((0, 0), text, font=font_large)
    tw = bbox[2] - bbox[0]
    draw.text(((W - tw) / 2, 30), text, font=font_large, fill=(255, 255, 255))

    # Values Labels
    draw.text((80, 130), "BASE:", font=font_med, fill=(200, 200, 200)) # Light Grey for labels
    draw.text((80, 180), "HP:", font=font_med, fill=(200, 200, 200))
    draw.text((80, 230), "HC:", font=font_med, fill=(200, 200, 200))
    
    # 4. Generate C File
    with open("img_index_bg.c", "w") as f:
        f.write("#include \"lvgl.h\"\n\n")
        f.write("#ifndef LV_ATTRIBUTE_MEM_ALIGN\n#define LV_ATTRIBUTE_MEM_ALIGN\n#endif\n\n")
        
        f.write(f"const LV_ATTRIBUTE_MEM_ALIGN uint8_t img_index_bg_map[{W*H*2}] = {{\n")
        
        pixels = img.load()
        data = []
        for y in range(H):
            row_str = "    "
            for x in range(W):
                r, g, b = pixels[x, y]
                # RGB565 conversion
                # R: 5 bits, G: 6 bits, B: 5 bits
                val = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                # Split into 2 bytes (Little Endian: Low byte first)
                low = val & 0xFF
                high = (val >> 8) & 0xFF
                row_str += f"0x{low:02x}, 0x{high:02x}, "
            f.write(row_str + "\n")
            
        f.write("};\n\n")
        
        f.write("const lv_img_dsc_t img_index_bg = {\n")
        f.write("  .header.always_zero = 0,\n")
        f.write(f"  .header.w = {W},\n")
        f.write(f"  .header.h = {H},\n")
        f.write("  .data_size = " + str(W * H * 2) + ",\n")
        f.write("  .header.cf = LV_IMG_CF_TRUE_COLOR,\n")
        f.write("  .data = img_index_bg_map,\n")
        f.write("};\n")

    # Save PNG just for preview
    img.save("preview.png")
    print("Generated img_index_bg.c and preview.png")

if __name__ == "__main__":
    create_image()

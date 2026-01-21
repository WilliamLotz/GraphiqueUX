import os

def clean_conflict(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    new_lines = []
    in_head = False
    in_remote = False
    
    for line in lines:
        if line.startswith('<<<<<<< HEAD'):
            in_head = True
            in_remote = False
            continue
        elif line.startswith('======='):
            in_head = False
            in_remote = True
            continue
        elif line.startswith('>>>>>>>'):
            in_head = False
            in_remote = False
            continue
        
        if in_head:
            new_lines.append(line)
        elif in_remote:
            pass # Skip remote content
        else:
            new_lines.append(line) # Keep common content

    with open(file_path, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)
    print(f"Cleaned {file_path}")

clean_conflict('ui_linky.c')
clean_conflict('lcd_bsp.c')

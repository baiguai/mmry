#!/usr/bin/env python3
"""
Theme Gallery Generator
Generates an HTML gallery preview from JSON theme files in the themes directory.
"""

import json
import os
import sys
from pathlib import Path

def read_theme_file(file_path):
    """Read a theme JSON file and return color data."""
    theme_data = {}
    try:
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()
                if line and '=' in line:
                    key, value = line.split('=', 1)
                    theme_data[key.strip()] = value.strip()
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return None
    return theme_data

def generate_theme_name(filename):
    """Generate a readable name from filename."""
    name = filename.replace('.json', '').replace('_theme', '')
    return ' '.join(word.capitalize() for word in name.split('_'))

def generate_html(themes_dir, output_file):
    """Generate the HTML gallery."""
    themes_dir = Path(themes_dir)
    if not themes_dir.exists():
        print(f"Error: Themes directory {themes_dir} does not exist")
        return False
    
    # Read all theme files
    themes = []
    for json_file in sorted(themes_dir.glob('*.json')):
        theme_data = read_theme_file(json_file)
        if theme_data:
            themes.append({
                'name': generate_theme_name(json_file.name),
                'file': json_file.name,
                'data': theme_data
            })
    
    if not themes:
        print("No theme files found")
        return False
    
    # Generate HTML
    html_content = f'''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Theme Preview Gallery</title>
    <style>
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
        }}
        h1 {{
            text-align: center;
            color: #333;
            margin-bottom: 30px;
        }}
        .theme-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 20px;
        }}
        .theme-card {{
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            transition: transform 0.2s ease;
        }}
        .theme-card:hover {{
            transform: translateY(-2px);
            box-shadow: 0 4px 20px rgba(0,0,0,0.15);
        }}
        .theme-preview {{
            padding: 20px;
            border: 2px solid;
            position: relative;
        }}
        .theme-name {{
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 15px;
        }}
        .sample-text {{
            margin-bottom: 10px;
            line-height: 1.5;
        }}
        .sample-code {{
            font-family: 'Courier New', monospace;
            background: rgba(0,0,0,0.1);
            padding: 10px;
            border-radius: 4px;
            margin: 10px 0;
        }}
        .sample-selection {{
            padding: 2px 4px;
            border-radius: 2px;
        }}
        .theme-info {{
            background: white;
            padding: 15px;
            border-top: 1px solid #eee;
        }}
        .color-info {{
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 8px;
            font-size: 12px;
        }}
        .color-item {{
            display: flex;
            align-items: center;
            gap: 5px;
        }}
        .color-box {{
            width: 16px;
            height: 16px;
            border-radius: 2px;
            border: 1px solid #ddd;
        }}
        .stats {{
            text-align: center;
            margin-bottom: 20px;
            color: #666;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Theme Preview Gallery</h1>
        <div class="stats">Found {len(themes)} themes</div>
        <div class="theme-grid" id="themeGrid"></div>
    </div>

    <script>
        const themes = {json.dumps(themes, indent=12)};

        function createThemeCard(theme) {{
            const card = document.createElement('div');
            card.className = 'theme-card';
            
            const preview = document.createElement('div');
            preview.className = 'theme-preview';
            preview.style.backgroundColor = theme.data.backgroundColor;
            preview.style.color = theme.data.textColor;
            preview.style.borderColor = theme.data.borderColor;
            
            preview.innerHTML = `
                <div class="theme-name">${{theme.name}}</div>
                <div class="sample-text">
                    This is how normal text appears in this theme.
                    The quick brown fox jumps over the lazy dog.
                </div>
                <div class="sample-code">
                    function example() {{<br>
                    &nbsp;&nbsp;return "Hello World";<br>
                    }}<br>
                    <span class="sample-selection" style="background-color: ${{theme.data.selectionColor}}; color: ${{theme.data.textColor}};">selected text</span>
                </div>
                <div class="sample-text">
                    Some <span class="sample-selection" style="background-color: ${{theme.data.selectionColor}}; color: ${{theme.data.textColor}};">highlighted</span> text to show selection color.
                </div>
            `;
            
            const info = document.createElement('div');
            info.className = 'theme-info';
            info.innerHTML = `
                <div style="font-weight: bold; margin-bottom: 8px;">${{theme.file}}</div>
                <div class="color-info">
                    <div class="color-item">
                        <div class="color-box" style="background: ${{theme.data.backgroundColor}}"></div>
                        <span>Background: ${{theme.data.backgroundColor}}</span>
                    </div>
                    <div class="color-item">
                        <div class="color-box" style="background: ${{theme.data.textColor}}"></div>
                        <span>Text: ${{theme.data.textColor}}</span>
                    </div>
                    <div class="color-item">
                        <div class="color-box" style="background: ${{theme.data.selectionColor}}"></div>
                        <span>Selection: ${{theme.data.selectionColor}}</span>
                    </div>
                    <div class="color-item">
                        <div class="color-box" style="background: ${{theme.data.borderColor}}"></div>
                        <span>Border: ${{theme.data.borderColor}}</span>
                    </div>
                </div>
            `;
            
            card.appendChild(preview);
            card.appendChild(info);
            
            return card;
        }}

        function renderThemes() {{
            const grid = document.getElementById('themeGrid');
            themes.forEach(theme => {{
                grid.appendChild(createThemeCard(theme));
            }});
        }}

        // Initialize the page
        renderThemes();
    </script>
</body>
</html>'''
    
    try:
        with open(output_file, 'w') as f:
            f.write(html_content)
        print(f"Generated theme gallery with {len(themes)} themes: {output_file}")
        return True
    except Exception as e:
        print(f"Error writing HTML file: {e}")
        return False

def main():
    """Main function."""
    # Show help if requested
    if len(sys.argv) > 1 and sys.argv[1] in ['-h', '--help']:
        print("Usage: python3 generate_theme_gallery.py [themes_dir] [output_file]")
        print("  themes_dir: Directory containing theme JSON files (default: ./themes or current directory)")
        print("  output_file: HTML output file (default: ./theme_gallery.html)")
        print("")
        print("Examples:")
        print("  python3 generate_theme_gallery.py                    # Use ./themes or current directory")
        print("  python3 generate_theme_gallery.py /path/to/themes   # Specific themes directory")
        print("  python3 generate_theme_gallery.py . gallery.html     # Current dir, custom output")
        sys.exit(0)
    
    # Default to current directory
    current_dir = os.getcwd()
    
    if len(sys.argv) > 1:
        themes_dir = sys.argv[1]
    else:
        # Look for 'themes' directory in current directory
        themes_dir = os.path.join(current_dir, 'themes')
        if not os.path.exists(themes_dir):
            # If no themes subdirectory, use current directory
            themes_dir = current_dir
    
    if len(sys.argv) > 2:
        output_file = sys.argv[2]
    else:
        output_file = os.path.join(current_dir, 'theme_gallery.html')
    
    print(f"Reading themes from: {themes_dir}")
    print(f"Output file: {output_file}")
    
    success = generate_html(themes_dir, output_file)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
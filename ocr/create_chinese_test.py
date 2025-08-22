#!/usr/bin/env python3
"""
Create a simple test image with Chinese text for OCR testing
"""

from PIL import Image, ImageDraw, ImageFont
import sys

def create_chinese_test_image():
    # Create a white background image
    width, height = 800, 400
    image = Image.new('RGB', (width, height), 'white')
    draw = ImageDraw.Draw(image)
    
    # Try to use a system font that supports Chinese, fallback to default if not available
    try:
        # Try to load a system font that supports Chinese
        font_size = 40
        # Common Chinese fonts on macOS
        chinese_fonts = [
            '/System/Library/Fonts/PingFang.ttc',
            '/System/Library/Fonts/STHeiti Light.ttc',
            '/System/Library/Fonts/Hiragino Sans GB.ttc',
            '/Library/Fonts/Arial Unicode MS.ttf'
        ]
        
        font = None
        for font_path in chinese_fonts:
            try:
                font = ImageFont.truetype(font_path, font_size)
                break
            except:
                continue
        
        if font is None:
            # Use default font if no Chinese fonts are available
            font = ImageFont.load_default()
            
    except:
        # Use default font if system fonts are not available
        font = ImageFont.load_default()
    
    # Add Chinese text to the image
    text_lines = [
        "你好，世界！",
        "这是一个中文OCR测试。",
        "光学字符识别技术",
        "数字：1234567890",
        "混合文本：Hello 世界 123"
    ]
    
    y_position = 50
    line_height = 60
    
    for line in text_lines:
        # Calculate text position to center it
        try:
            bbox = draw.textbbox((0, 0), line, font=font)
            text_width = bbox[2] - bbox[0]
        except:
            # Fallback for older PIL versions
            text_width = len(line) * 20  # Rough estimate
        
        x_position = (width - text_width) // 2
        
        # Draw the text
        draw.text((x_position, y_position), line, fill='black', font=font)
        y_position += line_height
    
    # Save the image
    image.save('chinese_test.png')
    print("Chinese test image created: chinese_test.png")

if __name__ == '__main__':
    create_chinese_test_image()
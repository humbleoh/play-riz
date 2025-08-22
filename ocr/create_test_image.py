#!/usr/bin/env python3
"""
Create a simple test image with text for OCR testing
"""

from PIL import Image, ImageDraw, ImageFont
import sys

def create_test_image():
    # Create a white background image
    width, height = 800, 400
    image = Image.new('RGB', (width, height), 'white')
    draw = ImageDraw.Draw(image)
    
    # Try to use a system font, fallback to default if not available
    try:
        # Try to load a system font
        font_size = 40
        font = ImageFont.truetype('/System/Library/Fonts/Arial.ttf', font_size)
    except:
        try:
            font = ImageFont.truetype('/System/Library/Fonts/Helvetica.ttc', font_size)
        except:
            # Use default font if system fonts are not available
            font = ImageFont.load_default()
    
    # Add some text to the image
    text_lines = [
        "Hello, World!",
        "This is a test image for OCR.",
        "The quick brown fox jumps over the lazy dog.",
        "Numbers: 1234567890",
        "Special chars: !@#$%^&*()"
    ]
    
    y_position = 50
    line_height = 60
    
    for line in text_lines:
        # Calculate text position to center it
        bbox = draw.textbbox((0, 0), line, font=font)
        text_width = bbox[2] - bbox[0]
        x_position = (width - text_width) // 2
        
        # Draw the text
        draw.text((x_position, y_position), line, fill='black', font=font)
        y_position += line_height
    
    # Save the image
    image.save('test_image.png')
    print("Test image created: test_image.png")

if __name__ == '__main__':
    create_test_image()
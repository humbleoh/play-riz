#!/usr/bin/env python3
"""
Simple OCR CLI Tool
A command-line interface for optical character recognition using Tesseract OCR.
"""

import argparse
import sys
import os
import json
from pathlib import Path

try:
    import pytesseract
    from PIL import Image
    import cv2
    import numpy as np
except ImportError as e:
    print(f"Error: Missing required dependency: {e}")
    print("Please install dependencies with: pip install -r requirements.txt")
    sys.exit(1)


class OCRProcessor:
    """Main OCR processing class"""
    
    def __init__(self):
        self.supported_formats = ['.png', '.jpg', '.jpeg', '.tiff', '.bmp', '.gif']
    
    def preprocess_image(self, image_path, enhance=False):
        """Preprocess image for better OCR results"""
        try:
            # Read image using OpenCV
            img = cv2.imread(image_path)
            if img is None:
                raise ValueError(f"Could not read image: {image_path}")
            
            if enhance:
                # Convert to grayscale
                gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
                
                # Apply denoising
                denoised = cv2.fastNlMeansDenoising(gray)
                
                # Apply threshold to get better contrast
                _, thresh = cv2.threshold(denoised, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
                
                return thresh
            else:
                # Convert to RGB for PIL
                return cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
                
        except Exception as e:
            raise Exception(f"Error preprocessing image: {e}")
    
    def extract_text(self, image_path, lang='eng', enhance=False, config=''):
        """Extract text from image using OCR"""
        try:
            # Validate file exists and format
            if not os.path.exists(image_path):
                raise FileNotFoundError(f"Image file not found: {image_path}")
            
            file_ext = Path(image_path).suffix.lower()
            if file_ext not in self.supported_formats:
                raise ValueError(f"Unsupported file format: {file_ext}")
            
            # Preprocess image
            processed_img = self.preprocess_image(image_path, enhance)
            
            # Convert numpy array to PIL Image if needed
            if isinstance(processed_img, np.ndarray):
                pil_img = Image.fromarray(processed_img)
            else:
                pil_img = Image.fromarray(processed_img)
            
            # Perform OCR
            custom_config = config if config else r'--oem 3 --psm 6'
            text = pytesseract.image_to_string(pil_img, lang=lang, config=custom_config)
            
            return text.strip()
            
        except Exception as e:
            raise Exception(f"OCR extraction failed: {e}")
    
    def get_text_data(self, image_path, lang='eng', enhance=False):
        """Get detailed text data including bounding boxes"""
        try:
            processed_img = self.preprocess_image(image_path, enhance)
            
            if isinstance(processed_img, np.ndarray):
                pil_img = Image.fromarray(processed_img)
            else:
                pil_img = Image.fromarray(processed_img)
            
            # Get detailed data
            data = pytesseract.image_to_data(pil_img, lang=lang, output_type=pytesseract.Output.DICT)
            
            return data
            
        except Exception as e:
            raise Exception(f"Failed to get text data: {e}")


def main():
    """Main CLI function"""
    parser = argparse.ArgumentParser(
        description='Simple OCR CLI Tool - Extract text from images using Tesseract OCR',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s image.png                    # Basic OCR
  %(prog)s image.jpg -l chi_sim         # OCR with Chinese language
  %(prog)s image.png -e -o json         # Enhanced preprocessing with JSON output
  %(prog)s image.png -c '--psm 8'       # Custom Tesseract config
        """
    )
    
    parser.add_argument('image', help='Path to the image file')
    parser.add_argument('-l', '--lang', default='eng', 
                       help='Language for OCR (default: eng). Use + for multiple languages (e.g., eng+fra)')
    parser.add_argument('-o', '--output', choices=['text', 'json'], default='text',
                       help='Output format (default: text)')
    parser.add_argument('-e', '--enhance', action='store_true',
                       help='Apply image enhancement for better OCR results')
    parser.add_argument('-c', '--config', default='',
                       help='Custom Tesseract configuration string')
    parser.add_argument('-f', '--file', 
                       help='Save output to file instead of printing to stdout')
    parser.add_argument('--detailed', action='store_true',
                       help='Include bounding box information (only with JSON output)')
    
    args = parser.parse_args()
    
    # Initialize OCR processor
    ocr = OCRProcessor()
    
    try:
        if args.detailed and args.output == 'json':
            # Get detailed data with bounding boxes
            data = ocr.get_text_data(args.image, args.lang, args.enhance)
            result = {
                'text': ' '.join([word for word in data['text'] if word.strip()]),
                'detailed_data': data
            }
            output = json.dumps(result, indent=2, ensure_ascii=False)
        else:
            # Extract text
            text = ocr.extract_text(args.image, args.lang, args.enhance, args.config)
            
            if args.output == 'json':
                result = {
                    'image': args.image,
                    'language': args.lang,
                    'enhanced': args.enhance,
                    'text': text
                }
                output = json.dumps(result, indent=2, ensure_ascii=False)
            else:
                output = text
        
        # Output result
        if args.file:
            with open(args.file, 'w', encoding='utf-8') as f:
                f.write(output)
            print(f"Output saved to: {args.file}")
        else:
            print(output)
            
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
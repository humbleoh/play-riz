# Simple OCR CLI Tool

A command-line interface for optical character recognition (OCR) using Tesseract OCR. This tool can extract text from images in various formats and output the results in plain text or JSON format.

## Features

- **Multiple Image Formats**: Supports PNG, JPG, JPEG, TIFF, BMP, and GIF
- **Language Support**: OCR in multiple languages (default: English)
- **Image Enhancement**: Optional preprocessing for better OCR accuracy
- **Output Formats**: Plain text or structured JSON output
- **Detailed Analysis**: Optional bounding box information for text elements
- **File Output**: Save results to file instead of stdout
- **Custom Configuration**: Support for custom Tesseract configuration strings

## Prerequisites

### System Requirements

1. **Tesseract OCR Engine**: Install using Homebrew (macOS)
   ```bash
   brew install tesseract
   ```

2. **Python 3.8+**: Make sure you have Python 3.8 or higher installed

### Python Dependencies

Install the required Python packages:

```bash
pip install -r requirements.txt
```

Or install manually:

```bash
pip install pytesseract Pillow opencv-python numpy
```

## Installation

1. Clone or download this repository
2. Install system dependencies (Tesseract)
3. Install Python dependencies
4. Make the script executable:
   ```bash
   chmod +x ocr_cli.py
   ```

## Usage

### Basic Usage

```bash
python3 ocr_cli.py image.png
```

### Command Line Options

```
usage: ocr_cli.py [-h] [-l LANG] [-o {text,json}] [-e] [-c CONFIG] [-f FILE] [--detailed] image

Simple OCR CLI Tool - Extract text from images using Tesseract OCR

positional arguments:
  image                 Path to the image file

optional arguments:
  -h, --help            show this help message and exit
  -l LANG, --lang LANG  Language for OCR (default: eng). Use + for multiple languages (e.g., eng+fra)
  -o {text,json}, --output {text,json}
                        Output format (default: text)
  -e, --enhance         Apply image enhancement for better OCR results
  -c CONFIG, --config CONFIG
                        Custom Tesseract configuration string
  -f FILE, --file FILE  Save output to file instead of printing to stdout
  --detailed            Include bounding box information (only with JSON output)
```

### Examples

#### Basic OCR
```bash
python3 ocr_cli.py image.png
```

#### OCR with Different Language
```bash
# Chinese Simplified
python3 ocr_cli.py image.jpg -l chi_sim

# Multiple languages
python3 ocr_cli.py image.png -l eng+fra
```

#### Enhanced Image Processing
```bash
python3 ocr_cli.py image.png -e
```

#### JSON Output
```bash
python3 ocr_cli.py image.png -o json
```

#### Save to File
```bash
python3 ocr_cli.py image.png -f output.txt
python3 ocr_cli.py image.png -o json -f output.json
```

#### Detailed Analysis (with bounding boxes)
```bash
python3 ocr_cli.py image.png -o json --detailed
```

#### Custom Tesseract Configuration
```bash
# Single character recognition
python3 ocr_cli.py image.png -c '--psm 8'

# Single word recognition
python3 ocr_cli.py image.png -c '--psm 7'
```

## Supported Languages

The tool supports all languages that Tesseract OCR supports. Common language codes include:

- `eng` - English (default)
- `chi_sim` - Chinese Simplified
- `chi_tra` - Chinese Traditional
- `fra` - French
- `deu` - German
- `spa` - Spanish
- `jpn` - Japanese
- `kor` - Korean
- `rus` - Russian
- `ara` - Arabic

To see all available languages on your system:
```bash
tesseract --list-langs
```

## Image Enhancement

When using the `-e` or `--enhance` flag, the tool applies several preprocessing steps:

1. **Grayscale Conversion**: Converts the image to grayscale
2. **Denoising**: Removes noise from the image
3. **Thresholding**: Applies binary thresholding for better contrast

This can significantly improve OCR accuracy for low-quality images.

## Output Formats

### Text Output (default)
Plain text extracted from the image.

### JSON Output
Structured JSON containing:
- `image`: Input image path
- `language`: Language used for OCR
- `enhanced`: Whether image enhancement was applied
- `text`: Extracted text
- `detailed_data`: (with --detailed flag) Bounding box information

## Testing

A test image generator is included to help you test the OCR functionality:

```bash
python3 create_test_image.py
python3 ocr_cli.py test_image.png
```

## Troubleshooting

### Common Issues

1. **"tesseract: command not found"**
   - Install Tesseract: `brew install tesseract`

2. **"No module named 'pytesseract'"**
   - Install Python dependencies: `pip install -r requirements.txt`

3. **Poor OCR accuracy**
   - Try using the `-e` flag for image enhancement
   - Experiment with different Tesseract PSM modes using `-c` flag
   - Ensure the image has good contrast and resolution

4. **Language not recognized**
   - Check available languages: `tesseract --list-langs`
   - Install additional language packs if needed

### Performance Tips

- Use high-resolution images for better accuracy
- Ensure good contrast between text and background
- Use the enhancement option for scanned documents
- Experiment with different PSM (Page Segmentation Mode) settings

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit issues and enhancement requests.
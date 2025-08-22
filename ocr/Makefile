# OCR CLI Tool Makefile
# This Makefile provides targets for building, testing, and packaging the OCR CLI application

# Variables
APP_NAME = ocr-cli
PYTHON = python3
PIP = pip3
PYINSTALLER = pyinstaller
SOURCE_FILE = ocr_cli.py
SPEC_FILE = $(APP_NAME).spec
DIST_DIR = dist
BUILD_DIR = build
VENV_DIR = venv
REQUIREMENTS = requirements.txt
DEV_REQUIREMENTS = requirements-dev.txt

# Default target
.PHONY: all
all: help

# Help target
.PHONY: help
help:
	@echo "OCR CLI Tool Build System"
	@echo "========================"
	@echo ""
	@echo "Available targets:"
	@echo "  help          - Show this help message"
	@echo "  install       - Install dependencies"
	@echo "  install-dev   - Install development dependencies"
	@echo "  venv          - Create virtual environment"
	@echo "  test          - Run basic functionality tests"
	@echo "  build         - Build standalone executable"
	@echo "  build-onefile - Build single-file executable"
	@echo "  build-dir     - Build directory-based executable"
	@echo "  package       - Create distribution package"
	@echo "  clean         - Clean build artifacts"
	@echo "  clean-all     - Clean everything including venv"
	@echo "  run           - Run the application directly"
	@echo "  run-built     - Run the built executable"
	@echo ""
	@echo "Build options:"
	@echo "  The build process creates a standalone executable that:"
	@echo "  - Bundles all Python dependencies"
	@echo "  - Protects source code from inspection"
	@echo "  - Runs without requiring Python installation"

# Install dependencies
.PHONY: install
install:
	@echo "Installing dependencies..."
	$(PIP) install -r $(REQUIREMENTS)
	@echo "Dependencies installed successfully!"

# Install development dependencies
.PHONY: install-dev
install-dev: install
	@echo "Installing development dependencies..."
	$(PIP) install pyinstaller>=5.0 pytest>=6.0 black>=22.0 flake8>=4.0
	@echo "Development dependencies installed successfully!"

# Create virtual environment
.PHONY: venv
venv:
	@echo "Creating virtual environment..."
	$(PYTHON) -m venv $(VENV_DIR)
	@echo "Virtual environment created in $(VENV_DIR)/"
	@echo "Activate it with: source $(VENV_DIR)/bin/activate"

# Test the application
.PHONY: test
test:
	@echo "Running basic functionality tests..."
	@echo "Testing help command..."
	$(PYTHON) $(SOURCE_FILE) --help
	@echo "Testing with sample image..."
	@if [ -f "test_image.png" ]; then \
		$(PYTHON) $(SOURCE_FILE) test_image.png; \
	else \
		echo "Creating test image first..."; \
		$(PYTHON) create_test_image.py; \
		$(PYTHON) $(SOURCE_FILE) test_image.png; \
	fi
	@echo "Basic tests completed successfully!"

# Generate PyInstaller spec file
$(SPEC_FILE):
	@echo "Generating PyInstaller spec file..."
	$(PYINSTALLER) --name=$(APP_NAME) \
		--onefile \
		--console \
		--add-data="README.md:." \
		--hidden-import=PIL \
		--hidden-import=PIL._tkinter_finder \
		--hidden-import=cv2 \
		--hidden-import=numpy \
		--hidden-import=pytesseract \
		--collect-all=PIL \
		--collect-all=cv2 \
		--noconfirm \
		$(SOURCE_FILE)
	@echo "Spec file generated: $(SPEC_FILE)"

# Build standalone executable (default: onefile)
.PHONY: build
build: build-onefile

# Build single-file executable
.PHONY: build-onefile
build-onefile: install-dev $(SPEC_FILE)
	@echo "Building single-file executable..."
	@echo "This may take several minutes..."
	$(PYINSTALLER) --clean --noconfirm $(SPEC_FILE)
	@echo "Build completed!"
	@echo "Executable created: $(DIST_DIR)/$(APP_NAME)"
	@ls -la $(DIST_DIR)/

# Build directory-based executable
.PHONY: build-dir
build-dir: install-dev
	@echo "Building directory-based executable..."
	$(PYINSTALLER) --name=$(APP_NAME) \
		--onedir \
		--console \
		--add-data="README.md:."\
		--hidden-import=PIL \
		--hidden-import=cv2 \
		--hidden-import=numpy \
		--hidden-import=pytesseract \
		--collect-all=PIL \
		--collect-all=cv2 \
		--noconfirm \
		--clean \
		$(SOURCE_FILE)
	@echo "Directory-based build completed!"
	@echo "Executable directory: $(DIST_DIR)/$(APP_NAME)/"
	@ls -la $(DIST_DIR)/$(APP_NAME)/

# Create distribution package
.PHONY: package
package: build
	@echo "Creating distribution package..."
	@mkdir -p package/$(APP_NAME)
	@cp $(DIST_DIR)/$(APP_NAME) package/$(APP_NAME)/
	@cp README.md package/$(APP_NAME)/
	@cp -r images package/$(APP_NAME)/ 2>/dev/null || true
	@echo "#!/bin/bash" > package/$(APP_NAME)/install.sh
	@echo "# OCR CLI Installation Script" >> package/$(APP_NAME)/install.sh
	@echo "echo 'Installing OCR CLI to /usr/local/bin/'" >> package/$(APP_NAME)/install.sh
	@echo "sudo cp $(APP_NAME) /usr/local/bin/" >> package/$(APP_NAME)/install.sh
	@echo "sudo chmod +x /usr/local/bin/$(APP_NAME)" >> package/$(APP_NAME)/install.sh
	@echo "echo 'Installation completed! You can now use: $(APP_NAME) <image>'" >> package/$(APP_NAME)/install.sh
	@chmod +x package/$(APP_NAME)/install.sh
	@cd package && tar -czf $(APP_NAME)-$(shell uname -s)-$(shell uname -m).tar.gz $(APP_NAME)/
	@echo "Distribution package created: package/$(APP_NAME)-$(shell uname -s)-$(shell uname -m).tar.gz"
	@ls -la package/

# Run the application directly
.PHONY: run
run:
	@echo "Running OCR CLI directly..."
	@if [ -f "test_image.png" ]; then \
		$(PYTHON) $(SOURCE_FILE) test_image.png; \
	else \
		echo "No test image found. Use: make run IMAGE=<path_to_image>"; \
	fi

# Run the built executable
.PHONY: run-built
run-built:
	@echo "Running built executable..."
	@if [ -f "$(DIST_DIR)/$(APP_NAME)" ]; then \
		if [ -f "test_image.png" ]; then \
			$(DIST_DIR)/$(APP_NAME) test_image.png; \
		else \
			echo "No test image found. Use: $(DIST_DIR)/$(APP_NAME) <image_path>"; \
		fi; \
	else \
		echo "Executable not found. Run 'make build' first."; \
	fi

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(DIST_DIR)
	@rm -rf *.spec
	@rm -rf __pycache__
	@rm -rf *.pyc
	@rm -rf .pytest_cache
	@rm -rf package
	@echo "Build artifacts cleaned!"

# Clean everything including virtual environment
.PHONY: clean-all
clean-all: clean
	@echo "Cleaning virtual environment..."
	@rm -rf $(VENV_DIR)
	@echo "Everything cleaned!"

# Check if required tools are installed
.PHONY: check-deps
check-deps:
	@echo "Checking dependencies..."
	@command -v $(PYTHON) >/dev/null 2>&1 || { echo "Python 3 is required but not installed."; exit 1; }
	@command -v tesseract >/dev/null 2>&1 || { echo "Tesseract OCR is required but not installed. Run: brew install tesseract"; exit 1; }
	@echo "All required tools are available!"

# Show build information
.PHONY: info
info:
	@echo "Build Information:"
	@echo "================="
	@echo "Python: $(shell $(PYTHON) --version)"
	@echo "Platform: $(shell uname -s) $(shell uname -m)"
	@echo "Tesseract: $(shell tesseract --version | head -n1)"
	@echo "PyInstaller: $(shell $(PYINSTALLER) --version 2>/dev/null || echo 'Not installed')"
	@echo "Source file: $(SOURCE_FILE)"
	@echo "App name: $(APP_NAME)"
	@echo "Dist directory: $(DIST_DIR)"

# Development workflow
.PHONY: dev
dev: install-dev test
	@echo "Development environment ready!"

# Quick build and test
.PHONY: quick
quick: build run-built
	@echo "Quick build and test completed!"
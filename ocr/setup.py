#!/usr/bin/env python3
"""
Setup script for OCR CLI Tool
"""

from setuptools import setup, find_packages
import os

# Read the contents of README file
this_directory = os.path.abspath(os.path.dirname(__file__))
with open(os.path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

# Read requirements
with open('requirements.txt') as f:
    requirements = f.read().splitlines()

setup(
    name='ocr-cli',
    version='1.0.0',
    author='OCR CLI Developer',
    author_email='developer@example.com',
    description='A simple command-line interface for optical character recognition using Tesseract OCR',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/example/ocr-cli',
    packages=find_packages(),
    py_modules=['ocr_cli'],
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Intended Audience :: End Users/Desktop',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Topic :: Multimedia :: Graphics :: Capture :: Scanners',
        'Topic :: Scientific/Engineering :: Image Recognition',
        'Topic :: Text Processing',
        'Topic :: Utilities',
    ],
    python_requires='>=3.8',
    install_requires=requirements,
    extras_require={
        'dev': [
            'pyinstaller>=5.0',
            'pytest>=6.0',
            'black>=22.0',
            'flake8>=4.0',
        ],
    },
    entry_points={
        'console_scripts': [
            'ocr-cli=ocr_cli:main',
        ],
    },
    include_package_data=True,
    package_data={
        '': ['*.md', '*.txt', '*.png'],
    },
    keywords='ocr tesseract cli image text recognition optical character',
    project_urls={
        'Bug Reports': 'https://github.com/example/ocr-cli/issues',
        'Source': 'https://github.com/example/ocr-cli',
        'Documentation': 'https://github.com/example/ocr-cli/blob/main/README.md',
    },
)
# BarcodeReaderAndWriter

A complete barcode reading and generation tool built using the open-source ZXing (Zebra Crossing) Library C++ Port with a QML-based user interface.

![WhatsApp Image 2025-03-10 at 01 55 12_e37746fa](https://github.com/user-attachments/assets/57322de1-c045-4434-9067-ede2f5da0345)

## Overview

This application provides a comprehensive solution for both reading and generating various barcode formats. It leverages the powerful ZXing C++ port to ensure high accuracy in barcode detection and creation while offering an intuitive QML-based interface.

## Features

- **Barcode Reading**:
  - Scan barcodes from image files
  - Real-time scanning from webcam/camera input
  - Support for multiple barcode formats
  - Error correction and handling for damaged barcodes

- **Barcode Generation**:
  - Create barcodes from user-provided text/data
  - Export generated barcodes as PNG/SVG images
  - Customize barcode size and error correction level
  - Support for multiple barcode formats

- **Supported Barcode Formats**:
  - QR Code
  - Data Matrix
  - Aztec
  - PDF417
  - Code 128
  - Code 39
  - EAN-13/UPC-A
  - EAN-8
  - UPC-E
  - And more...

## Requirements

- C++17 compatible compiler
- Qt 6.0+ with QML support
- CMake 3.15+
- The ZXing-C++ library
- A camera device (for live scanning)

## Building from Source

### Prerequisites

Make sure you have the required development tools installed:

```bash
# Ubuntu/Debian
sudo apt-get install build-essential qt5-default qtdeclarative5-dev cmake

# Fedora
sudo dnf install gcc-c++ qt5-qtbase-devel qt5-qtdeclarative-devel cmake

# macOS (using Homebrew)
brew install qt cmake

# Windows
# Install Qt through the Qt installer, and CMake from their website
```

### Build Steps

1. Clone the repository:
```bash
git clone https://github.com/GilbertAshivaka/BarcodeReaderAndWriter.git
cd BarcodeReaderAndWriter
```

2. Generate the build files with CMake:
```bash
mkdir build
cd build
cmake ..
```

3. Build the project:
```bash
make -j4  # or use 'cmake --build .' on Windows
```

4. Run the application:
```bash
./BarcodeReaderAndWriter
```

## Project Structure

- `barcodereader.cpp/h` - Barcode reading functionality
- `barcodewriter.cpp/h` - Barcode generation functionality
- `main.cpp` - Application entry point
- `Main.qml` - Main QML UI definition
- `ReadBarcode.qml` - QML for barcode reading interface
- `BarcodeWriter.qml` - QML for barcode generation interface
- `CMakeLists.txt` - CMake build configuration

## Usage

### Reading Barcodes

1. Launch the application and select the "Read" tab
2. Choose between file input or camera input
3. For file input: Select an image containing a barcode
4. For camera input: Point the camera at a barcode
5. View the detected barcode content in the results area

### Generating Barcodes

1. Select the "Generate" tab
2. Enter the text/data to encode
3. Select the desired barcode format
4. Adjust size and error correction settings if needed
5. Click "Generate" to create the barcode
6. Save the generated barcode using the "Save" button

## Acknowledgments

- [ZXing](https://github.com/zxing/zxing) - The original Zebra Crossing barcode processing library
- [ZXing-C++](https://github.com/nu-book/zxing-cpp) - The C++ port of ZXing used in this project
- Qt and QML for providing the cross-platform UI framework

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Contact

Gilbert Ashivaka - [GitHub Profile](https://github.com/GilbertAshivaka)

Project Link: [https://github.com/GilbertAshivaka/BarcodeReaderAndWriter](https://github.com/GilbertAshivaka/BarcodeReaderAndWriter)

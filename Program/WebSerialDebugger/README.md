# Web Serial Debugger

A cross-platform web-based serial port debugger with a clean and modern interface.

## Features

- 🔌 **Serial Port Connection**: Connect to serial ports with customizable baud rate, data bits, parity, and stop bits
- 📊 **Real-time Monitoring**: Display received data in real-time with timestamp
- 💾 **Data Logging**: Save received data to file
- 🎨 **Clean UI**: Modern, responsive interface with dark/light mode support
- 🌐 **Cross-platform**: Works on Linux, Windows, and macOS (with compatible browsers)
- ⌨️ **Keyboard Shortcuts**: Send data with Enter key, quick commands
- 📋 **Hex/ASCII Display**: Support both ASCII and hexadecimal display modes

## Requirements

- Python 3.7+
- Modern browser with Web Serial API support (Chrome/Edge 89+)

## Installation

```bash
# Clone or download the project
cd WebSerialDebugger

# Install dependencies
pip install -r requirements.txt
```

## Usage

### Start the Server

```bash
python app.py
```

Then open your browser and navigate to: `http://localhost:5000`

### Connect to Serial Port

1. Click "Connect" button to open serial port selection
2. Select your serial port from the list
3. Configure baud rate and other parameters
4. Click "Open" to start communication

### Send Data

- Type data in the input box
- Press Enter or click "Send" button
- Use "Send with Newline" for line-terminated commands

### Receive Data

- Data appears in the terminal area in real-time
- Toggle "Show Timestamp" to add timestamps
- Toggle "Hex Mode" to display data in hexadecimal
- Click "Clear" to clear the terminal
- Click "Save" to save data to file

## Browser Compatibility

| Browser | Version | Support |
|---------|---------|---------|
| Chrome  | 89+     | ✅ Full |
| Edge    | 89+     | ✅ Full |
| Firefox | -       | ❌ No   |
| Safari  | -       | ❌ No   |

## Configuration

Default serial port settings:
- Baud Rate: 115200
- Data Bits: 8
- Parity: None
- Stop Bits: 1
- Flow Control: None

## Project Structure

```
WebSerialDebugger/
├── app.py              # Flask backend server
├── requirements.txt    # Python dependencies
├── README.md          # This file
├── static/            # Static assets
│   ├── css/
│   │   └── style.css  # Stylesheet
│   └── js/
│       └── main.js    # Frontend JavaScript
└── templates/
    └── index.html     # Main HTML template
```

## License

MIT License

## Author

Created for cross-platform serial debugging needs.

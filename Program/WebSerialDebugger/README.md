# Web Serial Debugger

A cross-platform web-based serial port debugger with a clean and modern interface.

## Features

- 🔌 **Serial Port Connection**: Connect to serial ports with customizable baud rate, data bits, parity, and stop bits
- 📊 **Real-time Monitoring**: Display received data in real-time with millisecond-precision timestamp
- 💾 **Data Logging**: Save received data to file
- 📁 **File-based Transmission**: Send data from file with multiple modes and configurable timing
- 🎨 **Clean UI**: Modern, responsive interface with dark theme
- 🌐 **Cross-platform**: Works on Linux, Windows, and macOS (with compatible browsers)
- ⌨️ **Keyboard Shortcuts**: Send data with Enter key, quick commands
- 📋 **Hex/ASCII Display**: Support both ASCII and hexadecimal display modes
- 🔄 **Frame Buffering**: Delimiter-based frame reassembly prevents split data frames

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
- Toggle "Show Timestamp" to add timestamps (HH:MM:SS.mmm format)
- Toggle "Hex Mode" to display data in hexadecimal
- Click "Clear" to clear the terminal
- Click "Save" to save data to file
- **Frame Buffering**: Data is automatically reassembled using `\n` delimiter to prevent split frames

### Send from File

Send data from a file with advanced controls:

1. **Select File**: Choose a data file (.txt, .bin, .hex, .csv)
2. **Format**: 
   - **ASCII**: Send file content as-is (`,` and `;` are treated as delimiters and skipped)
   - **Hex**: Parse hex values (supports `0x01` or `01 FF` format)
3. **Interval**: Set delay between bytes (1-1000ms)
4. **Mode**:
   - **OneShot**: Send once and stop
   - **Continuous**: Loop repeatedly (1,2,3,1,2,3...)
   - **PingPong**: Bounce back and forth (1,2,3,2,1,2...)
5. **Controls**:
   - **Start**: Begin sending from the beginning
   - **Pause**: Pause at current position
   - **Continue**: Resume from paused position
   - **Stop**: Stop and reset to beginning

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
├── app.py                   # Flask backend server
├── requirements.txt         # Python dependencies
├── README.md               # This file
├── todolist.md             # Feature roadmap
├── test_frame_buffer.py    # Frame buffering tests
├── test_file_send.py       # File send functionality tests
├── static/                 # Static assets
│   ├── css/
│   │   └── style.css       # Stylesheet
│   └── js/
│       └── main.js         # Frontend JavaScript
└── templates/
    └── index.html          # Main HTML template
```

## Testing

Run the test suites to verify functionality:

```bash
# Test frame buffering logic
python test_frame_buffer.py

# Test file send parsing
python test_file_send.py
```

## License

MIT License

## Author

Created for cross-platform serial debugging needs.

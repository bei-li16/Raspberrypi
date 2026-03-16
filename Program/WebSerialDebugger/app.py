#!/usr/bin/env python3
"""
Web Serial Debugger - Flask Backend
A cross-platform web-based serial port debugger
"""

import json
import serial
import serial.tools.list_ports
from flask import Flask, render_template, jsonify, request
from flask_cors import CORS
import threading
import time

app = Flask(__name__)
CORS(app)

# Global serial connection
serial_conn = None
serial_thread = None
is_reading = False
received_data_buffer = []

# Frame buffering - delimiter based
frame_buffer = bytearray()
FRAME_DELIMITER = b'\n'  # LF as frame delimiter


def get_serial_ports():
    """Get list of available serial ports"""
    ports = []
    for port in serial.tools.list_ports.comports():
        ports.append({
            'device': port.device,
            'description': port.description,
            'hwid': port.hwid
        })
    return ports


def read_serial_data():
    """Background thread to read serial data with delimiter-based frame buffering"""
    global serial_conn, is_reading, received_data_buffer, frame_buffer
    
    while is_reading and serial_conn and serial_conn.is_open:
        try:
            # Read any available data into frame buffer
            if serial_conn.in_waiting > 0:
                data = serial_conn.read(serial_conn.in_waiting)
                frame_buffer.extend(data)
            
            # Process complete frames (split by delimiter)
            while FRAME_DELIMITER in frame_buffer:
                # Split at first delimiter
                frame_end = frame_buffer.index(FRAME_DELIMITER)
                frame_data = frame_buffer[:frame_end]  # Exclude delimiter
                
                timestamp = time.strftime('%H:%M:%S') + f'.{int(time.time() * 1000) % 1000:03d}'
                received_data_buffer.append({
                    'timestamp': timestamp,
                    'data': frame_data.hex(),
                    'ascii': frame_data.decode('utf-8', errors='replace')
                })
                
                # Keep remaining data (after delimiter)
                frame_buffer = frame_buffer[frame_end + 1:]
                
        except Exception as e:
            print(f"Error reading serial: {e}")
            break
        time.sleep(0.005)  # 5ms poll interval for lower latency


@app.route('/')
def index():
    """Main page"""
    return render_template('index.html')


@app.route('/api/ports')
def list_ports():
    """API: List available serial ports"""
    return jsonify({
        'success': True,
        'ports': get_serial_ports()
    })


@app.route('/api/connect', methods=['POST'])
def connect_port():
    """API: Connect to serial port"""
    global serial_conn, serial_thread, is_reading
    
    try:
        data = request.json
        port = data.get('port')
        baudrate = int(data.get('baudrate', 115200))
        bytesize = int(data.get('bytesize', 8))
        parity = data.get('parity', 'N')
        stopbits = int(data.get('stopbits', 1))
        
        # Close existing connection
        if serial_conn and serial_conn.is_open:
            is_reading = False
            serial_conn.close()
        
        # Open new connection
        serial_conn = serial.Serial(
            port=port,
            baudrate=baudrate,
            bytesize=bytesize,
            parity=parity,
            stopbits=stopbits,
            timeout=0.1
        )
        
        # Start reading thread
        is_reading = True
        serial_thread = threading.Thread(target=read_serial_data)
        serial_thread.daemon = True
        serial_thread.start()
        
        return jsonify({
            'success': True,
            'message': f'Connected to {port} at {baudrate} baud'
        })
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        }), 400


@app.route('/api/disconnect', methods=['POST'])
def disconnect_port():
    """API: Disconnect from serial port"""
    global serial_conn, is_reading, frame_buffer
    
    try:
        is_reading = False
        # Flush any remaining incomplete frame data
        if len(frame_buffer) > 0:
            timestamp = time.strftime('%H:%M:%S') + f'.{int(time.time() * 1000) % 1000:03d}'
            received_data_buffer.append({
                'timestamp': timestamp,
                'data': frame_buffer.hex(),
                'ascii': frame_buffer.decode('utf-8', errors='replace') + ' [incomplete]'
            })
            frame_buffer = bytearray()
        
        if serial_conn and serial_conn.is_open:
            serial_conn.close()
            serial_conn = None
        
        return jsonify({
            'success': True,
            'message': 'Disconnected'
        })
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        }), 400


@app.route('/api/send', methods=['POST'])
def send_data():
    """API: Send data to serial port"""
    global serial_conn
    
    try:
        if not serial_conn or not serial_conn.is_open:
            return jsonify({
                'success': False,
                'error': 'Not connected'
            }), 400
        
        data = request.json
        text = data.get('text', '')
        hex_mode = data.get('hex', False)
        add_newline = data.get('newline', False)
        
        if hex_mode:
            # Send as hex
            text = text.replace(' ', '')
            data_bytes = bytes.fromhex(text)
        else:
            # Send as ASCII
            data_bytes = text.encode('utf-8')
        
        if add_newline:
            data_bytes += b'\n'
        
        serial_conn.write(data_bytes)
        
        return jsonify({
            'success': True,
            'bytes_sent': len(data_bytes)
        })
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        }), 400


@app.route('/api/receive')
def receive_data():
    """API: Get received data"""
    global received_data_buffer
    
    data = received_data_buffer.copy()
    received_data_buffer = []  # Clear buffer
    
    return jsonify({
        'success': True,
        'data': data
    })


@app.route('/api/status')
def get_status():
    """API: Get connection status"""
    global serial_conn
    
    is_connected = serial_conn is not None and serial_conn.is_open
    
    return jsonify({
        'success': True,
        'connected': is_connected,
        'port': serial_conn.port if is_connected else None,
        'baudrate': serial_conn.baudrate if is_connected else None
    })


if __name__ == '__main__':
    print("Starting Web Serial Debugger...")
    print("Open your browser and navigate to: http://localhost:5000")
    app.run(host='0.0.0.0', port=5000, debug=False)

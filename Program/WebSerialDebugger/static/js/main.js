/**
 * Web Serial Debugger - Frontend JavaScript
 */

// Global state
let isConnected = false;
let rxBytes = 0;
let txBytes = 0;
let pollInterval = null;

// DOM Elements
const elements = {
    port: document.getElementById('port'),
    baudrate: document.getElementById('baudrate'),
    databits: document.getElementById('databits'),
    parity: document.getElementById('parity'),
    stopbits: document.getElementById('stopbits'),
    refreshPorts: document.getElementById('refresh-ports'),
    connectBtn: document.getElementById('connect-btn'),
    disconnectBtn: document.getElementById('disconnect-btn'),
    statusIndicator: document.getElementById('status-indicator'),
    statusText: document.getElementById('status-text'),
    terminal: document.getElementById('terminal'),
    showTimestamp: document.getElementById('show-timestamp'),
    hexMode: document.getElementById('hex-mode'),
    autoscroll: document.getElementById('autoscroll'),
    clearBtn: document.getElementById('clear-btn'),
    saveBtn: document.getElementById('save-btn'),
    rxCount: document.getElementById('rx-count'),
    sendInput: document.getElementById('send-input'),
    sendBtn: document.getElementById('send-btn'),
    sendHex: document.getElementById('send-hex'),
    sendNewline: document.getElementById('send-newline'),
    txCount: document.getElementById('tx-count'),
    aboutLink: document.getElementById('about-link'),
    aboutModal: document.getElementById('about-modal'),
    closeModal: document.querySelector('.close')
};

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    loadPorts();
    setupEventListeners();
    checkStatus();
});

// Setup event listeners
function setupEventListeners() {
    // Connection
    elements.refreshPorts.addEventListener('click', loadPorts);
    elements.connectBtn.addEventListener('click', connect);
    elements.disconnectBtn.addEventListener('click', disconnect);
    
    // Terminal
    elements.clearBtn.addEventListener('click', clearTerminal);
    elements.saveBtn.addEventListener('click', saveToFile);
    
    // Send
    elements.sendBtn.addEventListener('click', sendData);
    elements.sendInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') sendData();
    });
    
    // Quick commands
    document.querySelectorAll('.btn-quick').forEach(btn => {
        btn.addEventListener('click', () => {
            elements.sendInput.value = btn.dataset.cmd;
            sendData();
        });
    });
    
    // Modal
    elements.aboutLink.addEventListener('click', (e) => {
        e.preventDefault();
        elements.aboutModal.style.display = 'block';
    });
    elements.closeModal.addEventListener('click', () => {
        elements.aboutModal.style.display = 'none';
    });
    window.addEventListener('click', (e) => {
        if (e.target === elements.aboutModal) {
            elements.aboutModal.style.display = 'none';
        }
    });
}

// Load available serial ports
async function loadPorts() {
    try {
        const response = await fetch('/api/ports');
        const data = await response.json();
        
        if (data.success) {
            elements.port.innerHTML = '<option value="">Select Port...</option>';
            data.ports.forEach(port => {
                const option = document.createElement('option');
                option.value = port.device;
                option.textContent = `${port.device} - ${port.description}`;
                elements.port.appendChild(option);
            });
            logToTerminal('system', 'Ports refreshed');
        }
    } catch (error) {
        logToTerminal('system', `Error loading ports: ${error.message}`);
    }
}

// Connect to serial port
async function connect() {
    const port = elements.port.value;
    if (!port) {
        alert('Please select a port');
        return;
    }
    
    const config = {
        port: port,
        baudrate: elements.baudrate.value,
        bytesize: elements.databits.value,
        parity: elements.parity.value,
        stopbits: elements.stopbits.value
    };
    
    try {
        elements.connectBtn.disabled = true;
        elements.connectBtn.textContent = 'Connecting...';
        
        const response = await fetch('/api/connect', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(config)
        });
        
        const data = await response.json();
        
        if (data.success) {
            isConnected = true;
            updateConnectionStatus(true, port);
            logToTerminal('system', `Connected to ${port}`);
            startPolling();
        } else {
            throw new Error(data.error);
        }
    } catch (error) {
        logToTerminal('system', `Connection failed: ${error.message}`);
        alert(`Connection failed: ${error.message}`);
    } finally {
        elements.connectBtn.disabled = false;
        elements.connectBtn.textContent = 'Connect';
    }
}

// Disconnect from serial port
async function disconnect() {
    try {
        const response = await fetch('/api/disconnect', {
            method: 'POST'
        });
        
        const data = await response.json();
        
        if (data.success) {
            isConnected = false;
            updateConnectionStatus(false);
            stopPolling();
            logToTerminal('system', 'Disconnected');
        }
    } catch (error) {
        logToTerminal('system', `Disconnect error: ${error.message}`);
    }
}

// Update connection status UI
function updateConnectionStatus(connected, port = '') {
    isConnected = connected;
    
    if (connected) {
        elements.statusIndicator.classList.remove('disconnected');
        elements.statusIndicator.classList.add('connected');
        elements.statusText.textContent = `Connected to ${port}`;
        elements.connectBtn.disabled = true;
        elements.disconnectBtn.disabled = false;
        elements.sendBtn.disabled = false;
        elements.port.disabled = true;
    } else {
        elements.statusIndicator.classList.remove('connected');
        elements.statusIndicator.classList.add('disconnected');
        elements.statusText.textContent = 'Disconnected';
        elements.connectBtn.disabled = false;
        elements.disconnectBtn.disabled = true;
        elements.sendBtn.disabled = true;
        elements.port.disabled = false;
    }
}

// Check initial status
async function checkStatus() {
    try {
        const response = await fetch('/api/status');
        const data = await response.json();
        
        if (data.success && data.connected) {
            updateConnectionStatus(true, data.port);
            startPolling();
        }
    } catch (error) {
        console.error('Status check failed:', error);
    }
}

// Start polling for received data
function startPolling() {
    if (pollInterval) return;
    pollInterval = setInterval(pollReceivedData, 100);
}

// Stop polling
function stopPolling() {
    if (pollInterval) {
        clearInterval(pollInterval);
        pollInterval = null;
    }
}

// Poll for received data
async function pollReceivedData() {
    try {
        const response = await fetch('/api/receive');
        const data = await response.json();
        
        if (data.success && data.data.length > 0) {
            data.data.forEach(item => {
                displayReceivedData(item);
            });
        }
    } catch (error) {
        console.error('Poll error:', error);
    }
}

// Display received data
function displayReceivedData(item) {
    const hexMode = elements.hexMode.checked;
    const showTimestamp = elements.showTimestamp.checked;
    
    let displayData = hexMode ? item.data : item.ascii;
    
    // Format hex data with spaces
    if (hexMode) {
        displayData = displayData.match(/.{1,2}/g).join(' ');
    }
    
    const timestamp = showTimestamp ? item.timestamp : '';
    logToTerminal('rx', displayData, timestamp);
    
    rxBytes += item.data.length / 2;
    elements.rxCount.textContent = `RX: ${rxBytes} bytes`;
}

// Send data
async function sendData() {
    const text = elements.sendInput.value;
    if (!text || !isConnected) return;
    
    const hexMode = elements.sendHex.checked;
    const addNewline = elements.sendNewline.checked;
    
    try {
        const response = await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                text: text,
                hex: hexMode,
                newline: addNewline
            })
        });
        
        const data = await response.json();
        
        if (data.success) {
            const displayText = hexMode ? 
                text.replace(/\s/g, '').match(/.{1,2}/g).join(' ') : 
                text + (addNewline ? '\\n' : '');
            
            const timestamp = elements.showTimestamp.checked ? 
                new Date().toLocaleTimeString() : '';
            
            logToTerminal('tx', displayText, timestamp);
            
            txBytes += data.bytes_sent;
            elements.txCount.textContent = `TX: ${txBytes} bytes`;
            
            elements.sendInput.value = '';
        } else {
            throw new Error(data.error);
        }
    } catch (error) {
        logToTerminal('system', `Send error: ${error.message}`);
    }
}

// Log to terminal
function logToTerminal(type, message, timestamp = '') {
    const line = document.createElement('div');
    line.className = 'line';
    
    let prefix = '';
    if (type === 'rx') prefix = '← ';
    else if (type === 'tx') prefix = '→ ';
    else if (type === 'system') prefix = '⚙ ';
    
    let html = '';
    if (timestamp) {
        html += `<span class="timestamp">[${timestamp}]</span>`;
    }
    html += `<span class="${type}">${prefix}${escapeHtml(message)}</span>`;
    
    line.innerHTML = html;
    elements.terminal.appendChild(line);
    
    // Auto scroll
    if (elements.autoscroll.checked) {
        elements.terminal.scrollTop = elements.terminal.scrollHeight;
    }
}

// Clear terminal
function clearTerminal() {
    elements.terminal.innerHTML = '';
    rxBytes = 0;
    txBytes = 0;
    elements.rxCount.textContent = 'RX: 0 bytes';
    elements.txCount.textContent = 'TX: 0 bytes';
    logToTerminal('system', 'Terminal cleared');
}

// Save to file
function saveToFile() {
    const lines = elements.terminal.querySelectorAll('.line');
    let content = '';
    
    lines.forEach(line => {
        content += line.textContent + '\n';
    });
    
    const blob = new Blob([content], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `serial_log_${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.txt`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    
    logToTerminal('system', 'Log saved to file');
}

// Escape HTML
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Handle page unload
window.addEventListener('beforeunload', () => {
    if (isConnected) {
        disconnect();
    }
});

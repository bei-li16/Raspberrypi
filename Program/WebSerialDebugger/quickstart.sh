#!/bin/bash
# Quick Start Script for Web Serial Debugger
# Usage: ./quickstart.sh [port]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "  Web Serial Debugger - Quick Start"
echo "=========================================="

# Check Python
if ! command -v python3 &> /dev/null; then
    echo "❌ Error: Python 3 is not installed"
    exit 1
fi

PYTHON_VERSION=$(python3 --version 2>&1 | awk '{print $2}')
echo "✅ Python version: $PYTHON_VERSION"

# Check/Install dependencies
echo ""
echo "📦 Checking dependencies..."
if ! python3 -c "import flask, serial" 2>/dev/null; then
    echo "Installing required packages..."
    pip3 install -r requirements.txt
else
    echo "✅ Dependencies already installed"
fi

# Get port from argument or default
PORT=${1:-5000}

echo ""
echo "🚀 Starting Web Serial Debugger..."
echo "   URL: http://localhost:$PORT"
echo ""
echo "Press Ctrl+C to stop"
echo "=========================================="

python3 app.py --port "$PORT"

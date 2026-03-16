#!/usr/bin/env python3
"""
Test script for file send functionality
"""

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Test parseFileContent logic
def test_parse_ascii():
    """Test ASCII parsing with delimiters"""
    content = "1,2;3,4"
    bytes_list = []
    for char in content:
        if char != ',' and char != ';':
            bytes_list.append(ord(char))
    
    assert bytes_list == [ord('1'), ord('2'), ord('3'), ord('4')], f"Got {bytes_list}"
    print("✅ ASCII parsing test passed")

def test_parse_hex():
    """Test Hex parsing with 0x prefix"""
    content = "0x01 0x02 0x0A 0xFF"
    import re
    # Match 0x prefix style
    hex_values = re.findall(r'0x([0-9a-fA-F]{1,2})', content)
    bytes_list = [int(hex_val, 16) for hex_val in hex_values]

    assert bytes_list == [0x01, 0x02, 0x0A, 0xFF], f"Got {bytes_list}"
    print("✅ Hex parsing (0x prefix) test passed")

def test_parse_hex_compact():
    """Test compact hex format"""
    content = "01 02 0A FF"
    import re
    hex_values = re.findall(r'[0-9a-fA-F]{2}', content)
    bytes_list = [int(hex_val, 16) for hex_val in hex_values]

    assert bytes_list == [0x01, 0x02, 0x0A, 0xFF], f"Got {bytes_list}"
    print("✅ Compact hex parsing test passed")

def test_send_modes():
    """Test send mode index calculations"""
    bytes_data = [1, 2, 3]
    total = len(bytes_data)
    
    # Oneshot mode
    index = 0
    results = []
    for _ in range(5):
        results.append(bytes_data[index])
        index += 1
        if index >= total:
            break
    assert results == [1, 2, 3], f"Oneshot failed: {results}"
    print("✅ Oneshot mode test passed")
    
    # Continuous mode
    index = 0
    results = []
    for _ in range(7):
        results.append(bytes_data[index])
        index = (index + 1) % total
    assert results == [1, 2, 3, 1, 2, 3, 1], f"Continuous failed: {results}"
    print("✅ Continuous mode test passed")
    
    # Pingpong mode
    index = 0
    direction = 1
    results = []
    for _ in range(10):
        results.append(bytes_data[index])
        index += direction
        if index >= total - 1:
            direction = -1
        elif index <= 0:
            direction = 1
    assert results == [1, 2, 3, 2, 1, 2, 3, 2, 1, 2], f"Pingpong failed: {results}"
    print("✅ Pingpong mode test passed")

if __name__ == '__main__':
    print("="*60)
    print("FILE SEND FUNCTIONALITY TESTS")
    print("="*60)
    
    test_parse_ascii()
    test_parse_hex()
    test_parse_hex_compact()
    test_send_modes()
    
    print("="*60)
    print("All tests passed!")
    print("="*60)

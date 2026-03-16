#!/usr/bin/env python3
"""
Test script for delimiter-based frame parsing logic
Simulates serial data arrival patterns
"""

import time

# Test configuration
FRAME_DELIMITER = b'\n'

class FrameBuffer:
    """Simulates the frame buffering logic from app.py"""
    
    def __init__(self):
        self.frame_buffer = bytearray()
        self.received_frames = []
    
    def feed_data(self, data: bytes):
        """Simulate receiving data from serial port"""
        self.frame_buffer.extend(data)
        self._process_frames()
    
    def _process_frames(self):
        """Process complete frames (same logic as app.py)"""
        while FRAME_DELIMITER in self.frame_buffer:
            frame_end = self.frame_buffer.index(FRAME_DELIMITER)
            frame_data = self.frame_buffer[:frame_end]

            timestamp = time.strftime('%H:%M:%S') + f'.{int(time.time() * 1000) % 1000:03d}'
            self.received_frames.append({
                'timestamp': timestamp,
                'data': frame_data.hex(),
                'ascii': frame_data.decode('utf-8', errors='replace')
            })

            self.frame_buffer = self.frame_buffer[frame_end + 1:]
    
    def flush_incomplete(self):
        """Flush any remaining incomplete frame"""
        if len(self.frame_buffer) > 0:
            timestamp = time.strftime('%H:%M:%S') + f'.{int(time.time() * 1000) % 1000:03d}'
            self.received_frames.append({
                'timestamp': timestamp,
                'data': self.frame_buffer.hex(),
                'ascii': self.frame_buffer.decode('utf-8', errors='replace') + ' [incomplete]'
            })
            self.frame_buffer = bytearray()
    
    def print_frames(self, test_name: str):
        """Print received frames"""
        print(f"\n{'='*60}")
        print(f"Test: {test_name}")
        print(f"{'='*60}")
        for i, frame in enumerate(self.received_frames, 1):
            print(f"[{frame['timestamp']}]← {frame['ascii']!r}")
        if not self.received_frames:
            print("(no frames received)")
        print(f"Buffer remaining: {self.frame_buffer!r}")


def test_case_1_normal_frames():
    """Test: Normal frame reception"""
    print("\n" + "="*60)
    print("TEST 1: Normal frame reception")
    print("Input: 'hello\\nworld\\n'")
    
    fb = FrameBuffer()
    fb.feed_data(b'hello\nworld\n')
    fb.print_frames("Normal frames")
    
    assert len(fb.received_frames) == 2
    assert fb.received_frames[0]['ascii'] == 'hello'
    assert fb.received_frames[1]['ascii'] == 'world'
    assert len(fb.frame_buffer) == 0
    print("✅ PASSED")


def test_case_2_split_frame():
    """Test: Frame split across multiple reads (the original bug)"""
    print("\n" + "="*60)
    print("TEST 2: Frame split across multiple reads (original bug)")
    print("Input: 'lock' then 'ed\\n' (simulating slow arrival)")
    
    fb = FrameBuffer()
    # Simulate data arriving in chunks
    fb.feed_data(b'lock')      # First chunk
    fb.feed_data(b'ed\n')       # Second chunk completes the frame
    fb.print_frames("Split frame")
    
    assert len(fb.received_frames) == 1
    assert fb.received_frames[0]['ascii'] == 'locked'
    print("✅ PASSED - Frame correctly reassembled!")


def test_case_3_multiple_splits():
    """Test: Multiple frames with various split patterns"""
    print("\n" + "="*60)
    print("TEST 3: Multiple frames with various split patterns")
    print("Input: 'abc' then '\\n' then 'def' then 'ghi\\n' then 'jkl\\n'")
    
    fb = FrameBuffer()
    fb.feed_data(b'abc')       # incomplete
    fb.feed_data(b'\n')        # completes 'abc'
    fb.feed_data(b'def')       # incomplete
    fb.feed_data(b'ghi\n')     # completes 'defghi'
    fb.feed_data(b'jkl\n')     # complete frame
    fb.print_frames("Multiple splits")
    
    assert len(fb.received_frames) == 3
    assert fb.received_frames[0]['ascii'] == 'abc'
    assert fb.received_frames[1]['ascii'] == 'defghi'
    assert fb.received_frames[2]['ascii'] == 'jkl'
    print("✅ PASSED")


def test_case_4_empty_frames():
    """Test: Empty frames (consecutive delimiters)"""
    print("\n" + "="*60)
    print("TEST 4: Empty frames (consecutive delimiters)")
    print("Input: 'hello\\n\\nworld\\n'")
    
    fb = FrameBuffer()
    fb.feed_data(b'hello\n\nworld\n')
    fb.print_frames("Empty frames")
    
    # Should get: 'hello', '', 'world'
    assert len(fb.received_frames) == 3
    assert fb.received_frames[0]['ascii'] == 'hello'
    assert fb.received_frames[1]['ascii'] == ''
    assert fb.received_frames[2]['ascii'] == 'world'
    print("✅ PASSED")


def test_case_5_no_delimiter():
    """Test: Data without delimiter (incomplete frame)"""
    print("\n" + "="*60)
    print("TEST 5: Data without delimiter (incomplete frame)")
    print("Input: 'incomplete_data' (no newline)")
    
    fb = FrameBuffer()
    fb.feed_data(b'incomplete_data')
    fb.print_frames("No delimiter")
    
    assert len(fb.received_frames) == 0
    assert fb.frame_buffer == b'incomplete_data'
    
    # Now flush incomplete
    fb.flush_incomplete()
    fb.print_frames("After flush")
    
    assert len(fb.received_frames) == 1
    assert 'incomplete' in fb.received_frames[0]['ascii']
    print("✅ PASSED")


def test_case_6_delimiter_at_start():
    """Test: Delimiter at start (empty first frame)"""
    print("\n" + "="*60)
    print("TEST 6: Delimiter at start")
    print("Input: '\\nhello\\n'")
    
    fb = FrameBuffer()
    fb.feed_data(b'\nhello\n')
    fb.print_frames("Delimiter at start")
    
    assert len(fb.received_frames) == 2
    assert fb.received_frames[0]['ascii'] == ''
    assert fb.received_frames[1]['ascii'] == 'hello'
    print("✅ PASSED")


def test_case_7_byte_by_byte():
    """Test: Byte-by-byte arrival (worst case)"""
    print("\n" + "="*60)
    print("TEST 7: Byte-by-byte arrival (worst case)")
    print("Input: 'l', 'o', 'c', 'k', 'e', 'd', '\\n'")
    
    fb = FrameBuffer()
    data = b'locked\n'
    for byte in data:
        fb.feed_data(bytes([byte]))
    fb.print_frames("Byte by byte")
    
    assert len(fb.received_frames) == 1
    assert fb.received_frames[0]['ascii'] == 'locked'
    print("✅ PASSED")


def test_case_8_mixed_content():
    """Test: Mixed content with special characters"""
    print("\n" + "="*60)
    print("TEST 8: Mixed content with special characters")
    print("Input: 'locked .\\n\\r\\nerror\\n'")
    
    fb = FrameBuffer()
    fb.feed_data(b'locked .\n\r\nerror\n')
    fb.print_frames("Mixed content")
    
    # Should get: 'locked .', '\r', 'error'
    assert len(fb.received_frames) == 3
    assert fb.received_frames[0]['ascii'] == 'locked .'
    assert fb.received_frames[1]['ascii'] == '\r'
    assert fb.received_frames[2]['ascii'] == 'error'
    print("✅ PASSED")


def test_case_9_large_frame():
    """Test: Large frame"""
    print("\n" + "="*60)
    print("TEST 9: Large frame")
    large_data = b'x' * 1000 + b'\n'
    print(f"Input: 1000 'x' characters + newline")
    
    fb = FrameBuffer()
    # Split into chunks
    fb.feed_data(large_data[:300])
    fb.feed_data(large_data[300:700])
    fb.feed_data(large_data[700:])
    fb.print_frames("Large frame")
    
    assert len(fb.received_frames) == 1
    assert len(fb.received_frames[0]['ascii']) == 1000
    print("✅ PASSED")


def run_all_tests():
    """Run all test cases"""
    print("\n" + "="*60)
    print("FRAME BUFFER TEST SUITE")
    print("Testing delimiter-based frame parsing logic")
    print("="*60)
    
    tests = [
        test_case_1_normal_frames,
        test_case_2_split_frame,
        test_case_3_multiple_splits,
        test_case_4_empty_frames,
        test_case_5_no_delimiter,
        test_case_6_delimiter_at_start,
        test_case_7_byte_by_byte,
        test_case_8_mixed_content,
        test_case_9_large_frame,
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        try:
            test()
            passed += 1
        except AssertionError as e:
            print(f"❌ FAILED: {e}")
            failed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            failed += 1
    
    print("\n" + "="*60)
    print(f"RESULTS: {passed} passed, {failed} failed")
    print("="*60)
    
    return failed == 0


if __name__ == '__main__':
    success = run_all_tests()
    exit(0 if success else 1)

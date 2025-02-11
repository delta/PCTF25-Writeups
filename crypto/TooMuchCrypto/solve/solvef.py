def m_to_bytes(m):

    message = b""
    for word in m:
        message += word.to_bytes(4, "big")  # Convert 32-bit word to 4 bytes (big-endian)
    return message

def remove_padding(message):

    padding_length = message[-1]  # Get the padding length from the last byte
    if padding_length > len(message):
        raise ValueError("Invalid padding length.")
    return message[:-padding_length]  # Remove the padding bytes

def main():
    
    m = [0x705f6374, 0x667b4d72, 0x5f41756d, 0x34733530, 
     0x4e5f7430, 0x6c345f74, 0x6869735f, 0x77307531, 
     0x445f7730, 0x726b5f68, 0x65683321, 0x7d800000, 
     0x00000000, 0x00000001, 0x00000000, 0x00000168];

    # Step 1: Convert m[] to bytes
    reconstructed_message = m_to_bytes(m)
    print("Reconstructed message (with padding):", reconstructed_message)

    # Step 2: Remove padding
    try:
        original_message = remove_padding(reconstructed_message)
        print("Original message (without padding):", original_message.decode())
    except ValueError as e:
        print("Error removing padding:", e)

if __name__ == "__main__":
    main()
import can
import struct

CAN_ID = 0x786
BITRATE = 250000

def main():
    print("Connecting to Kvaser...")

    bus = can.Bus(
        interface="kvaser",
        channel=0,
        bitrate=BITRATE
    )

    print("Listening for CAN frames... (Ctrl+C to stop)")

    while True:
        msg = bus.recv(timeout=1.0)
        if msg is None:
            continue

        if msg.arbitration_id != CAN_ID:
            continue

        if len(msg.data) < 4:
            continue

        # Decode big-endian, signed 16-bit values
        temp_1_raw, temp_2_raw = struct.unpack(">hh", msg.data[0:4])

        temp_1 = temp_1_raw / 100.0
        temp_2 = temp_2_raw / 100.0

        print(f"Sensor1: {temp_1:6.2f} C   Sensor2: {temp_2:6.2f} C")


if __name__ == "__main__":
    main()
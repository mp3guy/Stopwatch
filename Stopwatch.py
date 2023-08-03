import time
import socket
import struct

class Stopwatch:
    SEND_INTERVAL_MS = 10000
    timings_ms = {}
    tocks_us = {}
    ticks_us = {}
    signature = 0
    last_send = current_send = int(time.perf_counter() * 1000000)
    sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    servaddr = ("127.0.0.1", 45454)

    @staticmethod
    def add_stopwatch_timing(name, duration):
        if duration > 0:
            Stopwatch.timings_ms[name] = float(duration) / 1000.0

    @staticmethod
    def set_custom_signature(new_signature):
        Stopwatch.signature = new_signature

    @staticmethod
    def get_timings():
        return Stopwatch.timings_ms

    @staticmethod
    def print_all():
        for name, duration in Stopwatch.timings_ms.items():
            print(f"{name}: {duration}ms")
        print()

    @staticmethod
    def send_all():
        if not (Stopwatch.timings_ms or Stopwatch.ticks_us or Stopwatch.tocks_us):
            return  # Nothing to send

        current_time = int(time.perf_counter() * 1000000)
        if current_time - Stopwatch.last_send > Stopwatch.SEND_INTERVAL_MS:
            data = Stopwatch.serialise_timings()
            Stopwatch.sockfd.sendto(data, Stopwatch.servaddr)
            Stopwatch.last_send = current_time

    @staticmethod
    def get_current_system_time():
        return int(time.perf_counter() * 1000000)

    @staticmethod
    def tick(name):
        Stopwatch.ticks_us[name] = Stopwatch.get_current_system_time()

    @staticmethod
    def tock(name):
        end = Stopwatch.get_current_system_time()
        Stopwatch.tocks_us[name] = end

        tick_us = Stopwatch.ticks_us.get(name)
        if tick_us:
            duration = (end - tick_us) / 1000.0
            if duration > 0:
                Stopwatch.timings_ms[name] = duration

    @staticmethod
    def serialise_timings():
        # Compute the total size of the packet
        packet_size_bytes = struct.calcsize("=Iq")  # int32_t + uint64_t
        for name in Stopwatch.timings_ms.keys():
            packet_size_bytes += (
                struct.calcsize("=B") + len(name) + 1 + struct.calcsize("=f")
            )
        for name in Stopwatch.ticks_us.keys():
            packet_size_bytes += (
                struct.calcsize("=B") + len(name) + 1 + struct.calcsize("=Q")
            )
        for name in Stopwatch.tocks_us.keys():
            packet_size_bytes += (
                struct.calcsize("=B") + len(name) + 1 + struct.calcsize("=Q")
            )

        data = bytearray(packet_size_bytes)
        offset = 0

        # Write the packet size
        struct.pack_into("=I", data, offset, packet_size_bytes)
        offset += struct.calcsize("=I")

        # Write the signature
        struct.pack_into("=q", data, offset, Stopwatch.signature)
        offset += struct.calcsize("=q")

        # Serialize the timings
        def serialize_map(type_byte, name_value_map, value_format):
            nonlocal offset
            for name, value in name_value_map.items():
                struct.pack_into("=B", data, offset, type_byte)
                offset += struct.calcsize("=B")
                struct.pack_into(f"={len(name)}s", data, offset, name.encode("ascii"))
                offset += len(name)
                data[offset] = 0  # Null terminator
                offset += 1
                struct.pack_into(value_format, data, offset, value)
                offset += struct.calcsize(value_format)

        serialize_map(0, Stopwatch.timings_ms, "=f")
        serialize_map(1, Stopwatch.ticks_us, "=Q")
        serialize_map(2, Stopwatch.tocks_us, "=Q")

        return data

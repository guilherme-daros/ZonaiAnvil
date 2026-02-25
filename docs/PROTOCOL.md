# Communication Protocol Specification (v1.0)

This document describes the binary TLV (Type-Length-Value) protocol used between the Desktop App and the Embedded Device.

## 1. Packet Structure
All communications follow this fixed-header format:

| Offset | Field | Type | Description |
| :--- | :--- | :--- | :--- |
| 0 | Start Byte | uint8_t | Fixed value: `0xAA` |
| 1 | Command ID | uint8_t | See Command Set below |
| 2-3 | Length | uint16_t | Payload length (Little Endian) |
| 4...N | Payload | bytes | Data specific to the command |
| N+1 | Checksum | uint8_t | XOR sum of all Payload bytes |

---

## 2. Command Set

### 2.1 CMD_PING (0x00)
**Description:** Connectivity check.
*   **Request:** Empty payload.
*   **Response:** `[0x01]` (ACK).

### 2.2 CMD_GET_SCHEMA (0x10)
**Description:** Requests the list of parameters supported by the device.
*   **Request:** Empty payload.
*   **Response Payload:**
    *   `[0]` Count (uint8_t)
    *   `[...]` Repeated for each parameter:
        *   `[0]` Parameter ID (uint8_t)
        *   `[1]` Type (uint8_t: 1=Toggle, 2=Slider, 3=Numeric)
        *   `[2]` Name Length (uint8_t)
        *   `[3...M]` Name String (UTF-8)
        *   `[M+1...M+4]` Min Value (float32)
        *   `[M+5...M+8]` Max Value (float32)

### 2.3 CMD_READ_ALL (0x21)
**Description:** Requests current values for all parameters in the schema.
*   **Request:** Empty payload.
*   **Response Payload:**
    *   `[0]` Count (uint8_t)
    *   `[...]` Repeated for each parameter:
        *   `[0]` Parameter ID (uint8_t)
        *   `[1...4]` Value (float32)

### 2.4 CMD_WRITE_VALUE (0x30)
**Description:** Updates a single parameter on the device.
*   **Request Payload:**
    *   `[0]` Parameter ID (uint8_t)
    *   `[1...4]` New Value (float32)
*   **Response Payload:**
    *   `[0]` Parameter ID (uint8_t) - Serves as an ACK for that specific ID.

---

## 3. Implementation Details
*   **Byte Order:** Little Endian for all multi-byte values (`uint16_t`, `float`).
*   **Reliability:** The App waits for a response packet with a matching Command ID and valid checksum before marking a "Pending" UI state as complete.

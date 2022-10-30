# Example: pipe_pirate

Contains code that uses the kernel arbitrary read/write primitive provided by WebKit to change the process credentials to a tagged uid (0x1337 by default). 

## Notes
- Replace `PC_IP` and `PC_PORT` macros on lines 10-11 with your TCP server's IP/port.
- This example is firmware-dependent. This will only work on firmwares supported by the SDK for kernel hacking. See SDK main README.md for this information.
  - Set `PS5SDK_FW` to the correct target before building. For example, to target 4.03, `PS5SDK_FW` should be set to `0x403`.


## Expected result
Log:
```
[+] kernel .data base is ffffffffde7c0000, pipe 10->11, rw pair 12->78, pipe addr is ffffbfad00f20fc0
[+] PID = 0x82
[+] ucred = ffffbfad7e084800
[+] did we patch uid? uid = 0x1337 (tag is 0x1337)
```

## Authors
- Specter
- Znullptr
- ChendoChap
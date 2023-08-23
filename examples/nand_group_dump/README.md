# Example: nand_group_dumper
Contains code to dump NAND groups to USB device.

## Notes
Be careful with this payload, it disables a critical controller in order to read NAND. There is a risk of bricking when
using these APIs, and the system will be put into a bad state after running this ELF. Use at your own risk and restart
the system after using.

## Expected result
Assuming logs are enabled, log server should report:
```
[+] opened output files: 15, 16, 17
[+] kernel .data base is ffffffff90b30000, pipe 10->11, rw pair 12->20, pipe addr is ffffe4d46b765200
[+] PID = 0x4e
[+] kernel r/w initialized...
[+] ucred = ffffe4d459504200
[+] a53io dev = 0x15 (errno = 0)
[+] disable controller = 0xffffffff (errno = 37)
[+] open nand = -1 (errno = 37)
[+] pup update dev = 0x16 (errno = 37)
[+] read nand group 0 = 0 (errno = 37)
[+] wrote 67108864 bytes...
[+] read nand group 1 = 0 (errno = 37)
[+] wrote 67108864 bytes...
[+] read nand group 2 = 0 (errno = 37)
[+] wrote 67108864 bytes...
Done!
```

Additionally, three dump files named "nandgroup[N]" should be written to `[usb root]/PS5/`.

## Authors
- Specter
- flatz
